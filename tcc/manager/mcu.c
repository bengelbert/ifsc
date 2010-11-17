#include "wrapper.h"

/******************************************************************************/
/*
 * Constants
 */
#define MCU_CBCH_LEN        10
#define MCU_IP_LEN          16
#define MCU_SMS_TO_LEN      50
#define MCU_SMS_FROM_LEN    50
#define MCU_SMS_MSG_LEN     160

#define MCU_CMD_RECV_KEEP_ALIVE     0x00
#define MCU_CMD_RECV_LOC_UPD_REJ    0x02
#define MCU_CMD_RECV_CALL           0x03
#define MCU_CMD_RECV_SMS            0x04
#define MCU_CMD_RECV_DETACH         0x05
#define MCU_CMD_RECV_LOC_UPD_ACC    0x06
#define MCU_CMD_RECV_SCANNED_NEIGH  0x08
#define MCU_CMD_RECV_CONF_REQUEST   0x09

#define MCU_CMD_SEND_KEEP_ALIVE         0x20
#define MCU_CMD_SEND_STARTUP_CHANNEL    0x23
#define MCU_CMD_SEND_CONF               0x24
#define MCU_CMD_SEND_NEIGHBOURS_LIST    0x26
#define MCU_CMD_SEND_SMS                0x27

#define MCU_CONF_APPLICATION_NGCELL 4
#define MCU_CONF_CHANNEL_INIT       90
#define MCU_CONF_CHANNEL_RANGE      5
#define MCU_CONF_MODE_RASTER        0

#define MCU_ORIG_ARFCN        MCU_CONF_CHANNEL_INIT
#define MCU_STARTUP_CHANNEL   MCU_CONF_CHANNEL_INIT

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct mcu_s mcu_t;
typedef struct mcu_queue_s mcu_async_queue_t;
typedef struct mcu_tags_s mcu_tags_t;

struct mcu_s {
    GAsyncQueue *queue;
    service_t *service;
    mcu_tags_t *tags;
    GMainLoop *loop;
};

/******************************************************************************/

struct mcu_queue_s {
    gchar *from;
    gchar *to;
    gchar *msg;
};

/******************************************************************************/

struct mcu_tags_s {
    guint pop_queue;
    guint send_ping;
};

/******************************************************************************/
/*
 * Function prototypes
 */

/**
 *
 * @param to
 * @param from
 * @param msg
 * @return
 */
static mcu_async_queue_t *
mcu_async_queue_new(gchar *to,
        gchar *from,
        gchar *msg);

/**
 *
 * @param user_data
 * @return
 */
static gboolean
mcu_async_try_pop(gpointer user_data);

/**
 *
 * @param user_data
 */
static void
mcu_destroy(gpointer user_data);

/**
 *
 * @param message
 * @return
 */
static GByteArray *
mcu_make_conf(GByteArray *message);

/**
 *
 * @param message
 * @return
 */
static GByteArray *
mcu_make_neighbours_list(GByteArray *message);

/**
 *
 * @param message
 * @param sms_queue
 * @return
 */
static GByteArray *
mcu_make_sms(GByteArray *message,
        mcu_async_queue_t *sms_queue);

/**
 *
 * @param message
 * @return
 */
static GByteArray *
mcu_make_startup_channel(GByteArray *message);

/**
 *
 * @param connection
 * @return
 */
static mcu_t *
mcu_new(GSocketConnection *connection);

/**
 *
 * @param queue
 */
static void
mcu_queue_free(mcu_async_queue_t *queue);

/**
 *
 * @param user_data
 */
static void
mcu_recv_pack_handler(gpointer user_data);

/**
 *
 * @param user_data
 * @return
 */
static gboolean
mcu_send_keep_alive(gpointer user_data);

/**
 *
 * @param mcu
 */
static void
mcu_send_conf(mcu_t *self);

/**
 *
 * @param mcu
 */
static void
mcu_send_neighbours_list(mcu_t *self);

/**
 *
 * @param self
 * @param sms_queue
 */
static void
mcu_send_sms(mcu_t *self,
        mcu_async_queue_t *sms_queue);

/**
 *
 * @param mcu
 */
static void
mcu_send_startup_channel(mcu_t *self);

/******************************************************************************/
/*
 * Private variables
 */
static GAsyncQueue *mcu_queue = NULL;

/******************************************************************************/

/*
 * Function definitions
 */
void
mcu_async_queue_push(gchar *to,
        gchar *from,
        gchar *msg)
{
    g_assert(from);
    g_assert(msg);
    g_assert(to);

    g_async_queue_push(mcu_queue,
            mcu_async_queue_new(to, from, msg));
}

/******************************************************************************/

void
mcu_async_queue_init(void)
{
    mcu_queue = g_async_queue_new();

    g_assert(mcu_queue);
}

/******************************************************************************/

static mcu_async_queue_t *
mcu_async_queue_new(gchar *to,
        gchar *from,
        gchar *msg)
{
    mcu_async_queue_t *queue = NULL;

    g_assert(from);
    g_assert(msg);
    g_assert(to);

    queue = g_new0(mcu_async_queue_t, 1);

    queue->from = g_strdup(from);
    queue->msg = g_strdup(msg);
    queue->to = g_strdup(to);

    return queue;
}

/******************************************************************************/

static gboolean
mcu_async_try_pop(gpointer user_data)
{
    mcu_t *obj = user_data;
    mcu_async_queue_t *queue = NULL;

    queue = g_async_queue_try_pop(mcu_queue);

    if (queue) {
        message("MCU", "QUEUE --------- [ SMS (%s => %s: %s) ]",
                queue->from,
                queue->to,
                queue->msg);

        mcu_send_sms(obj, queue);
        mcu_queue_free(queue);
    } else {
        debug("MCU", "QUEUE --------- [ EMPTY ]");
    }

    return TRUE;
}

/******************************************************************************/

gboolean
mcu_connect_handler(GThreadedSocketService *service,
        GSocketConnection *connection,
        GSocketListener *listener,
        gpointer user_data)
{
    mcu_t *obj = NULL;

    g_assert(service);
    g_assert(connection);

    debug("MCU", "listener(%p) data(%p)", listener, user_data);

    message("MCU", "EVENT --------- [ CONNECTED ]");

    obj = mcu_new(connection);
    obj->loop = g_main_loop_new(NULL, FALSE);

    service_input_stream_read_async(obj->service,
            mcu_recv_pack_handler,
            mcu_destroy,
            obj);

    obj->tags->pop_queue = g_timeout_add(1000, mcu_async_try_pop, obj);
    obj->tags->send_ping = g_timeout_add(10000, mcu_send_keep_alive, obj);

    g_main_loop_run(obj->loop);

    return TRUE;
}

/******************************************************************************/

static void
mcu_destroy(gpointer user_data)
{
    mcu_t *obj = user_data;

    g_assert(obj);
    g_assert(obj->loop);

    message("MCU", "EVENT --------- [ DISCONNECTED ]");

    service_free(obj->service);

    g_main_loop_quit(obj->loop);
    g_main_loop_unref(obj->loop);

    g_source_remove(obj->tags->send_ping);

    g_free(obj);
}

/******************************************************************************/

static GByteArray *
mcu_make_conf(GByteArray *message)
{
    g_assert(message);

    message = service_message_append_16(message, 900);
    message = service_message_append_16(message, MCU_CONF_CHANNEL_INIT);
    message = service_message_append_16(message, MCU_CONF_CHANNEL_RANGE);
    message = service_message_append_string_sized(message, "172.23.255.254", MCU_IP_LEN);
    message = service_message_append_string_sized(message, "172.23.255.254", MCU_IP_LEN);
    message = service_message_append_string_sized(message, "172.23.255.254", MCU_IP_LEN);
    message = service_message_append_16(message, 0x0724);
    message = service_message_append_16(message, 0x0019);
    message = service_message_append_16(message, 0x00AA);
    message = service_message_append_16(message, 13);
    message = service_message_append_string_sized(message, "", MCU_CBCH_LEN);
    message = service_message_append_16(message, MCU_CONF_APPLICATION_NGCELL);
    message = service_message_append_16(message, 15);
    message = service_message_append_16(message, 1024);
    message = service_message_append_16(message, MCU_CONF_MODE_RASTER);

    return message;
}

/******************************************************************************/

static GByteArray *
mcu_make_neighbours_list(GByteArray *message)
{
    g_assert(message);

    message = service_message_append_16(message, 0);
    message = service_message_append_16(message, MCU_ORIG_ARFCN);

    return message;
}

/******************************************************************************/

static GByteArray *
mcu_make_sms(GByteArray *message,
        mcu_async_queue_t *sms_queue)
{
    g_assert(message);
    g_assert(sms_queue);

    message = service_message_append_string(message,
            sms_queue->to,
            MCU_SMS_TO_LEN);

    message = service_message_append_string(message,
            sms_queue->from,
            MCU_SMS_FROM_LEN);

    message = service_message_append_string(message,
            sms_queue->msg,
            MCU_SMS_MSG_LEN);

    return message;
}

/******************************************************************************/

static GByteArray *
mcu_make_startup_channel(GByteArray *message)
{
    g_assert(message);

    message = service_message_append_16(message, MCU_STARTUP_CHANNEL);

    return message;
}

/******************************************************************************/

static mcu_t *
mcu_new(GSocketConnection *connection)
{
    mcu_t *obj = NULL;

    g_assert(connection);

    obj = g_new0(mcu_t, 1);

    g_assert(obj);

    obj->service = service_new(connection);
    obj->tags = g_new0(mcu_tags_t, 1);

    g_assert(obj->service);
    g_assert(obj->tags);

    return obj;
}

/******************************************************************************/

static void
mcu_queue_free(mcu_async_queue_t *queue)
{
    g_assert(queue);

    g_free(queue->from);
    g_free(queue->msg);
    g_free(queue->to);
    g_free(queue);
}

/******************************************************************************/

static void
mcu_recv_pack_handler(gpointer user_data)
{
    mcu_t *obj = user_data;

    g_assert(obj);

    switch (service_message_header_get_command(obj->service)) {
    case MCU_CMD_RECV_CALL:
        message("MCU", "EVENT RECV ---- [ CALL ]");
        break;
    case MCU_CMD_RECV_CONF_REQUEST:
        mcu_send_conf(obj);
        break;
    case MCU_CMD_RECV_DETACH:
        message("MCU", "EVENT RECV ---- [ DETACH ]");
        break;
    case MCU_CMD_RECV_KEEP_ALIVE:
        message("MCU", "EVENT RECV ---- [ KEEP_ALIVE ]");
        break;
    case MCU_CMD_RECV_LOC_UPD_ACC:
        message("MCU", "EVENT RECV ---- [ LOC_UPD_ACC ]");
        break;
    case MCU_CMD_RECV_LOC_UPD_REJ:
        message("MCU", "EVENT RECV ---- [ LOC_UPD_REJ ]");
        break;
    case MCU_CMD_RECV_SCANNED_NEIGH:
        mcu_send_startup_channel(obj);
        mcu_send_neighbours_list(obj);
        break;
    default:
        warning("MCU", "Invalid command [ 0x%x ]",
                service_message_header_get_command(obj->service));
    }
}

/******************************************************************************/

static void
mcu_send_conf(mcu_t *self)
{
    GByteArray *message = NULL;

    g_assert(self);

    message("MCU", "EVENT SEND ---- [ CONF ]");

    message = g_byte_array_new();

    message = mcu_make_conf(message);
    message = service_message_header_make(message, MCU_CMD_SEND_CONF);

    service_output_stream_write(self->service, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static gboolean
mcu_send_keep_alive(gpointer user_data)
{
    mcu_t *obj = user_data;
    GByteArray *message = NULL;

    g_assert(obj);

    message("MCU", "EVENT SEND ---- [ KEEP_ALIVE ]");

    message = g_byte_array_new();

    message = service_message_header_make(message, MCU_CMD_SEND_KEEP_ALIVE);

    service_output_stream_write(obj->service, message);

    g_byte_array_free(message, TRUE);

    return TRUE;
}

/******************************************************************************/

static void
mcu_send_neighbours_list(mcu_t *self)
{
    GByteArray *message = NULL;

    g_assert(self);

    message("MCU", "EVENT SEND ---- [ NEIGHBOURS_LIST ]");

    message = g_byte_array_new();

    message = mcu_make_neighbours_list(message);
    message = service_message_header_make(message, MCU_CMD_SEND_NEIGHBOURS_LIST);

    service_output_stream_write(self->service, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static void
mcu_send_sms(mcu_t *self,
        mcu_async_queue_t *sms_queue)
{
    GByteArray *message = NULL;

    g_assert(self);
    g_assert(sms_queue);

    message("MCU", "EVENT SEND ---- [ SMS ]");

    message = g_byte_array_new();

    message = mcu_make_sms(message, sms_queue);
    message = service_message_header_make(message, MCU_CMD_SEND_SMS);

    service_output_stream_write(self->service, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static void
mcu_send_startup_channel(mcu_t *self)
{
    GByteArray *message = NULL;

    g_assert(self);

    message("MCU", "EVENT SEND ---- [ STARTUP_CHANNEL ]");

    message = g_byte_array_new();

    message = mcu_make_startup_channel(message);
    message = service_message_header_make(message, MCU_CMD_SEND_STARTUP_CHANNEL);

    service_output_stream_write(self->service, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

