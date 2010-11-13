#include "wrapper.h"

/******************************************************************************/
/*
 * Constants
 */
#define GSM02_BUFFER_LEN    4096
#define GSM02_CBCH_LEN      10
#define GSM02_IP_LEN        16

#define GSM02_CMD_RECV_KEEP_ALIVE       0x00
#define GSM02_CMD_RECV_SCANNED_NEIGH    0x08
#define GSM02_CMD_RECV_CONF_REQUEST     0x09

#define GSM02_CMD_SEND_KEEP_ALIVE       0x20
#define GSM02_CMD_SEND_CONF             0x24
#define GSM02_CMD_SEND_STARTUP_CHANNEL  0x23
#define GSM02_CMD_SEND_NEIGHBOURS_LIST  0x26

#define GSM02_CONF_APPLICATION_NGCELL   4
#define GSM02_CONF_CHANNEL_INIT         588
#define GSM02_CONF_CHANNEL_RANGE        5
#define GSM02_CONF_MODE_RASTER          0

#define GSM02_ORIG_ARFCN        GSM02_CONF_CHANNEL_INIT
#define GSM02_STARTUP_CHANNEL   GSM02_CONF_CHANNEL_INIT

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct gsm02_s gsm02_t;
typedef struct gsm02_buffer_s gsm02_buffer_t;
typedef struct gsm02_packet_header_s gsm02_packet_header_t;
typedef struct gsm02_stream_s gsm02_stream_t;
typedef struct gsm02_tags_s gsm02_tags_t;

struct gsm02_s {
    gsm02_buffer_t *buffer;
    gsm02_stream_t *stream;
    gsm02_tags_t *tags;
    GMainLoop *loop;
};

/******************************************************************************/

struct gsm02_buffer_s {
    guchar *in;
    gssize in_offset;
};

/******************************************************************************/

struct gsm02_packet_header_s {
    guint16 n_bytes;
    guint16 version;
    guint8 cmd;
    guint32 time_event;
    guint32 time_send;
    guint16 id;
} PACKED;

/******************************************************************************/

struct gsm02_stream_s {
    GInputStream *in;
    GOutputStream *out;
};

/******************************************************************************/

struct gsm02_tags_s {
    guint send_ping;
};

/******************************************************************************/
/*
 * Macros
 */
#define GSM02_PACKET_HEADER_LEN (sizeof(gsm02_packet_header_t))

/******************************************************************************/
/*
 * Function prototypes
 */
/**
 * 
 * @return 
 */
static gsm02_buffer_t *
gsm02_buffer_new(void);

/**
 * 
 * @param buffer
 * @param len
 */
static void
gsm02_data_dump(guchar *buffer,
        gssize len);

/**
 * 
 * @param gsm02
 */
static void
gsm02_destroy(gsm02_t *gsm02);

/**
 * 
 * @param gsm02
 * @param offset
 */
static void
gsm02_input_stream_read_async(gsm02_t *gsm02);

/**
 * 
 * @param message
 * @param cmd
 * @param payload_len
 * @return
 */
static GByteArray *
gsm02_make_header(GByteArray * message,
        guchar cmd);

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
 * @param message
 * @param data
 * @return
 */
static GByteArray *
gsm02_packet_append_16(GByteArray *message,
        guint data);

/**
 * 
 * @param message
 * @param data
 * @param len
 * @return
 */
static GByteArray *
gsm02_packet_append_string_sized(GByteArray *message,
        gchar *data,
        gsize len);

/**
 * 
 * @param source
 * @param result
 * @param user_data
 */
static void
gsm02_read_data(GObject *source,
        GAsyncResult *result,
        gpointer user_data);

/**
 * 
 * @param gsm02
 * @param command
 */
static void
gsm02_recv_pack_handler(gsm02_t *gsm02,
        guchar command);

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
gsm02_send_conf(gsm02_t *gsm02);

/**
 * 
 * @param gsm02
 */
static void
gsm02_send_neighbours_list(gsm02_t *gsm02);

/**
 * 
 * @param gsm02
 */
static void
gsm02_send_startup_channel(gsm02_t *gsm02);

/**
 * 
 * @param stream
 */
static void
gsm02_stream_free(gsm02_stream_t *stream);

/**
 * 
 * @param connection
 * @return 
 */
static gsm02_stream_t *
gsm02_stream_new(GSocketConnection *connection);

/**
 * 
 * @param buffer
 * @return
 */
static gsm02_packet_header_t *
gsm02_unpack_header(gpointer buffer);

/******************************************************************************/

static void
gsm02_buffer_free(gsm02_buffer_t *buffer)
{
    g_assert(buffer);

    if (buffer->in) {
        g_free(buffer->in);
    }
}

/******************************************************************************/

static gsm02_buffer_t *
gsm02_buffer_new(void)
{
    gsm02_buffer_t *buffer = NULL;

    buffer = g_new0(gsm02_buffer_t, 1);
    g_assert(buffer);

    buffer->in = g_new0(guchar, GSM02_BUFFER_LEN);
    g_assert(buffer->in);

    return buffer;
}

/******************************************************************************/

gboolean
gsm02_connect_handler(GThreadedSocketService *service,
        GSocketConnection *connection,
        GSocketListener *listener,
        gpointer user_data)
{
    gsm02_t *gsm02 = NULL;

    g_assert(service);
    g_assert(connection);

    debug("GSM02", "listener(%p) data(%p)", listener, user_data);

    gsm02 = gsm02_new(connection);
    gsm02->loop = g_main_loop_new(NULL, FALSE);

    gsm02_input_stream_read_async(gsm02);

    gsm02->tags->send_ping = g_timeout_add(10000, gsm02_send_keep_alive, gsm02);

    g_main_loop_run(gsm02->loop);

    return TRUE;
}

/******************************************************************************/

static void
gsm02_data_dump(guchar *buffer,
        gssize len)
{
    gchar dump[4096];
    gint i, j;
    gint dec = 0;

    g_assert(buffer);

    memset(dump, 0, sizeof (dump));
    for (i = 0, j = 0; i < len; i++, j += 3) {

        if (!(i % 10)) {
            g_snprintf(&dump[j + dec], 2, "\n");
            dec += 1;
        }

        g_snprintf(&dump[j + dec], 4, "%02X ", buffer[i]);
    }

    debug("GSM02", "\n%s\n", dump);
}

/******************************************************************************/

static void
gsm02_destroy(gsm02_t *gsm02)
{
    g_assert(gsm02);
    g_assert(gsm02->loop);

    gsm02_buffer_free(gsm02->buffer);
    gsm02_stream_free(gsm02->stream);

    g_main_loop_quit(gsm02->loop);
    g_main_loop_unref(gsm02->loop);

    g_source_remove(gsm02->tags->send_ping);

    g_free(gsm02);
}

/******************************************************************************/

static void
gsm02_input_stream_read_async(gsm02_t *gsm02)
{
    g_assert(gsm02);
    g_assert(gsm02->stream);
    g_assert(gsm02->stream->in);

    g_input_stream_read_async(gsm02->stream->in,
            gsm02->buffer->in,
            GSM02_BUFFER_LEN,
            G_PRIORITY_DEFAULT,
            NULL,
            gsm02_read_data,
            gsm02);
}

/******************************************************************************/

static GByteArray *
gsm02_make_conf(GByteArray *message)
{
    g_assert(message);

    message = gsm02_packet_append_16(message, 1800);
    message = gsm02_packet_append_16(message, GSM02_CONF_CHANNEL_INIT);
    message = gsm02_packet_append_16(message, GSM02_CONF_CHANNEL_RANGE);
    message = gsm02_packet_append_string_sized(message, "172.23.255.254", GSM02_IP_LEN);
    message = gsm02_packet_append_string_sized(message, "172.23.255.254", GSM02_IP_LEN);
    message = gsm02_packet_append_string_sized(message, "172.23.255.254", GSM02_IP_LEN);
    message = gsm02_packet_append_16(message, 0x0724);
    message = gsm02_packet_append_16(message, 0x0019);
    message = gsm02_packet_append_16(message, 0x00AA);
    message = gsm02_packet_append_16(message, 13);
    message = gsm02_packet_append_string_sized(message, "", GSM02_CBCH_LEN);
    message = gsm02_packet_append_16(message, GSM02_CONF_APPLICATION_NGCELL);
    message = gsm02_packet_append_16(message, 15);
    message = gsm02_packet_append_16(message, 1024);
    message = gsm02_packet_append_16(message, GSM02_CONF_MODE_RASTER);

    return message;
}

/******************************************************************************/

static GByteArray *
gsm02_make_header(GByteArray * message,
        guchar cmd)
{
    gsm02_packet_header_t header = {0, 0, 0, 0, 0, 0};

    g_assert(message);

    header.cmd = cmd;
    header.id = 0;
    header.n_bytes = g_htons(GSM02_PACKET_HEADER_LEN + message->len);
    header.time_event = g_htonl(time(NULL));
    header.time_send = g_htonl(time(NULL));
    header.version = g_htons(2);

    message = g_byte_array_prepend(message, (guint8 *) & header, GSM02_PACKET_HEADER_LEN);

    return message;
}

/******************************************************************************/

static GByteArray *
gsm02_make_neighbours_list(GByteArray *message)
{
    g_assert(message);

    message = gsm02_packet_append_16(message, 0);
    message = gsm02_packet_append_16(message, GSM02_ORIG_ARFCN);

    return message;
}

/******************************************************************************/

static GByteArray *
gsm02_make_startup_channel(GByteArray *message)
{
    g_assert(message);

    message = gsm02_packet_append_16(message, GSM02_STARTUP_CHANNEL);

    return message;
}

/******************************************************************************/

static gsm02_t *
gsm02_new(GSocketConnection *connection)
{
    gsm02_t *gsm02 = NULL;

    g_assert(connection);

    gsm02 = g_new0(gsm02_t, 1);

    g_assert(gsm02);

    gsm02->buffer = gsm02_buffer_new();
    gsm02->stream = gsm02_stream_new(connection);
    gsm02->tags = g_new0(gsm02_tags_t, 1);

    g_assert(gsm02->buffer);
    g_assert(gsm02->stream);
    g_assert(gsm02->tags);

    return gsm02;
}

/******************************************************************************/

static void
gsm02_output_stream_write(gsm02_t *gsm02,
        GByteArray *message)
{
    GError *error = NULL;

    g_assert(gsm02);
    g_assert(gsm02->stream);
    g_assert(gsm02->stream->out);
    g_assert(message);

    if (message->len > 0) {
        g_output_stream_write(gsm02->stream->out,
                message->data,
                message->len,
                NULL,
                &error);
        g_assert_no_error(error);
    }
}

/******************************************************************************/

static GByteArray *
gsm02_packet_append_16(GByteArray *message,
        guint data)
{
    g_assert(message);

    data = g_htons(data);

    message = g_byte_array_append(message, (guint8 *) & data, sizeof (guint16));

    return message;
}

/******************************************************************************/

static GByteArray *
gsm02_packet_append_string_sized(GByteArray *message,
        gchar *data,
        gsize len)
{
    gchar aux[GSM02_BUFFER_LEN] = {};

    g_assert(data);
    g_assert(message);

    g_snprintf(aux, len, "%s", data);

    message = g_byte_array_append(message, (guint8 *) & aux, len);

    return message;
}

/******************************************************************************/

static void
gsm02_read_data(GObject *source,
        GAsyncResult *result,
        gpointer user_data)
{
    gsm02_t *gsm02 = user_data;
    GError *error = NULL;
    gsm02_packet_header_t *header = NULL;
    GInputStream *in = G_INPUT_STREAM(source);
    gssize nread;

    g_assert(gsm02);
    g_assert(gsm02->buffer);
    g_assert(gsm02->buffer->in);

    nread = g_input_stream_read_finish(in, result, &error);
    g_assert_no_error(error);

    if (nread > 0) {

        if (nread >= (gssize) GSM02_PACKET_HEADER_LEN) {
            gsm02_data_dump(gsm02->buffer->in, nread);
            header = gsm02_unpack_header(gsm02->buffer->in);
            gsm02_recv_pack_handler(gsm02, header->cmd);
        }

        gsm02_input_stream_read_async(gsm02);
    } else {
        gsm02_destroy(gsm02);
    }
}

/******************************************************************************/

static void
gsm02_recv_pack_handler(gsm02_t *gsm02,
        guchar command)
{
    g_assert(gsm02);

    switch (command) {
    case GSM02_CMD_RECV_CONF_REQUEST:
        gsm02_send_conf(gsm02);
        break;
    case GSM02_CMD_RECV_KEEP_ALIVE:
        message("GSM02", "Recv ping");
        break;
    case GSM02_CMD_RECV_SCANNED_NEIGH:
        gsm02_send_startup_channel(gsm02);
        gsm02_send_neighbours_list(gsm02);
        break;
    default:
        warning("GSM02", "Invalid command");
    }
}

/******************************************************************************/

static void
gsm02_send_conf(gsm02_t *gsm02)
{
    GByteArray *message = NULL;

    g_assert(gsm02);

    message = g_byte_array_new();

    message = gsm02_make_conf(message);
    message = gsm02_make_header(message, GSM02_CMD_SEND_CONF);

    gsm02_output_stream_write(gsm02, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static gboolean
gsm02_send_keep_alive(gpointer user_data)
{
    gsm02_t *gsm02 = user_data;
    GByteArray *message = NULL;

    g_assert(gsm02);

    message = g_byte_array_new();

    message = gsm02_make_header(message, GSM02_CMD_SEND_KEEP_ALIVE);

    gsm02_output_stream_write(gsm02, message);

    g_byte_array_free(message, TRUE);

    return TRUE;
}

/******************************************************************************/

static void
gsm02_send_neighbours_list(gsm02_t *gsm02)
{
    GByteArray *message = NULL;

    g_assert(gsm02);

    message = g_byte_array_new();

    message = gsm02_make_neighbours_list(message);
    message = gsm02_make_header(message, GSM02_CMD_SEND_NEIGHBOURS_LIST);

    gsm02_output_stream_write(gsm02, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static void
gsm02_send_startup_channel(gsm02_t *gsm02)
{
    GByteArray *message = NULL;

    g_assert(gsm02);

    message = g_byte_array_new();

    message = gsm02_make_startup_channel(message);
    message = gsm02_make_header(message, GSM02_CMD_SEND_STARTUP_CHANNEL);

    gsm02_output_stream_write(gsm02, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static void
gsm02_stream_free(gsm02_stream_t *stream)
{
    GError *error = NULL;

    g_assert(stream);

    if (stream->in) {
        g_input_stream_close(stream->in, NULL, &error);
        g_assert_no_error(error);
    }

    if (stream->in) {
        g_output_stream_close(stream->out, NULL, &error);
        g_assert_no_error(error);
    }
}

/******************************************************************************/

static gsm02_stream_t *
gsm02_stream_new(GSocketConnection *connection)
{
    gsm02_stream_t *stream = NULL;

    stream = g_new0(gsm02_stream_t, 1);

    g_assert(stream);

    stream->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    stream->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));

    return stream;
}

/******************************************************************************/

static gsm02_packet_header_t *
gsm02_unpack_header(gpointer buffer)
{
    gsm02_packet_header_t *header = buffer;

    g_assert(header);

    header->id = g_htons(header->id);
    header->n_bytes = g_htons(header->n_bytes);
    header->time_event = g_htonl(header->time_event);
    header->time_send = g_htonl(header->time_send);
    header->version = g_htons(header->version);

    return header;
}

/******************************************************************************/
