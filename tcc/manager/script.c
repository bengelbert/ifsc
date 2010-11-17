#include "wrapper.h"

/******************************************************************************/
/*
 * Constants
 */
#define SCRIPT_CMD_RECV_DTMF       0x00

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct script_s script_t;

struct script_s {
    service_t *service;
    GMainLoop *loop;
};

/******************************************************************************/
/*
 * Function prototypes
 */
/**
 *
 * @param user_data
 */
static void
script_destroy(gpointer user_data);

/**
 *
 * @param connection
 * @return
 */
static script_t *
script_new(GSocketConnection *connection);

/**
 *
 * @param user_data
 */
static void
script_recv_pack_handler(gpointer user_data);

/******************************************************************************/
/*
 * Function definitions
 */
gboolean
script_connect_handler(GThreadedSocketService *service,
        GSocketConnection *connection,
        GSocketListener *listener,
        gpointer user_data)
{
    script_t *obj = NULL;

    g_assert(service);
    g_assert(connection);

    debug("SCRIPT", "listener(%p) data(%p)", listener, user_data);

    message("SCRIPT", "EVENT --------- [ CONNECTED ]");

    gsm02_async_queue_push("4488", "99102820", "Olha a mensagem");

    obj = script_new(connection);
    obj->loop = g_main_loop_new(NULL, FALSE);

    service_input_stream_read_async(obj->service,
            script_recv_pack_handler,
            script_destroy,
            obj);

    g_main_loop_run(obj->loop);

    return TRUE;
}

/******************************************************************************/

static void
script_destroy(gpointer user_data)
{
    script_t *obj = user_data;

    g_assert(obj);
    g_assert(obj->loop);

    message("SCRIPT", "EVENT --------- [ DISCONNECTED ]");

    service_free(obj->service);

    g_main_loop_quit(obj->loop);
    g_main_loop_unref(obj->loop);

    g_free(obj);
}

/******************************************************************************/

static script_t *
script_new(GSocketConnection *connection)
{
    script_t *obj = NULL;

    g_assert(connection);

    obj = g_new0(script_t, 1);

    g_assert(obj);

    obj->service = service_new(connection);

    g_assert(obj->service);

    return obj;
}

/******************************************************************************/

static void
script_recv_pack_handler(gpointer user_data)
{
    script_t *obj = user_data;

    g_assert(obj);

    switch (service_message_header_get_command(obj->service)) {
    case SCRIPT_CMD_RECV_DTMF:
        message("SCRIPT", "EVENT RECV ---- [ DTMF ]");
        break;
    default:
        warning("SCRIPT", "Invalid command [ 0x%x ]",
                service_message_header_get_command(obj->service));
    }
}

/******************************************************************************/
