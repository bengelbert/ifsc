#include "wrapper.h"

/******************************************************************************/

struct service_s {
//    gsm02_buffer_t *buffer;
//    gsm02_stream_t *stream;
//    gsm02_tags_t *tags;
    GMainLoop *loop;
};

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
