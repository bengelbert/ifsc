#include "wrapper.h"

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

const guint8 *
service_socket_address_to_bytes(GSocketAddress *address)
{
    GInetAddress *inet_address;

    inet_address = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(address));

    return g_inet_address_to_bytes(inet_address);
}

/******************************************************************************/

gchar *
service_socket_address_to_string(GSocketAddress *address)
{
    GInetAddress *inet_address;

    inet_address = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(address));

    return g_inet_address_to_string(inet_address);
}

/******************************************************************************/

