#include "wrapper.h"

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct service_stream_s service_stream_t;

/******************************************************************************/

struct service_s {
    service_stream_t *stream;
    GMainLoop *loop;
};

/******************************************************************************/

struct service_stream_s {
    guint8 *buffer;
    GInputStream *in;
    GOutputStream *out;
};

/******************************************************************************/

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
    GAsyncReadyCallback func,
    gpointer user_data)
{
    g_assert(func);
    g_assert(self);

    g_input_stream_read_async(self->stream->in,
        self->stream->buffer,
        SERVICE_BUFFER_LEN,
        G_PRIORITY_DEFAULT,
        NULL,
        func,
        user_data);
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

    service = g_threaded_socket_service_new(10);

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
    gchar dump[SERVICE_BUFFER_LEN*4];
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

