#include "wrapper.h"

/******************************************************************************/
/*
 * Constants
 */
#define DBFC_BUFFER_LEN     4096
#define DBFC_CBCH_LEN       10
#define DBFC_IP_LEN         16

#define DBFC_CMD_RECV_KEEP_ALIVE    0x00
#define DBFC_CMD_RECV_SCANNED_NEIGH 0x08
#define DBFC_CMD_RECV_CONF_REQUEST  0x09

#define DBFC_CMD_SEND_KEEP_ALIVE        0x20
#define DBFC_CMD_SEND_CONF              0x24
#define DBFC_CMD_SEND_STARTUP_CHANNEL   0x23
#define DBFC_CMD_SEND_NEIGHBOURS_LIST   0x26

#define DBFC_CONF_APPLICATION_NGCELL    4
#define DBFC_CONF_CHANNEL_INIT          588
#define DBFC_CONF_CHANNEL_RANGE         5
#define DBFC_CONF_MODE_RASTER           0

#define DBFC_ORIG_ARFCN         DBFC_CONF_CHANNEL_INIT
#define DBFC_STARTUP_CHANNEL    DBFC_CONF_CHANNEL_INIT

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct dbfc_s dbfc_t;
typedef struct dbfc_buffer_s dbfc_buffer_t;
typedef struct dbfc_packet_conf_s dbfc_packet_conf_t;
typedef struct dbfc_packet_header_s dbfc_packet_header_t;
typedef struct dbfc_stream_s dbfc_stream_t;
typedef struct dbfc_tags_s dbfc_tags_t;

struct dbfc_s {
    dbfc_buffer_t *buffer;
    dbfc_stream_t *stream;
    dbfc_tags_t *tags;
    GMainLoop *loop;
};

/******************************************************************************/

struct dbfc_buffer_s {
    guchar *in;
    gssize in_offset;
};

/******************************************************************************/

struct dbfc_packet_conf_s {
    guint16 band;
    guint16 channel_init;
    guint16 channel_range;
    guint8 sip_realm[DBFC_IP_LEN];
    guint8 sip_proxy[DBFC_IP_LEN];
    guint8 sip_redirect[DBFC_IP_LEN];
    guint16 mcc;
    guint16 mnc;
    guint16 lac;
    guint16 reject_cause;
    guint8 cbch[DBFC_CBCH_LEN];
    guint16 application;
    guint16 power;
    guint16 afc;
    guint16 mode;
} PACKED;

/******************************************************************************/

struct dbfc_packet_header_s {
    guint16 n_bytes;
    guint16 version;
    guint8 cmd;
    guint32 time_event;
    guint32 time_send;
    guint16 id;
} PACKED;

/******************************************************************************/

struct dbfc_stream_s {
    GInputStream *in;
    GOutputStream *out;
};

/******************************************************************************/

struct dbfc_tags_s {
    guint send_ping;
};

/******************************************************************************/
/*
 * Macros
 */
#define DBFC_PACKET_HEADER_LEN (sizeof(dbfc_packet_header_t))
#define DBFC_PACKET_CONF_LEN (sizeof (dbfc_packet_conf_t))

/******************************************************************************/
/*
 * Function prototypes
 */
/**
 *
 * @param dbfc
 */
static void
dbfc_assert(dbfc_t *dbfc);

/**
 * 
 * @return 
 */
static dbfc_buffer_t *
dbfc_buffer_new(void);

/**
 * 
 * @param buffer
 * @param len
 */
static void
dbfc_data_dump(guchar *buffer,
        gssize len);

/**
 * 
 * @param dbfc
 * @param offset
 */
static void
dbfc_input_stream_read_async(dbfc_t *dbfc);

/**
 * 
 * @param message
 * @param cmd
 * @param payload_len
 * @return
 */
static GByteArray *
dbfc_make_header(GByteArray * message,
        guchar cmd);

/**
 * 
 * @param message
 * @return 
 */
static GByteArray *
dbfc_make_startup_channel(GByteArray *message);

/**
 *
 * @param connection
 * @return
 */
static dbfc_t *
dbfc_new(GSocketConnection *connection);

/**
 *
 * @param message
 * @param data
 * @return
 */
static GByteArray *
dbfc_packet_append_16(GByteArray *message,
        guint data);

/**
 * 
 * @param message
 * @param data
 * @param len
 * @return
 */
static GByteArray *
dbfc_packet_append_string_sized(GByteArray *message,
        gchar *data,
        gsize len);

/**
 * 
 * @param source
 * @param result
 * @param user_data
 */
static void
dbfc_read_data(GObject *source,
        GAsyncResult *result,
        gpointer user_data);

/**
 * 
 * @param dbfc
 * @param command
 */
static void
dbfc_recv_pack_handler(dbfc_t *dbfc,
        guchar command);

/**
 *
 * @param user_data
 * @return
 */
static gboolean
dbfc_send_keep_alive(gpointer user_data);

/**
 * 
 * @param dbfc
 */
static void
dbfc_send_conf(dbfc_t *dbfc);

/**
 * 
 * @param dbfc
 */
static void
dbfc_send_neighbours_list(dbfc_t *dbfc);

/**
 * 
 * @param dbfc
 */
static void
dbfc_send_startup_channel(dbfc_t *dbfc);

/**
 * 
 * @param stream
 */
static void
dbfc_stream_free(dbfc_stream_t *stream);

/**
 * 
 * @param connection
 * @return 
 */
static dbfc_stream_t *
service_stream_new(GSocketConnection *connection);

/**
 * 
 * @param buffer
 * @return
 */
static dbfc_packet_header_t *
dbfc_unpack_header(gpointer buffer);

/******************************************************************************/

static void
dbfc_buffer_free(dbfc_buffer_t *buffer)
{
    g_assert(buffer);

    if (buffer->in) {
        g_free(buffer->in);
    }
}

/******************************************************************************/

static dbfc_buffer_t *
dbfc_buffer_new(void)
{
    dbfc_buffer_t *buffer = NULL;

    buffer = g_new0(dbfc_buffer_t, 1);
    g_assert(buffer);

    buffer->in = g_new0(guchar, DBFC_BUFFER_LEN);
    g_assert(buffer->in);

    return buffer;
}

/******************************************************************************/

gboolean
dbfc_connect_handler(GThreadedSocketService *service,
        GSocketConnection *connection,
        GSocketListener *listener,
        gpointer user_data)
{
    dbfc_t *dbfc = NULL;

    g_assert(service);
    g_assert(connection);

    debug("DBFC", "listener(%p) data(%p)", listener, user_data);

    dbfc = dbfc_new(connection);
    dbfc->loop = g_main_loop_new(NULL, FALSE);

    dbfc_input_stream_read_async(dbfc);

    dbfc->tags->send_ping = g_timeout_add(10000, dbfc_send_keep_alive, dbfc);

    g_main_loop_run(dbfc->loop);

    return TRUE;
}

/******************************************************************************/

static void
dbfc_data_dump(guchar *buffer,
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

    debug("DBFC", "\n%s\n", dump);
}

/******************************************************************************/

static void
dbfc_destroy(dbfc_t *dbfc)
{
    g_assert(dbfc);
    g_assert(dbfc->loop);

    dbfc_buffer_free(dbfc->buffer);
    dbfc_stream_free(dbfc->stream);

    g_main_loop_quit(dbfc->loop);
    g_main_loop_unref(dbfc->loop);

    g_source_remove(dbfc->tags->send_ping);

    g_free(dbfc);
}

/******************************************************************************/

static void
dbfc_input_stream_read_async(dbfc_t *dbfc)
{
    dbfc_assert(dbfc);

    g_input_stream_read_async(dbfc->stream->in,
            dbfc->buffer->in,
            DBFC_BUFFER_LEN,
            G_PRIORITY_DEFAULT,
            NULL,
            dbfc_read_data,
            dbfc);
}

/******************************************************************************/

static GByteArray *
dbfc_make_conf(GByteArray *message)
{
    dbfc_packet_conf_t conf;

    g_assert(message);

    memset((guint8 *) & conf, 0, DBFC_PACKET_CONF_LEN);

    message = dbfc_packet_append_16(message, 1800);
    message = dbfc_packet_append_16(message, DBFC_CONF_CHANNEL_INIT);
    message = dbfc_packet_append_16(message, DBFC_CONF_CHANNEL_RANGE);
    message = dbfc_packet_append_string_sized(message, "172.23.255.254", DBFC_IP_LEN);
    message = dbfc_packet_append_string_sized(message, "172.23.255.254", DBFC_IP_LEN);
    message = dbfc_packet_append_string_sized(message, "172.23.255.254", DBFC_IP_LEN);
    message = dbfc_packet_append_16(message, 0x0724);
    message = dbfc_packet_append_16(message, 0x0019);
    message = dbfc_packet_append_16(message, 0x00AA);
    message = dbfc_packet_append_16(message, 13);
    message = dbfc_packet_append_string_sized(message, "", DBFC_CBCH_LEN);
    message = dbfc_packet_append_16(message, DBFC_CONF_APPLICATION_NGCELL);
    message = dbfc_packet_append_16(message, 15);
    message = dbfc_packet_append_16(message, 1024);
    message = dbfc_packet_append_16(message, DBFC_CONF_MODE_RASTER);

    return message;
}

/******************************************************************************/

static GByteArray *
dbfc_make_header(GByteArray * message,
        guchar cmd)
{
    dbfc_packet_header_t header = {0, 0, 0, 0, 0, 0};

    g_assert(message);

    header.cmd = cmd;
    header.id = 0;
    header.n_bytes = g_htons(DBFC_PACKET_HEADER_LEN + message->len);
    header.time_event = g_htonl(time(NULL));
    header.time_send = g_htonl(time(NULL));
    header.version = g_htons(2);

    message = g_byte_array_prepend(message, (guint8 *) & header, DBFC_PACKET_HEADER_LEN);

    return message;
}

/******************************************************************************/

static GByteArray *
dbfc_make_neighbours_list(GByteArray *message)
{
    g_assert(message);

    message = dbfc_packet_append_16(message, 0);
    message = dbfc_packet_append_16(message, DBFC_ORIG_ARFCN);

    return message;
}

/******************************************************************************/

static GByteArray *
dbfc_make_startup_channel(GByteArray *message)
{
    g_assert(message);

    message = dbfc_packet_append_16(message, DBFC_STARTUP_CHANNEL);

    return message;
}

/******************************************************************************/

static dbfc_t *
dbfc_new(GSocketConnection *connection)
{
    dbfc_t *dbfc = NULL;

    g_assert(connection);

    dbfc = g_new0(dbfc_t, 1);

    g_assert(dbfc);

    dbfc->buffer = dbfc_buffer_new();
    dbfc->stream = service_stream_new(connection);
    dbfc->tags = g_new0(dbfc_tags_t, 1);

    g_assert(dbfc->buffer);
    g_assert(dbfc->stream);
    g_assert(dbfc->tags);

    return dbfc;
}

/******************************************************************************/

static void
dbfc_output_stream_write(dbfc_t *dbfc,
        GByteArray *message)
{
    GError *error = NULL;

    g_assert(dbfc);
    g_assert(dbfc->stream);
    g_assert(dbfc->stream->out);
    g_assert(message);

    if (message->len > 0) {
        g_output_stream_write(dbfc->stream->out,
                message->data,
                message->len,
                NULL,
                &error);
        g_assert_no_error(error);
    }
}

/******************************************************************************/

static GByteArray *
dbfc_packet_append_16(GByteArray *message,
        guint data)
{
    g_assert(message);

    data = g_htons(data);

    message = g_byte_array_append(message, (guint8 *) & data, sizeof (guint16));

    return message;
}

/******************************************************************************/

static GByteArray *
dbfc_packet_append_string_sized(GByteArray *message,
        gchar *data,
        gsize len)
{
    gchar aux[DBFC_BUFFER_LEN] = {};

    g_assert(data);
    g_assert(message);

    g_snprintf(aux, len, "%s", data);

    message = g_byte_array_append(message, (guint8 *) & aux, len);

    return message;
}

/******************************************************************************/

static void
dbfc_read_data(GObject *source,
        GAsyncResult *result,
        gpointer user_data)
{
    dbfc_t *dbfc = user_data;
    GError *error = NULL;
    dbfc_packet_header_t *header = NULL;
    GInputStream *in = G_INPUT_STREAM(source);
    gssize nread;

    dbfc_assert(dbfc);

    nread = g_input_stream_read_finish(in, result, &error);
    g_assert_no_error(error);

    if (nread > 0) {

        if (nread >= (gssize) DBFC_PACKET_HEADER_LEN) {
            dbfc_data_dump(dbfc->buffer->in, nread);
            header = dbfc_unpack_header(dbfc->buffer->in);
            dbfc_recv_pack_handler(dbfc, header->cmd);
        }

        dbfc_input_stream_read_async(dbfc);
    } else {
        dbfc_destroy(dbfc);
    }
}

/******************************************************************************/

static void
dbfc_recv_pack_handler(dbfc_t *dbfc,
        guchar command)
{
    g_assert(dbfc);

    switch (command) {
    case DBFC_CMD_RECV_CONF_REQUEST:
        dbfc_send_conf(dbfc);
        break;
    case DBFC_CMD_RECV_KEEP_ALIVE:
        message("DBFC", "Recv ping");
        break;
    case DBFC_CMD_RECV_SCANNED_NEIGH:
        dbfc_send_startup_channel(dbfc);
        dbfc_send_neighbours_list(dbfc);
        break;
    default:
        warning("DBFC", "Invalid command");
    }
}

/******************************************************************************/

static void
dbfc_send_conf(dbfc_t *dbfc)
{
    GByteArray *message = NULL;

    g_assert(dbfc);

    message = g_byte_array_new();

    message = dbfc_make_conf(message);
    message = dbfc_make_header(message, DBFC_CMD_SEND_CONF);

    dbfc_output_stream_write(dbfc, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static gboolean
dbfc_send_keep_alive(gpointer user_data)
{
    dbfc_t *dbfc = user_data;
    GByteArray *message = NULL;

    g_assert(dbfc);

    message = g_byte_array_new();

    message = dbfc_make_header(message, DBFC_CMD_SEND_KEEP_ALIVE);

    dbfc_output_stream_write(dbfc, message);

    g_byte_array_free(message, TRUE);

    return TRUE;
}

/******************************************************************************/

static void
dbfc_send_neighbours_list(dbfc_t *dbfc)
{
    GByteArray *message = NULL;

    g_assert(dbfc);

    message = g_byte_array_new();

    message = dbfc_make_neighbours_list(message);
    message = dbfc_make_header(message, DBFC_CMD_SEND_NEIGHBOURS_LIST);

    dbfc_output_stream_write(dbfc, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static void
dbfc_send_startup_channel(dbfc_t *dbfc)
{
    GByteArray *message = NULL;

    g_assert(dbfc);

    message = g_byte_array_new();

    message = dbfc_make_startup_channel(message);
    message = dbfc_make_header(message, DBFC_CMD_SEND_STARTUP_CHANNEL);

    dbfc_output_stream_write(dbfc, message);

    g_byte_array_free(message, TRUE);
}

/******************************************************************************/

static void
dbfc_stream_free(dbfc_stream_t *stream)
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

static dbfc_stream_t *
service_stream_new(GSocketConnection *connection)
{
    dbfc_stream_t *stream = NULL;

    stream = g_new0(dbfc_stream_t, 1);

    g_assert(stream);

    stream->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    stream->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));

    return stream;
}

/******************************************************************************/

static dbfc_packet_header_t *
dbfc_unpack_header(gpointer buffer)
{
    dbfc_packet_header_t *header = buffer;

    g_assert(header);

    header->id = g_htons(header->id);
    header->n_bytes = g_htons(header->n_bytes);
    header->time_event = g_htonl(header->time_event);
    header->time_send = g_htonl(header->time_send);
    header->version = g_htons(header->version);

    return header;
}

/******************************************************************************/
