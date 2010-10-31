/******************************************************************************/
/*
 * Application include files
 */
#include "wrapper.h"

/******************************************************************************/
/*
 * Constants
 */
#define CLIENT_CMD_PING     0x00
#define CLIENT_HEADER_LEN   15
#define CLIENT_ID           0xAA71
#define CLIENT_VERSION      2

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct _Client          Client;
typedef struct _ClientNet       ClientNet;
typedef struct _ClientSourceId  ClientSourceId;

/******************************************************************************/

struct _Client {
    Global *            global;
    ClientNet *         net;
    ClientSourceId *    source_id;
};

/******************************************************************************/

struct _ClientNet {
    GConn *         conn;
    GConnEvent *    event;
};

/******************************************************************************/

struct _ClientSourceId {
    guint connect;
    guint ping;
};

/******************************************************************************/
/*
 * Function prototypes
 */
static void     client_cb           (GConn * __conn, GConnEvent * __event, gpointer __data);
static void     client_conn_connect (Client * __client);
static void     client_conn_delete  (Client * __client);
static void     client_conn_read    (Client * __client);
static gboolean client_connect      (gpointer __data);
static void     client_make_header  (guchar __cmd, gchar * __buffer, guint __len);
static Client * client_new          (Global * __global);
static gboolean client_send_ping    (gpointer __data);
static void     client_source_remove(guint __tag);

/******************************************************************************/
/*
 * Function definitions
 */
static void
client_cb(GConn *       __conn,
        GConnEvent *    __event,
        gpointer        __data)
{
    Client * dbfc = NULL;

    g_assert(__conn);
    g_assert(__data);
    g_assert(__event);

    debug("CLIENT", "([%p], [%p], [%p]): event->type: [%d]", 
                    __conn,
                    __event,
                    __data,
                    __event->type);

    dbfc = (Client *) __data;
    dbfc->net->event = __event;

    switch (__event->type) {
    case GNET_CONN_CONNECT: client_conn_connect(dbfc);  break;
    case GNET_CONN_READ:    client_conn_read(dbfc);     break;
    case GNET_CONN_WRITE:   break;
    case GNET_CONN_CLOSE:
    case GNET_CONN_TIMEOUT:
    case GNET_CONN_ERROR:   client_conn_delete(dbfc);   break;
    default:
        g_assert_not_reached();
    }

    return;
}

/******************************************************************************/

void
client_init(Global * __global)
{
    Client * dbfc = NULL;

    debug("CLIENT", "([%p])", __global);

    dbfc = client_new(__global);

    client_connect(dbfc);

    return;
}

/******************************************************************************/

static void
client_conn_connect(Client * __client)
{
    g_assert(__client);
    
    debug("CLIENT", "([%p])", __client);

    client_source_remove(__client->source_id->connect);
    
    gnet_conn_timeout(__client->net->conn, 0);

    gnet_conn_read(__client->net->conn);

    __client->source_id->ping = common_timeout_add("PING",
                                                    global_get_client_ping_interval(__client->global),
                                                    client_send_ping,
                                                    __client);
    
    return;
}

/******************************************************************************/

static void
client_conn_delete(Client * __client)
{
    g_assert(__client);

    debug("CLIENT", "([%p])", __client);
    
    client_source_remove(__client->source_id->connect);
    client_source_remove(__client->source_id->ping);

    gnet_conn_delete(__client->net->conn);
    __client->net->conn = NULL;

    __client->source_id->connect = common_timeout_add("CONNECT",
                                                        3000,
                                                        client_connect,
                                                        __client);

    return;
}

/******************************************************************************/

static void
client_conn_read(Client * __client)
{
    g_assert(__client);

    gnet_conn_read(__client->net->conn);

    return;
}

/******************************************************************************/

static gboolean
client_connect(gpointer __data)
{
    Client * dbfc = NULL;

    g_assert(__data);

    dbfc = (Client *) __data;
    
    debug("CLIENT", "([%p])", dbfc);

    dbfc->net->conn = gnet_conn_new("localhost",
                                    global_get_client_port(dbfc->global),
                                    client_cb,
                                    dbfc);

    debug("CLIENT", "()->net->conn: [%p]", dbfc->net->conn);

    gnet_conn_set_watch_error(dbfc->net->conn, true);
    gnet_conn_timeout(dbfc->net->conn, 3000);
    gnet_conn_connect(dbfc->net->conn);

    return true;
}

/******************************************************************************/

static void
client_make_header(guchar   __cmd,
                gchar *     __buffer,
                guint       __len)
{
    guint pack_len;
    
    pack_len = __len + CLIENT_HEADER_LEN;

    gnet_pack("!hhbiih", __buffer, CLIENT_HEADER_LEN,
            pack_len,
            CLIENT_VERSION,
            __cmd,
            time(NULL),
            time(NULL),
            CLIENT_ID);

    common_print_dump(__buffer, pack_len);
}

/******************************************************************************/

static Client *
client_new(Global * __global)
{
    Client * client = NULL;

    client = g_new0(Client, 1);

    client->global      = __global;
    client->net         = g_new0(ClientNet, 1);
    client->source_id   = g_new0(ClientSourceId, 1);

    debug("CLIENT", "()->global: [%p]", client->global);
    debug("CLIENT", "()->net: [%p]", client->net);
    debug("CLIENT", "()->source_id: [%p]", client->source_id);

    return client;
}

/******************************************************************************/

static gboolean
client_send_ping(gpointer __data)
{
    gchar buffer[COMMON_BUFFER_LEN];

    g_assert(__data);

    debug("CLIENT", "([%p])", __data);

    client_make_header(CLIENT_CMD_PING, buffer, 0);

     return true;
}

/******************************************************************************/

static void
client_source_remove(guint __tag)
{
    gboolean status = 0;

    if (__tag > 0) {
        status = g_source_remove(__tag);
        debug("CLIENT", "([%d]) - %s", __tag, (status) ? "pass" : "fail");
    } 
    
    return;
}

/******************************************************************************/
