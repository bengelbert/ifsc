#include "wrapper.h"

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct service_stream_s service_stream_t;
typedef struct service_message_header_s service_message_header_t;
typedef struct service_user_s service_user_t;

/******************************************************************************/

struct service_s {
    service_user_t *user;
    service_stream_t *stream;
    GMainLoop *loop;
};

/******************************************************************************/

struct service_message_header_s {
    guint16 n_bytes;
    guint16 version;
    guint8 cmd;
    guint32 time_event;
    guint32 time_send;
    guint16 id;
} PACKED;

/******************************************************************************/

struct service_stream_s {
    guint8 *buffer;
    GInputStream *in;
    GOutputStream *out;
};

/******************************************************************************/

struct service_user_s {
    gpointer data;
    service_func_t async_func;
    GDestroyNotify destroy_func;
};

/******************************************************************************/
/*
 * Function prototypes
 */
static void
service_message_read_data(GObject *source,
        GAsyncResult *result,
        gpointer user_data);


/******************************************************************************/
/*
 * Macros
 */
#define SERVICE_MESSAGE_HEADER_LEN (sizeof(service_message_header_t))

/******************************************************************************/
/*
 * Function definitions
 */
void
service_free(service_t *self)
{
    GError *error = NULL;

    g_assert(self);

    if (self->stream->in) {
        g_input_stream_close(self->stream->in, NULL, &error);
        g_assert_no_error(error);
    }

    if (self->stream->in) {
        g_output_stream_close(self->stream->out, NULL, &error);
        g_assert_no_error(error);
    }


    g_free(self->stream);
    g_free(self->user);
    g_free(self);
}

/******************************************************************************/

GInputStream *
service_get_input_stream(service_t *self)
{
    g_assert(self);
    g_assert(self->stream);
    g_assert(self->stream->in);

    return self->stream->in;
}

/******************************************************************************/

void
service_input_stream_read_async(service_t *self,
        service_func_t async_func,
        GDestroyNotify destroy_func,
        gpointer user_data)
{
    g_assert(async_func);
    g_assert(self);

    self->user->data = user_data;
    self->user->destroy_func = destroy_func;
    self->user->async_func = async_func;

    g_input_stream_read_async(self->stream->in,
            self->stream->buffer,
            SERVICE_BUFFER_LEN,
            G_PRIORITY_DEFAULT,
            NULL,
            service_message_read_data,
            self);
}

/******************************************************************************/

GByteArray *
service_message_append_16(GByteArray *message,
        guint data)
{
    g_assert(message);

    data = g_htons(data);

    message = g_byte_array_append(message, (guint8 *) & data, sizeof (guint16));

    return message;
}

/******************************************************************************/

GByteArray *
service_message_append_string_sized(GByteArray *message,
        gchar *data,
        gsize len)
{
    gchar aux[SERVICE_BUFFER_LEN] = {};

    g_assert(data);
    g_assert(message);

    g_snprintf(aux, len, "%s", data);

    message = g_byte_array_append(message, (guint8 *) & aux, len);

    return message;
}

/******************************************************************************/

guint8
service_message_header_get_command(service_t *self)
{
    service_message_header_t *header;

    g_assert(self);
    g_assert(self->stream);
    g_assert(self->stream->buffer);

    header = (service_message_header_t *) self->stream->buffer;

    return header->cmd;
}

/******************************************************************************/

GByteArray *
service_message_header_make(GByteArray *message,
        guchar cmd)
{
    service_message_header_t header = {0, 0, 0, 0, 0, 0};

    g_assert(message);

    header.cmd = cmd;
    header.id = 0;
    header.n_bytes = g_htons(SERVICE_MESSAGE_HEADER_LEN + message->len);
    header.time_event = g_htonl(time(NULL));
    header.time_send = g_htonl(time(NULL));
    header.version = g_htons(2);

    message = g_byte_array_prepend(message, (guint8 *) & header, SERVICE_MESSAGE_HEADER_LEN);

    return message;
}

/******************************************************************************/

gssize
service_message_header_size(void)
{
    return SERVICE_MESSAGE_HEADER_LEN;
}

/******************************************************************************/

void
service_message_header_unpack(service_t *self)
{
    service_message_header_t *header;

    g_assert(self);
    g_assert(self->stream);
    g_assert(self->stream->buffer);

    header = (service_message_header_t *) self->stream->buffer;

    header->id = g_htons(header->id);
    header->n_bytes = g_htons(header->n_bytes);
    header->time_event = g_htonl(header->time_event);
    header->time_send = g_htonl(header->time_send);
    header->version = g_htons(header->version);
}

/******************************************************************************/

static void
service_message_read_data(GObject *source,
        GAsyncResult *result,
        gpointer user_data)
{
    service_t *obj = user_data;
    GError *error = NULL;
    GInputStream *in = G_INPUT_STREAM(source);
    gssize nread;

    g_assert(obj);

    nread = g_input_stream_read_finish(in, result, &error);
    g_assert_no_error(error);

    if (nread > 0) {

        if (nread >= service_message_header_size()) {
            service_stream_buffer_dump(obj, nread);
            service_message_header_unpack(obj);

            if (obj->user->async_func) {
                obj->user->async_func(obj->user->data);
            }
        }

        service_input_stream_read_async(obj,
                obj->user->async_func,
                obj->user->destroy_func,
                obj->user->data);
    } else {
        if (obj->user->destroy_func) {
            obj->user->destroy_func(obj->user->data);
        }
    }
}

/******************************************************************************/

service_t *
service_new(GSocketConnection *connection)
{
    service_t *obj = NULL;

    g_assert(connection);

    obj = g_new0(service_t, 1);
    g_assert(obj);

    obj->stream = g_new0(service_stream_t, 1);
    g_assert(obj->stream);

    obj->stream->buffer = g_new0(guint8, SERVICE_BUFFER_LEN);
    obj->stream->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    obj->stream->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));

    obj->user = g_new0(service_user_t, 1);

    return obj;
}

/******************************************************************************/

void
service_output_stream_write(service_t *self,
        GByteArray *message)
{
    GError *error = NULL;

    g_assert(message);
    g_assert(self);
    g_assert(self->stream);
    g_assert(self->stream->out);

    if (message->len > 0) {
        g_output_stream_write(self->stream->out,
                message->data,
                message->len,
                NULL,
                &error);
        g_assert_no_error(error);
    }
}

/******************************************************************************/

void
service_socket_add(guint16 port,
        GCallback func,
        gpointer data)
{
    GError *error = NULL;
    GSocketService *service;

    service = g_threaded_socket_service_new(1);

    if (!g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service),
            port,
            NULL,
            &error)) {
        g_assert_no_error(error);
    } else {
        g_signal_connect(service, "run", func, data);
    }
}

/******************************************************************************/

void
service_stream_buffer_dump(service_t *self,
        gssize len)
{
    gchar dump[SERVICE_BUFFER_LEN * 4];
    gint i, j;
    gint dec = 0;

    g_assert(self);

    memset(dump, 0, sizeof (dump));
    for (i = 0, j = 0; i < len; i++, j += 3) {

        if (!(i % 10)) {
            g_snprintf(&dump[j + dec], 2, "\n");
            dec += 1;
        }

        g_snprintf(&dump[j + dec], 4, "%02X ", self->stream->buffer[i]);
    }

    debug("SERVICE", "\n%s\n", dump);
}

/******************************************************************************/

