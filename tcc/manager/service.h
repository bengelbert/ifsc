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
 * @param self
 */
void
service_free(service_t *self);

/**
 * 
 * @param self
 * @return
 */
GInputStream *
service_get_input_stream(service_t *self);

/**
 * 
 * @param self
 * @param func
 * @param user_data
 */
void
service_input_stream_read_async(service_t *self,
    GAsyncReadyCallback func,
    gpointer user_data);

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
 * @param connection
 * @return
 */
service_t *
service_new(GSocketConnection *connection);

/**
 * 
 * @param self
 * @param message
 */
void
service_output_stream_write(service_t *self,
    GByteArray *message);

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

/**
 * 
 * @param self
 * @param len
 */
void
service_stream_buffer_dump(service_t *self,
    gssize len);

#ifdef	__cplusplus
}
#endif

#endif	/* SERVICE_H */

