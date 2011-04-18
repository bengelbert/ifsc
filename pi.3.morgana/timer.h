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
typedef uint8_t (*timer_func_t)(void *);

typedef struct _timer0_regs timer0_regs_t;

/******************************************************************************/

struct _timer0_regs {
    volatile uint8_t tccra;
    volatile uint8_t tccrb;
    volatile uint8_t tcnt;
    volatile uint8_t ocra;
    volatile uint8_t ocrb;
};

/******************************************************************************/
/*
 * Public function prototypes
 */
void    timer_init      (void);
void    timer_loop      (void);
uint8_t timer_attach    (uint16_t timeout, timer_func_t func, void* data);
void    timer_detach    (uint8_t id);


#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */

