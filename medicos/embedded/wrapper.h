/* 
 * File:   wrapper.h
 * Author: bruno
 *
 * Created on 28 de Agosto de 2010, 12:51
 */

#ifndef WRAPPER_H
#define	WRAPPER_H

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Standart include files
 */
#include <avr/common.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <compat/deprecated.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/atomic.h>
#include <util/delay.h>

/*
 * Application include files
 */
#include "avrlibdefs.h"
#include "avrlibtypes.h"
#include "buffer.h"
#include "cmdline.h"
#include "rprintf.h"
#include "uart.h"

#include "cmdlineconf.h"
#include "global.h"
#include "histogram.h"
#include "timer.h"

#ifdef	__cplusplus
}
#endif

#endif	/* WRAPPER_H */

