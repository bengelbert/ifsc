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
#define MCU_CMD_RECV_LED_STATE      0x01

#define MCU_CMD_SEND_KEEP_ALIVE         0x20
#define MCU_CMD_SEND_DTMF               0x21
#define MCU_CMD_SEND_STARTUP_CHANNEL    0x23
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
 * @param dtmf
 * @return
 */
static GByteArray *
mcu_make_dtmf(GByteArray *message,
        guint dtmf);

/**
 *
 * @param connection
 * @return
 */
static mcu_t *
mcu_new(GSocketConnection *connection);

/**
 * 
 * @param this
 */
static void
mcu_recv_led_state_handler(mcu_t *this);

/**
 *
 * @param user_data
 */
static void
mcu_recv_pack_handler(gpointer user_data);

/**
 * 
 * @param self
 * @param dtmf
 */
static void
mcu_send_dtmf(mcu_t *self,
        guint dtmf);

/**
 *
 * @param user_data
 * @return
 */
static gboolean
mcu_send_keep_alive(gpointer user_data);

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
mcu_async_queue_push(guint data)
{
    guint *dtmf = g_new0(guint , 1);

    *dtmf = data;

    g_async_queue_push(mcu_queue, dtmf);
}

/******************************************************************************/

void
mcu_async_queue_init(void)
{
    mcu_queue = g_async_queue_new();

    g_assert(mcu_queue);
}

/******************************************************************************/

static gboolean
mcu_async_try_pop(gpointer user_data)
{
    mcu_t *obj = user_data;
    guint *dtmf = NULL;

    dtmf = g_async_queue_try_pop(mcu_queue);

    if (dtmf) {
        mcu_send_dtmf(obj, *dtmf);
        g_free(dtmf);
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

    obj->tags->pop_queue = g_timeout_add(100, mcu_async_try_pop, obj);
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

    g_source_remove(obj->tags->pop_queue);
    g_source_remove(obj->tags->send_ping);

    g_free(obj);
}

/******************************************************************************/

static GByteArray *
mcu_make_dtmf(GByteArray *message,
        guint dtmf)
{
    g_assert(message);

    message = service_message_append_16(message, dtmf);

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
mcu_recv_led_state_handler(mcu_t *this)
{
    guint ad = 0;
    guint8 *payload;

    payload = service_message_get_payload(this->service);

    service_message_unpack_u16(&ad, payload);

    if (ad) {
        message("MCU", "EVENT RECV ---- [ LED ON (%d) ]", ad);
        gsm02_async_queue_push("2006", "2010", "O Led esta ligado!");
    } else {
        message("MCU", "EVENT RECV ---- [ LED OFF (%d) ]", ad);
        gsm02_async_queue_push("2006", "2010", "O Led esta desligado!");
    }
}

/******************************************************************************/

static void
mcu_recv_pack_handler(gpointer user_data)
{
    mcu_t *obj = user_data;

    g_assert(obj);

    switch (service_message_header_get_command(obj->service)) {
    case MCU_CMD_RECV_LED_STATE:
        mcu_recv_led_state_handler(obj);
        break;

    case MCU_CMD_RECV_KEEP_ALIVE:
        message("MCU", "EVENT RECV ---- [ KEEP_ALIVE ]");
        break;

    default:
        warning("MCU", "Invalid command [ 0x%x ]",
                service_message_header_get_command(obj->service));
    }
}

/******************************************************************************/

static void
mcu_send_dtmf(mcu_t *self,
        guint dtmf)
{
    GByteArray *message = NULL;

    g_assert(self);

    message("MCU", "EVENT SEND ---- [ DTMF (%02d) ]", dtmf);

    message = g_byte_array_new();

    message = mcu_make_dtmf(message, dtmf);
    message = service_message_header_make(message, MCU_CMD_SEND_DTMF);

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

