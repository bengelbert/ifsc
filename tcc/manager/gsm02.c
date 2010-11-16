#include "wrapper.h"

/******************************************************************************/
/*
 * Constants
 */
#define GSM02_CBCH_LEN      10
#define GSM02_IP_LEN        16

#define GSM02_CMD_RECV_KEEP_ALIVE       0x00
#define GSM02_CMD_RECV_LOC_UPD_REJ      0x02
#define GSM02_CMD_RECV_CALL             0x03
#define GSM02_CMD_RECV_SMS              0x04
#define GSM02_CMD_RECV_DETACH           0x05
#define GSM02_CMD_RECV_LOC_UPD_ACC      0x06
#define GSM02_CMD_RECV_SCANNED_NEIGH    0x08
#define GSM02_CMD_RECV_CONF_REQUEST     0x09

#define GSM02_CMD_SEND_KEEP_ALIVE       0x20
#define GSM02_CMD_SEND_CONF             0x24
#define GSM02_CMD_SEND_STARTUP_CHANNEL  0x23
#define GSM02_CMD_SEND_NEIGHBOURS_LIST  0x26

#define GSM02_CONF_APPLICATION_NGCELL   4
#define GSM02_CONF_CHANNEL_INIT         90
#define GSM02_CONF_CHANNEL_RANGE        5
#define GSM02_CONF_MODE_RASTER          0

#define GSM02_ORIG_ARFCN        GSM02_CONF_CHANNEL_INIT
#define GSM02_STARTUP_CHANNEL   GSM02_CONF_CHANNEL_INIT

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct gsm02_s gsm02_t;
typedef struct gsm02_tags_s gsm02_tags_t;

struct gsm02_s {
    service_t *service;
    gsm02_tags_t *tags;
    GMainLoop *loop;
};

/******************************************************************************/

struct gsm02_tags_s {
    guint send_ping;
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
gsm02_destroy(gpointer user_data);

/**
 * 
 * @param message
 * @return 
 */
static GByteArray *
gsm02_make_startup_channel(GByteArray *message);

/**
 *
 * @param connection
 * @return
 */
static gsm02_t *
gsm02_new(GSocketConnection *connection);

/**
 * 
 * @param user_data
 */
static void
gsm02_recv_pack_handler(gpointer user_data);

/**
 *
 * @param user_data
 * @return
 */
static gboolean
gsm02_send_keep_alive(gpointer user_data);

/**
 * 
 * @param gsm02
 */
static void
gsm02_send_conf(gsm02_t *self);

/**
 * 
 * @param gsm02
 */
static void
gsm02_send_neighbours_list(gsm02_t *self);

/**
 * 
 * @param gsm02
 */
static void
gsm02_send_startup_channel(gsm02_t *self);

/******************************************************************************/

gboolean
gsm02_connect_handler(GThreadedSocketService *service,
        GSocketConnection *connection,
        GSocketListener *listener,
        gpointer user_data)
{
    gsm02_t *obj = NULL;

    g_assert(service);
    g_assert(connection);

    debug("GSM02", "listener(%p) data(%p)", listener, user_data);

    message("GSM02", "EVENT --------- [ CONNECTED ]");

    obj = gsm02_new(connection);
    obj->loop = g_main_loop_new(NULL, FALSE);

    service_input_stream_read_async(obj->service,
            gsm02_recv_pack_handler,
            gsm02_destroy,
            obj);

    obj->tags->send_ping = g_timeout_add(10000, gsm02_send_keep_alive, obj);

    g_main_loop_run(obj->loop);

    return TRUE;
}

/******************************************************************************/

static void
gsm02_destroy(gpointer user_data)
{
    gsm02_t *obj = user_data;

    g_assert(obj);
    g_assert(obj->loop);

    message("GSM02", "EVENT --------- [ DISCONNECTED ]");

    service_free(obj->service);

    g_main_loop_quit(obj->loop);
    g_main_loop_unref(obj->loop);

    g_source_remove(obj->tags->send_ping);

    g_free(obj);
}

/******************************************************************************/

static GByteArray *
gsm02_make_conf(GByteArray *message)
{
    g_assert(message);

    message = service_message_append_16(message, 900);
    message = service_message_append_16(message, GSM02_CONF_CHANNEL_INIT);
    message = service_message_append_16(message, GSM02_CONF_CHANNEL_RANGE);
    message = service_message_append_string_sized(message, "172.23.255.254", GSM02_IP_LEN);
    message = service_message_append_string_sized(message, "172.23.255.254", GSM02_IP_LEN);
    message = service_message_append_string_sized(message, "172.23.255.254", GSM02_IP_LEN);
    message = service_message_append_16(message, 0x0724);
    message = service_message_append_16(message, 0x0019);
    message = service_message_append_16(message, 0x00AA);
    message = service_message_append_16(message, 13);
    message = service_message_append_string_sized(message, "", GSM02_CBCH_LEN);
    message = service_message_append_16(message, GSM02_CONF_APPLICATION_NGCELL);
    message = service_message_append_16(message, 15);
    message = service_message_append_16(message, 1024);
    message = service_message_append_16(message, GSM02_CONF_MODE_RASTER);

    return message;
}

/******************************************************************************/

static GByteArray *
gsm02_make_neighbours_list(GByteArray *message)
{
    g_assert(message);

    message = service_message_append_16(message, 0);
    message = service_message_append_16(message, GSM02_ORIG_ARFCN);

    return message;
}

/******************************************************************************/

static GByteArray *
gsm02_make_startup_channel(GByteArray *message)
{
    g_assert(message);

    message = service_message_append_16(message, GSM02_STARTUP_CHANNEL);

    return message;
}

/******************************************************************************/

static gsm02_t *
gsm02_new(GSocketConnection *connection)
{
    gsm02_t *obj = NULL;

    g_assert(connection);

    obj = g_new0(gsm02_t, 1);

    g_assert(obj);

    obj->service = service_new(connection);
    obj->tags = g_new0(gsm02_tags_t, 1);

    g_assert(obj->service);
    g_assert(obj->tags);

    return obj;
}

/******************************************************************************/

static void
gsm02_recv_pack_handler(gpointer user_data)
{
    gsm02_t *obj = user_data;

    g_assert(obj);

    switch (service_message_header_get_command(obj->service)) {
    case GSM02_CMD_RECV_CALL:
        message("GSM02", "EVENT RECV ---- [ CALL ]");
        break;
    case GSM02_CMD_RECV_CONF_REQUEST:
        gsm02_send_conf(obj);
        break;
    case GSM02_CMD_RECV_DETACH:
        message("GSM02", "EVENT RECV ---- [ DETACH ]");
        break;
    case GSM02_CMD_RECV_KEEP_ALIVE:
        message("GSM02", "EVENT RECV ---- [ KEEP_ALIVE ]");
        break;
    case GSM02_CMD_RECV_LOC_UPD_ACC:
        message("GSM02", "EVENT RECV ---- [ LOC_UPD_ACC ]");
        break;
    case GSM02_CMD_RECV_LOC_UPD_REJ:
        message("GSM02", "EVENT RECV ---- [ LOC_UPD_REJ ]");
        break;
    case GSM02_CMD_RECV_SCANNED_NEIGH:
        gsm02_send_startup_channel(obj);
        gsm02_send_neighbours_list(obj);
        break;
    default:
        warning("GSM02", "Invalid command [ 0x%x ]",
                service_message_header_get_command(obj->service));
    }
}

/******************************************************************************/

static void
gsm02_send_conf(gsm02_t *self)
{
    GByteArray *message = NULL;

    g_assert(self);

    message = g_byte_array_new();

    message = gsm02_make_conf(message);
    message = service_message_header_make(message, GSM02_CMD_SEND_CONF);

    service_output_stream_write(self->service, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static gboolean
gsm02_send_keep_alive(gpointer user_data)
{
    gsm02_t *obj = user_data;
    GByteArray *message = NULL;

    g_assert(obj);

    message("GSM02", "EVENT SEND ---- [ KEEP_ALIVE ]");

    message = g_byte_array_new();

    message = service_message_header_make(message, GSM02_CMD_SEND_KEEP_ALIVE);

    service_output_stream_write(obj->service, message);

    g_byte_array_free(message, TRUE);

    return TRUE;
}

/******************************************************************************/

static void
gsm02_send_neighbours_list(gsm02_t *self)
{
    GByteArray *message = NULL;

    g_assert(self);

    message = g_byte_array_new();

    message = gsm02_make_neighbours_list(message);
    message = service_message_header_make(message, GSM02_CMD_SEND_NEIGHBOURS_LIST);

    service_output_stream_write(self->service, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static void
gsm02_send_startup_channel(gsm02_t *self)
{
    GByteArray *message = NULL;

    g_assert(self);

    message = g_byte_array_new();

    message = gsm02_make_startup_channel(message);
    message = service_message_header_make(message, GSM02_CMD_SEND_STARTUP_CHANNEL);

    service_output_stream_write(self->service, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/