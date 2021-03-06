/**
 * @file    gsm02.h
 */

#ifndef _MCU_H
#define _MCU_H
#ifdef	__cplusplus
extern "C" {
#endif

/**
 *
 */
void
mcu_async_queue_init(void);

/**
 *
 * @param data
 */
void
mcu_async_queue_push(guint data);

/**
 *
 * @param service
 * @param connection
 * @param listener
 * @param user_data
 * @return
 */
gboolean
mcu_connect_handler(GThreadedSocketService * service,
        GSocketConnection * connection,
        GSocketListener * listener,
        gpointer user_data);


#ifdef	__cplusplus
}
#endif

#endif /* _MCU_H */
