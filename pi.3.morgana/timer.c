#include <avr/interrupt.h>
#include <avr/io.h>
#include <compat/deprecated.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "timer.h"

/******************************************************************************/
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

/* timer states */
#define TIMER_STATE_NULL       0
#define TIMER_STATE_CREATED    1
#define TIMER_STATE_WAITING      2
#define TIMER_STATE_RUNNING    3
#define TIMER_STATE_BLOCKED    4
#define TIMER_STATE_TERMINATED 5

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct _timer       timer_t;
typedef struct _timer_cb    timer_cb_t;
typedef struct _timer0_regs timer0_regs_t;

struct _timer_cb {
    uint8_t      id;
    uint8_t      state;
    void*        data;
    timer_handle_t handle;
    uint16_t     timeout;
};

/******************************************************************************/

struct _timer0_regs {
    volatile uint8_t tccra;
    volatile uint8_t tccrb;
    volatile uint8_t tcnt;
    volatile uint8_t ocra;
    volatile uint8_t ocrb;
};

/******************************************************************************/

struct _timer {
    volatile uint32_t ticks;
    
    volatile uint8_t n_funcs;
    timer_cb_t       cb[TIMER_N_FUNCS];
};

/******************************************************************************/
/*
 * Funtion Prototypes
 */
static void timer_low_level_init    (void);

/******************************************************************************/
/*
 * Macros
 */
#define TIMER0(obj)((timer0_regs_t*)(obj))

/******************************************************************************/
/*
 * Variables
 */
static timer_t timer = {
    0,
    0,
    {}
};

static timer0_regs_t* t0 = TIMER0(TIMER0_BASE);

/******************************************************************************/
/*
 * Function definitions
 */
static void
timer_low_level_init(void)
{
    t0->tccrb = TCCR0B_PRESCALER_256;
    t0->tcnt  = 0;
    
    sbi(TIMSK0, TOIE0);
}

/******************************************************************************/

uint8_t
timer_timeout_add(
    uint16_t       timeout, 
    timer_handle_t handle, 
    timer_data_t   data)
{
    uint8_t i = 0;

    if (handle == NULL) return -1;
    if (timeout == 0) return -1;
    if (timer.n_funcs == TIMER_N_FUNCS) return -1;
    
    for (i = 0; i < TIMER_N_FUNCS; i++) {
        if (timer.cb[i].state == TIMER_STATE_NULL || 
            timer.cb[i].state == TIMER_STATE_TERMINATED) {
            timer.cb[i].id      = i;
            timer.cb[i].data    = data;
            timer.cb[i].handle  = handle;
            timer.cb[i].state   = TIMER_STATE_WAITING;
            timer.cb[i].timeout = timeout;
            timer.n_funcs++;
            break;
        }
    }
    
    return i;
}

/******************************************************************************/

void
timer_source_remove(uint8_t id)
{
    uint8_t i = 0;
    
    for (i = 0; i < timer.n_funcs; i++) {
        if (timer.cb[i].id == id) {
            timer.cb[i].id      = 0;
            timer.cb[i].data    = 0;
            timer.cb[i].handle  = NULL;
            timer.cb[i].state   = TIMER_STATE_TERMINATED;
            timer.cb[i].timeout = 0;
            timer.n_funcs--;
        }
    }
}

/******************************************************************************/

void
timer_init(void)
{
    timer_low_level_init();

    timer.n_funcs = 0;
    timer.ticks   = 0;
}

/******************************************************************************/

void
timer_loop_run(void)
{
    uint8_t i;

    sei();
    
    for (;;) {
        for (i = 0; i < timer.n_funcs; i++) {
            
            if (timer.cb[i].state == TIMER_STATE_RUNNING) {
                
                /* execute callback function */
                if (timer.cb[i].handle(timer.cb[i].data)) {
                    /* if return is true, put on standby state */
                    timer.cb[i].state = TIMER_STATE_WAITING;
                } else {
                    /* else put on terminated state and free position */
                    timer.cb[i].state = TIMER_STATE_TERMINATED;
                }
            }
        }
    }
}

/******************************************************************************/

ISR(TIMER0_OVF_vect)
{
    register uint8_t i = 0;

    t0->tcnt = TCNT0_RATE_1MS;
    
    timer.ticks++;

    for (i = 0; i < timer.n_funcs; i++) {
        
        if (timer.cb[i].state == TIMER_STATE_WAITING) {
            
            if (!(timer.ticks % timer.cb[i].timeout)) {
                timer.cb[i].state = TIMER_STATE_RUNNING;
            }
        }
    }
}

/******************************************************************************/
