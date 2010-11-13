/**
 * @file    gsm02.h
 */

#ifndef _GSM02_H
#define _GSM02_H
#ifdef	__cplusplus
extern "C" {
#endif

gboolean
gsm02_connect_handler(GThreadedSocketService * service,
        GSocketConnection * connection,
        GSocketListener * listener,
        gpointer user_data);


#ifdef	__cplusplus
}
#endif

#endif /* _GSM02_H */
