/* 
 * File:   wrapper.h
 * Author: engelbert
 *
 * Created on 17 de Novembro de 2010, 20:55
 */

#ifndef WRAPPER_H
#define	WRAPPER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <FreeRTOS.h>
#include <queue.h>
#include <targets/LPC2368.h>
#include <task.h>

#include "client.h"
#include "commom.h"
#include "dgt.h"
#include "lcd.h"
#include "uip-conf.h"

#ifdef	__cplusplus
}
#endif

#endif	/* WRAPPER_H */

