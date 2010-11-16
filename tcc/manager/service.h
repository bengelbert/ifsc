/* 
 * File:   service.h
 * Author: engelbert
 *
 * Created on 12 de Novembro de 2010, 23:27
 */

#ifndef SERVICE_H
#define	SERVICE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "wrapper.h"

#define SERVICE_BUFFER_LEN    4096

typedef struct service_s service_t;

/**
 *
 * @param message
 * @param data
 * @return
 */
GByteArray *
service_message_append_16(GByteArray *message,
        guint data);

/**
 *
 * @param message
 * @param data
 * @param len
 * @return
 */
GByteArray *
service_message_append_string_sized(GByteArray *message,
        gchar *data,
        gsize len);

/**
 *
 * @param port
 * @param func
 * @param data
 */
void
service_socket_add(guint16   port,
                   GCallback func,
                   gpointer  data);


#ifdef	__cplusplus
}
#endif

#endif	/* SERVICE_H */

