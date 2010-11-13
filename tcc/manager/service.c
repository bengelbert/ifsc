#include "wrapper.h"

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
