/* 
 * File:   common.h
 * Author: bruno
 *
 * Created on 12 de Setembro de 2010, 12:12
 */

#ifndef COMMON_H
#define	COMMON_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "wrapper.h"

#define COMMON_BUFFER_LEN   128

void    common_print_dump   (gchar * __buffer, guint __len);
guint   common_timeout_add  (gchar *  __name, guint __interval, GSourceFunc __func, gpointer __data);

#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_H */

