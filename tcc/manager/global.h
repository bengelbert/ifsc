/* 
 * File:   global.h
 * Author: bruno
 *
 * Created on 9 de Agosto de 2010, 22:14
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PACKED  __attribute__((packed))

typedef struct _Global  Global;

/*
 * Function prototypes
 */
guint       global_get_client_ping_interval (Global * __global);
guint       global_get_client_port          (Global * __global);
Global *    global_new                      (void);

#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

