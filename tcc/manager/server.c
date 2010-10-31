/*
 * application include files
 */
#include "wrapper.h"

/*
 * Type definitions
 */
typedef struct server_s             server_t;
typedef struct server_net_s         server_net_t;
typedef struct server_source_id_s   server_source_id_t;

/******************************************************************************/

struct server_s {
    Global *                global;
    server_net_t *          net;
    server_source_id_t *    source_id;
};

/******************************************************************************/

struct server_net_s {
    GConn *         conn;
    GConnEvent *    event;
    GServer *       server;
};

/******************************************************************************/

struct server_source_id_s {
    guint connect;
    guint ping;
};

/******************************************************************************/
/*
 * Function prototypes
 */
static void         server_cb           (GServer * server, GConn * conn, gpointer data);
static void         server_client_cb    (GConn * conn, GConnEvent * event, gpointer data);
static void         server_conn_connect (server_t * server);
static void         server_conn_delete  (server_t * server);
static void         server_conn_read    (server_t * server);
static server_t *   server_new          (Global * global);
static gboolean     server_send_ping    (gpointer data);
static void         server_source_remove(guint tag);

/******************************************************************************/
/*
 * Function definitions
 */
static void
server_cb(GServer * server, GConn * conn, gpointer data)
{
    server_t * dbfc = NULL;

    g_assert(data != NULL);

    dbfc = (server_t *) data;
    dbfc->net->conn = conn;

    if (conn != NULL) {
        gnet_conn_set_callback(conn, server_client_cb, data);
        gnet_conn_set_watch_error(conn, true);
        gnet_conn_read(conn);

        dbfc->source_id->ping = common_timeout_add("PING", 1000, server_send_ping, dbfc);
        
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
    server_t * server = NULL;

    g_assert(conn);
    g_assert(data);
    g_assert(event);

    debug("SERVER", "([%p], [%p], [%p]): event->type: [%d]", conn, event, data, event->type);

    server = (server_t *) data;
    server->net->event = event;

    switch (event->type) {
    case GNET_CONN_CONNECT: server_conn_connect(server);    break;
    case GNET_CONN_READ:    server_conn_read(server);       break;
    case GNET_CONN_WRITE:   break;
    case GNET_CONN_CLOSE:
    case GNET_CONN_TIMEOUT:
    case GNET_CONN_ERROR:   server_conn_delete(server);         break;
    default:
        g_assert_not_reached();
    }

    return;
}

/******************************************************************************/

static void
server_conn_connect(server_t * server)
{
    g_assert(server);

    debug("SERVER", "([%p])", server);

    server_source_remove(server->source_id->connect);

    gnet_conn_timeout(server->net->conn, 0);

    gnet_conn_read(server->net->conn);

    return;
}

/******************************************************************************/

static void
server_conn_delete(server_t * server)
{
    g_assert(server);

    debug("SERVER", "([%p])", server);

    server_source_remove(server->source_id->connect);
    server_source_remove(server->source_id->ping);

    gnet_conn_delete(server->net->conn);
    server->net->conn = NULL;

    return;
}

/******************************************************************************/

static void
server_conn_read(server_t * server)
{
    guchar buffer[] = {0x00, 0x20, 0x00, 0x01, 0x20, 'b', 'r','u', 'n', '\n'};
    g_assert(server);

    message("SERVER", "Chegou dados");
    gnet_conn_read(server->net->conn);

    gnet_conn_write(server->net->conn, buffer, sizeof (buffer));

    return;
}

/******************************************************************************/

void
server_init(Global * global)
{
    server_t * server = NULL;

    server = server_new(global);

    server->net->server = gnet_server_new(NULL, 9099, server_cb, server);

    return;
}

/******************************************************************************/

static server_t *
server_new(Global * global)
{
    server_t * server = NULL;

    server = g_new0(server_t, 1);

    server->global      = global;
    server->net         = g_new0(server_net_t, 1);
    server->source_id   = g_new0(server_source_id_t, 1);

    debug("SERVER", "()->global: [%p]", server->global);
    debug("SERVER", "()->net: [%p]", server->net);
    debug("SERVER", "()->source_id: [%p]", server->source_id);

    return server;
}

/******************************************************************************/

static gboolean
server_send_ping(gpointer data)
{
    g_assert(data);

    message("SERVER", "Enviar ping!!");

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

    return;
}

/******************************************************************************/
