/**
 * @file    gsm02.h
 */

#ifndef _GSM02_H
#define _GSM02_H
#ifdef	__cplusplus
extern "C" {
#endif

/**
 *
 */
void
gsm02_async_queue_init(void);

/**
 * 
 * @param to
 * @param from
 * @param msg
 */
void
gsm02_async_queue_push(gchar *to,
        gchar *from,
        gchar *msg);

/**
 *
 * @param service
 * @param connection
 * @param listener
 * @param user_data
 * @return
 */
gboolean
gsm02_connect_handler(GThreadedSocketService * service,
        GSocketConnection * connection,
        GSocketListener * listener,
        gpointer user_data);


#ifdef	__cplusplus
}
#endif

#endif /* _GSM02_H */
