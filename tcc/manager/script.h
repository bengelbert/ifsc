/**
 * @file    script.h
 */

#ifndef _SCRIPT_H
#define _SCRIPT_H
#ifdef	__cplusplus
extern "C" {
#endif

gboolean
script_connect_handler(GThreadedSocketService * service,
        GSocketConnection * connection,
        GSocketListener * listener,
        gpointer user_data);


#ifdef	__cplusplus
}
#endif

#endif /* _SCRIPT_H */
