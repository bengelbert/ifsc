/*
 * application include files
 */
#include "wrapper.h"

/*
 * Constants
 */
#define SERVER_SEND_PING    0x20

#define SERVER_RECV_PING    0x00

/*
 * Type definitions
 */
typedef struct server_header_s  server_header_t;
typedef struct server_host_s    server_host_t;
typedef struct server_net_s     server_net_t;
typedef struct server_tags_s    server_tags_t;

/******************************************************************************/

struct server_header_s {
    guint16 n_bytes;
    guint16 version;
    guint8  cmd;
    guint32 time_event;
    guint32 time_send;
    guint16 dbfc_id;
} PACKED;

/******************************************************************************/

struct server_host_s {
    global_t *        global;
    server_net_t *  net;
};

/******************************************************************************/

struct server_net_s {
    GConn *         conn;
    GConnEvent *    event;
    GByteArray *    queue;
    GServer *       server;
    server_tags_t * tags;
};

/******************************************************************************/

struct server_tags_s {
    guint connect;
    guint ping;
    guint queue_pop;
};

/******************************************************************************/
/*
 * Function prototypes
 */
static void                 server_cb                       (GServer * server, GConn * conn, gpointer data);
static void                 server_client_cb                (GConn * conn, GConnEvent * event, gpointer data);
static void                 server_conn_connect             (server_host_t * host);
static void                 server_conn_delete              (server_host_t * host);
static void                 server_conn_dump                (guint8 * buffer, gint len);
static void                 server_conn_read                (server_host_t * host);
static void                 server_conn_write               (server_host_t * host, GByteArray * array);
static server_host_t *      server_host_new                 (global_t * global);
static GByteArray *         server_make_header              (GByteArray * array, guchar cmd, guint payload_len);
static gint                 server_message_handler          (server_host_t * host);
static server_header_t *    server_message_header_unpack    (guint8 * data);
static GConn *              server_net_get_conn             (server_host_t * host);
static server_net_t *       server_net_new                  (void);
static guint8 *             server_net_queue_get_data       (server_host_t * host);
static guint                server_net_queue_get_len        (server_host_t * host);
static void                 server_net_queue_push           (server_host_t * host);
static gboolean             server_net_queue_try_pop        (gpointer host);
static GByteArray *         server_pack_prepend_int8        (GByteArray * array, guint data);
static GByteArray *         server_pack_prepend_int16       (GByteArray * array, guint data);
static GByteArray *         server_pack_prepend_int32       (GByteArray * array, guint data);
static GByteArray *         server_pack_string              (GByteArray * array, gchar * str);
static GByteArray *         server_pack_string_sized        (GByteArray * array, gchar * str, guint len);
static gboolean             server_send_ping                (gpointer data);
static void                 server_source_remove            (guint tag);

/******************************************************************************/
/*
 * Macros
 */
#define SERVER_HEADER_LEN   (sizeof (server_header_t))

/******************************************************************************/
/*
 * Function definitions
 */
static void
server_cb(GServer * server, GConn * conn, gpointer data)
{
    server_host_t * host = (server_host_t *) data;

    g_assert(host);
    g_assert(host->net);

    debug("SERVER", "server -------------- [%p]", server);
    debug("SERVER", "conn ---------------- [%p]", conn);
    debug("SERVER", "host ---------------- [%p]", host);
    debug("SERVER", "host->net ----------- [%p]", host->net);
    debug("SERVER", "host->net->tags ----- [%p]", host->net->tags);

    host->net->conn = conn;

    if (host->net->conn != NULL) {
        gnet_conn_set_callback(host->net->conn, server_client_cb, data);
        gnet_conn_set_watch_error(host->net->conn, true);
        gnet_conn_read(host->net->conn);

        host->net->tags->ping = common_timeout_add("PING", global_get_server_ping_interval(host->global), server_send_ping, host);
        host->net->tags->queue_pop = common_timeout_add("QUEUE_POP", 100, server_net_queue_try_pop, host);
        
    } else {
        gnet_server_delete(server);
        exit(EXIT_FAILURE);
    }

    return;
}

/******************************************************************************/

static void
server_client_cb(GConn * conn, GConnEvent * event, gpointer data)
{
    server_host_t * host = NULL;

    g_assert(conn);
    g_assert(data);
    g_assert(event);

    host = (server_host_t *) data;
    host->net->event = event;

    switch (event->type) {
    case GNET_CONN_CONNECT: server_conn_connect(host);    break;
    case GNET_CONN_READ:    server_conn_read(host);       break;
    case GNET_CONN_WRITE:   break;
    case GNET_CONN_CLOSE:
    case GNET_CONN_TIMEOUT:
    case GNET_CONN_ERROR:   server_conn_delete(host);     break;
    default:
        g_assert_not_reached();
    }

    return;
}

/******************************************************************************/

static void
server_conn_connect(server_host_t * host)
{
    g_assert(host);
    g_assert(host->net);
    g_assert(host->net->conn);
    g_assert(host->net->tags);

    server_source_remove(host->net->tags->connect);

    gnet_conn_timeout(host->net->conn, 0);

    gnet_conn_read(host->net->conn);
}

/******************************************************************************/

static void
server_conn_delete(server_host_t * host)
{
    g_assert(host);

    debug("SERVER", "([%p])", host);

    server_source_remove(host->net->tags->connect);
    server_source_remove(host->net->tags->ping);

    gnet_conn_delete(host->net->conn);
    host->net->conn = NULL;

    return;
}

/******************************************************************************/

static void
server_conn_dump(guint8 * buffer, gint len)
{
    gchar   dump[4096];
    gint    i, j;
    gint    dec = 0;

    g_assert(buffer);

    debug("SERVER", "len ----- [%d]", len);

    memset(dump, 0, sizeof (dump));
    for (i = 0, j = 0; i < len; i++, j+=3) {

        if (!(i % 10)) {
            g_snprintf(&dump[j + dec], 2, "\n");
            dec += 1;
        }

        g_snprintf(&dump[j+dec], 4, "%02X ", buffer[i]);
    }

    debug("SERVER", "\n%s\n", dump);
}

/******************************************************************************/

static void
server_conn_read(server_host_t * host)
{
    g_assert(host);
    g_assert(host->net);
    g_assert(host->net->event);
    g_assert(host->net->event->buffer);

    server_conn_dump((guint8 *) host->net->event->buffer, host->net->event->length);

    server_net_queue_push(host);

    gnet_conn_read(host->net->conn);
}

/******************************************************************************/

static void
server_conn_write(server_host_t * host, GByteArray * array)
{
    g_assert(array);
    g_assert(array->data);
    g_assert(host);

    server_conn_dump(array->data, array->len);

    gnet_conn_write(server_net_get_conn(host), (gchar *) array->data, array->len);

    g_byte_array_free(array, true);
}

/******************************************************************************/

static server_header_t *
server_message_header_unpack(guint8 * data)
{
    server_header_t * header = (server_header_t *) data;

    g_assert(header);

    header->n_bytes = g_htons(header->n_bytes);
    header->version = g_htons(header->version);
    header->time_event = g_htonl(header->time_event);
    header->time_send = g_htonl(header->time_send);
    header->dbfc_id = g_htons(header->dbfc_id);

    return header;
}

/******************************************************************************/

static server_host_t *
server_host_new(global_t * global)
{
    server_host_t * host = NULL;

    host = g_new0(server_host_t, 1);

    host->global      = global;
    host->net         = server_net_new();
    host->net->server = gnet_server_new(NULL, global_get_server_port(global), server_cb, host);

    debug("SERVER", "host ----------------- [%p]", host);
    debug("SERVER", "host->global --------- [%p]", host->global);
    debug("SERVER", "host->net ------------ [%p]", host->net);
    debug("SERVER", "host->net->queue ----- [%p]", host->net->queue);
    debug("SERVER", "host->net->tags ------ [%p]", host->net->tags);

    return host;
}

/******************************************************************************/

void
server_init(global_t * global)
{
    server_host_t * host = NULL;

    host = server_host_new(global);
}

/******************************************************************************/

static GByteArray *
server_make_header(GByteArray * array, guchar cmd, guint payload_len)
{
    g_assert(array);

    array = server_pack_prepend_int16(array, g_htons(0x0001));
    array = server_pack_prepend_int32(array, g_htonl(time(NULL)));
    array = server_pack_prepend_int32(array, g_htonl(time(NULL)));
    array = server_pack_prepend_int8(array, cmd);
    array = server_pack_prepend_int16(array, g_htons(0x0001));
    array = server_pack_prepend_int16(array, g_htons(payload_len + SERVER_HEADER_LEN));

    return array;
}

/******************************************************************************/

static gint
server_message_handler(server_host_t * host)
{
    server_header_t * header = NULL;

    g_assert(host);

    header = server_message_header_unpack(server_net_queue_get_data(host));

    switch (header->cmd) {
    case SERVER_RECV_PING:  message("SERVER", "Ping received."); break;
    default:
        message("SERVER", "Invalid command ----- [%02X]", header->cmd);
    }

    return header->n_bytes;
}

/******************************************************************************/

static GConn *
server_net_get_conn(server_host_t * host)
{
    g_assert(host);
    g_assert(host->net);
    g_assert(host->net->conn);
    
    return host->net->conn;
}

/******************************************************************************/

static server_net_t *
server_net_new(void)
{
    server_net_t * net = NULL;
    
    net = g_new0(server_net_t, 1);
    
    net->queue  = g_byte_array_new();
    net->tags   = g_new0(server_tags_t, 1);
    
    return net;
}

/******************************************************************************/

static guint8 *
server_net_queue_get_data(server_host_t * host)
{
    g_assert(host);
    g_assert(host->net);
    g_assert(host->net->queue);
    g_assert(host->net->queue->data);

    return host->net->queue->data;
}

/******************************************************************************/

static guint
server_net_queue_get_len(server_host_t * host)
{
    g_assert(host);
    g_assert(host->net);
    g_assert(host->net->queue);

    return host->net->queue->len;
}

/******************************************************************************/

static void
server_net_queue_push(server_host_t * host)
{
    g_assert(host);
    g_assert(host->net);
    g_assert(host->net->event);
    g_assert(host->net->event->buffer);
    g_assert(host->net->queue);

    host->net->queue = g_byte_array_append(host->net->queue, (guint8 *) host->net->event->buffer, host->net->event->length);

    debug("SERVER", "host->net->queue->len ------ [%d]", host->net->queue->len);
    debug("SERVER", "host->net->queue->data ----- [%p]", host->net->queue->data);
}

/******************************************************************************/

static void
server_net_queue_remove_data(server_host_t * host, guint len)
{
    g_assert(host);

    if (len <= server_net_queue_get_len(host)) {
        host->net->queue = g_byte_array_remove_range(host->net->queue, 0, len);
    } else {
        warning("SERVER", "lost of data!!");
        warning("SERVER", "n_bytes ------------------- [%d]", len);
        warning("SERVER", "host->net->queue->len ----- [%d]", server_net_queue_get_len(host));
        host->net->queue = g_byte_array_remove_range(host->net->queue, 0, host->net->queue->len);
    }
}

/******************************************************************************/

static gboolean
server_net_queue_try_pop(gpointer data)
{
    gint n_bytes = 0;
    server_host_t * host = data;

    g_assert(host);

    if (server_net_queue_get_len(host) >= SERVER_HEADER_LEN) {

        n_bytes = server_message_handler(host);

        server_net_queue_remove_data(host, n_bytes);
    }

    return true;
}

/******************************************************************************/

static GByteArray *
server_pack_prepend_int8(GByteArray * array, guint data)
{
    g_assert(array);

    return g_byte_array_prepend(array, (guint8 *) & data, sizeof (guint8));
}

/******************************************************************************/

static GByteArray *
server_pack_prepend_int16(GByteArray * array, guint data)
{
    g_assert(array);

    return g_byte_array_prepend(array, (guint8 *) & data, sizeof (guint16));
}

/******************************************************************************/

static GByteArray *
server_pack_prepend_int32(GByteArray * array, guint data)
{
    g_assert(array);

    return g_byte_array_prepend(array, (guint8 *) & data, sizeof (guint32));
}

/******************************************************************************/

static GByteArray *
server_pack_string(GByteArray * array, gchar * str)
{
    g_assert(array);
    g_assert(str);

    return g_byte_array_prepend(array, (guint8 *) str, strlen(str) + 1);
}

/******************************************************************************/

static GByteArray *
server_pack_string_sized(GByteArray * array, gchar * str, guint len)
{
    g_assert(array);
    g_assert(str);

    return g_byte_array_prepend(array, (guint8 *) str, len);
}

/******************************************************************************/

static gboolean
server_send_ping(gpointer data)
{
    server_host_t * host = (server_host_t *) data;

    g_assert(host);

    GByteArray * array = NULL;

    message("SERVER", "<--------------------");

    array = g_byte_array_new();

    array = server_make_header(array, SERVER_SEND_PING, 0);

    server_conn_write(host, array);

    return true;
}

/******************************************************************************/

static void
server_source_remove(guint tag)
{
    gboolean status = 0;

    if (tag > 0) {
        status = g_source_remove(tag);
        debug("SERVER", "([%d]) - %s", tag, (status) ? "pass" : "fail");
    }
}

/******************************************************************************/
