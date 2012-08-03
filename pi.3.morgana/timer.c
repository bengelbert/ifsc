#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
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
#define TIMER_MS_RATE 1000
#define TIMER_US_RATE 1000000

#define TIMER_PRESCALER_MASK 0x07
#define TIMER_PRESCALER_8    8
#define TIMER_PRESCALER_256  256

#define TIMER_MAX_TASKS 10

#define TCCR0B_PRESCALER_1   (1 << CS00)
#define TCCR0B_PRESCALER_8   (1 << CS01)
#define TCCR0B_PRESCALER_256 (1 << CS02)

#define TCNT0_RATE_1MS   (255 - (1000 * (F_CPU/TIMER_US_RATE) / TIMER_PRESCALER_256))
#define TCNT0_RATE_100US (255 - (100 * (F_CPU/TIMER_US_RATE) / TIMER_PRESCALER_8))

/* timer states */
#define TIMER_STATE_NULL       0
#define TIMER_STATE_CREATED    1
#define TIMER_STATE_WAITING    2
#define TIMER_STATE_RUNNING    3
#define TIMER_STATE_BLOCKED    4
#define TIMER_STATE_TERMINATED 5

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct _timer       timer_t;
typedef struct _timer_task  timer_task_t;
typedef struct _timer0_regs timer0_regs_t;

struct _timer_task {
    uint8_t        id;
    uint8_t        state;
    void*          data;
    timer_handle_t handle;
    uint16_t       timeout;
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
    
    volatile uint8_t n_tasks;
    timer_task_t     task[TIMER_MAX_TASKS];
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
    uint8_t       i    = TIMER_MAX_TASKS;
    timer_task_t* task = NULL;

    if (timeout == 0) return -1;
    if (handle == NULL) return -1;
    if (timer.n_tasks >= TIMER_MAX_TASKS) return -1;
    
    task = &(timer.task[0]);
    
    do {
        if (task->state == TIMER_STATE_NULL || 
            task->state == TIMER_STATE_TERMINATED) {
            task->id      = i;
            task->data    = data;
            task->handle  = handle;
            task->state   = TIMER_STATE_WAITING;
            task->timeout = timeout;
            timer.n_tasks++;
            break;
        } else {
            task++;
        }
    } while (--i != 0);
    
    return i;
}

/******************************************************************************/

void
timer_source_remove(uint8_t id)
{
    uint8_t       i    = timer.n_tasks;
    timer_task_t* task = NULL;
    
    if (i == 0) return;
    
    task = &(timer.task[0]);
    
    do {
        if (task->id == id) {
            task->id      = 0;
            task->data    = 0;
            task->handle  = NULL;
            task->state   = TIMER_STATE_TERMINATED;
            task->timeout = 0;
            timer.n_tasks--;
            break;
        } else {
            task++;
        }
    } while (i != 0);
}

/******************************************************************************/

void
timer_init(void)
{
    timer_low_level_init();

    timer.n_tasks = 0;
    timer.ticks   = 0;
}

/******************************************************************************/

void
timer_loop_run(void)
{
    uint8_t       i    = 0;
    timer_task_t* task = NULL;

    sei();
    
    for (;;) {
        
        task = &(timer.task[0]);
        
        for (i = timer.n_tasks; i != 0; i--, task++) {
            
            if (task->state == TIMER_STATE_RUNNING) {
                /* execute callback function */
                if (task->handle(task->data)) {
                    /* if return is true, put on standby state */
                    task->state = TIMER_STATE_WAITING;
                } else {
                    /* else put on terminated state and free position */
                    task->state = TIMER_STATE_TERMINATED;
                }
            } 
        }
    }
}

/******************************************************************************/

ISR(TIMER0_OVF_vect)
{
    timer_task_t*    task = timer.task;
    register uint8_t i    = 0;

    t0->tcnt = TCNT0_RATE_1MS;
    
    timer.ticks++;

    for (i = timer.n_tasks; i != 0; i--) {
        if (task->state == TIMER_STATE_WAITING && !(timer.ticks % task->timeout)) {
            task->state = TIMER_STATE_RUNNING;
        } 
        
        task++;
    }
}

/******************************************************************************/
