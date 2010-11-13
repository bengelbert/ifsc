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

