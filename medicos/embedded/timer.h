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

#include "wrapper.h"

/*
 * Public constants
 */
#define TIMER0_BASE 0x44

/******************************************************************************/
/*
 * Public type definition
 */
typedef bool (*timer_func_t)(void *);

typedef struct timer0_regs_s timer0_regs_t;

/******************************************************************************/

struct timer0_regs_s {
    volatile u08 tccra;
    volatile u08 tccrb;
    volatile u08 tcnt;
    volatile u08 ocra;
    volatile u08 ocrb;
};

/******************************************************************************/
/*
 * Public function prototypes
 */
void    timer_init          (void);
void    timer_poll_funcs    (void);
void    timer_attach_func   (u16 __timeout, timer_func_t __func, void * __data);


#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */

