#include <avr/interrupt.h>
#include <avr/io.h>
#include <compat/deprecated.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "timer.h"

/*
 * Private constants
 */
#define TIMER_0                 0
#define TIMER_TOTAL_REGISTERS   3

#define TIMER_MS_RATE           1000
#define TIMER_US_RATE           1000000

#define TIMER_PRESCALER_MASK    0x07
#define TIMER_PRESCALER_8       8
#define TIMER_PRESCALER_256     256

#define TIMER_N_FUNCS           10

#define TCCR0B_PRESCALER_1      (1 << CS00)
#define TCCR0B_PRESCALER_8      (1 << CS01)
#define TCCR0B_PRESCALER_256    (1 << CS02)

#define TCNT0_RATE_1MS      (255 - (1000 * (F_CPU/TIMER_US_RATE) / TIMER_PRESCALER_256))
#define TCNT0_RATE_100US    (255 - (100 * (F_CPU/TIMER_US_RATE) / TIMER_PRESCALER_8))
/******************************************************************************/

/*
 * Private type definitions
 */
typedef struct _timer    timer_t;
typedef struct _timer_cb timer_cb_t;

struct _timer_cb {
    uint8_t      id;
    uint8_t      active;
    void*        data;
    timer_func_t func;
    uint16_t     timeout;
};

/******************************************************************************/

struct _timer {
    uint8_t           tcnt0;
    timer_cb_t        cb[TIMER_N_FUNCS];
    volatile uint8_t  n_funcs;
    volatile uint32_t ticks;
};

/******************************************************************************/
/*
 * Private function prototypes
 */
static uint8_t timer_poll(timer_cb_t* funcs);

/*
 * Private variables
 */
static timer_t        timer   = {0, {}, 0, 0};
static timer0_regs_t* t0_regs = (timer0_regs_t *) TIMER0_BASE;

/******************************************************************************/

/*
 * Function definitions
 */
uint8_t
timer_attach(
    uint16_t     timeout, 
    timer_func_t func, 
    void*        data)
{
    uint8_t i = 0;

    if (func == NULL) return -1;
    if (timeout == 0) return -1;
    if (timer.n_funcs == TIMER_N_FUNCS) return -1;
    
    for (i = 0; i < TIMER_N_FUNCS; i++) {
        if (timer.cb[i].active == false) {
            timer.cb[i].id      = i;
            timer.cb[i].data    = data;
            timer.cb[i].func    = func;
            timer.cb[i].active  = true;
            timer.cb[i].timeout = timeout;
            timer.n_funcs++;
            break;
        }
    }
    
    return i;
}

/******************************************************************************/

void
timer_detach(uint8_t id)
{
    uint8_t i = 0;
    
    for (i = 0; i < timer.n_funcs; i++) {
        if (timer.cb[i].id == id) {
            timer.cb[i].id      = 0;
            timer.cb[i].data    = 0;
            timer.cb[i].func    = NULL;
            timer.cb[i].active  = false;
            timer.cb[i].timeout = 0;
            timer.n_funcs--;
        }
    }
}

/******************************************************************************/

void
timer_init(void)
{
    t0_regs->tccrb = TCCR0B_PRESCALER_256;
    t0_regs->tcnt  = 0;
    
    sbi(TIMSK0, TOIE0);

    DDRD = TCCR0A;

    timer.n_funcs = 0;
    timer.tcnt0   = TCNT0_RATE_1MS;
    timer.ticks   = 0;
}

/******************************************************************************/

void
timer_loop(void)
{
    uint8_t i;
    
    for (;;) {
        for (i = 0; i < timer.n_funcs; i++) {
            timer_poll(&(timer.cb[i]));
        }
    }
}

/******************************************************************************/

static uint8_t
timer_poll(timer_cb_t* cb)
{
    if (cb->active) {
        if (cb->func(cb->data)) {
            cb->active = false;
        }
    }

    return 0;
}

/******************************************************************************/

ISR(TIMER0_OVF_vect)
{
    register uint8_t i = 0;

    t0_regs->tcnt = timer.tcnt0;
    
    timer.ticks++;

    for (i = 0; i < timer.n_funcs; i++) {
        if (!(timer.ticks % timer.cb[i].timeout)) {
            timer.cb[i].active = true;
        }
    }
}

/******************************************************************************/
