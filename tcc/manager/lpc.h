/* 
 * File:   conn.h
 * Author: bruno
 *
 * Created on 8 de Agosto de 2010, 21:34
 */

#ifndef CONN_H
#define	CONN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "wrapper.h"

void    conn_dbfc_server_cb(GServer *   __server,
                            GConn *     __conn,
                            gpointer    __user_data);

void    lpc_server_cb( GServer *   __server,
                            GConn *     __conn,
                            gpointer    __user_data);

#ifdef	__cplusplus
}
#endif

#endif	/* CONN_H */

