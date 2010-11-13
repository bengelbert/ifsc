/**
 * @file    dbfc.h
 */

#ifndef _DBFC_H
#define _DBFC_H
#ifdef	__cplusplus
extern "C" {
#endif

gboolean
dbfc_connect_handler(GThreadedSocketService * service,
        GSocketConnection * connection,
        GSocketListener * listener,
        gpointer user_data);


#ifdef	__cplusplus
}
#endif

#endif /* _DBFC_H */
