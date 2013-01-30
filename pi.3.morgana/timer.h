/* 
 * File:   timer.h
 * Author: bruno
 *
 * Created on 28 de Agosto de 2010, 13:03
 */

#ifndef TIMER_H
#define	TIMER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * Public constants
 */
#define TIMER0_BASE 0x44

/******************************************************************************/
/*
 * Public type definition
 */
typedef uint8_t (*timer_handle_t)(void *);

typedef void* timer_data_t;

/******************************************************************************/
/*
 * Macros
 */
#define TIMER_HANDLE(func)   ((timer_handle_t) (func))
#define TIMER_FUNC_DATA(data)((timer_data_t)   (data))

/******************************************************************************/
/*
 * Public function prototypes
 */
void    timer_init          (void);
void    timer_loop_run      (void);
uint8_t timer_timeout_add   (uint16_t timeout, timer_handle_t handle, timer_data_t data);
void    timer_source_remove (uint8_t id);


#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */

