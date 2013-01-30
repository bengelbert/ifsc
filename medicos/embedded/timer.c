#include "wrapper.h"

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
typedef struct timer_s          timer_t;
typedef struct timer_funcs_s    timer_funcs_t;

struct timer_funcs_s {
    void *          data;
    timer_func_t    func;
    u08             active;
    u16             timeout;
};

/******************************************************************************/

struct timer_s {
    volatile u08    n_funcs;
    u08             tcnt0;
    volatile u32    ticks;
    timer_funcs_t   funcs[TIMER_N_FUNCS];
};

/******************************************************************************/
/*
 * Private function prototypes
 */
static u08  timer_poll_func(timer_funcs_t * funcs);

/*
 * Private variables
 */
static timer_t          timer;
static timer0_regs_t *  t0_regs = (timer0_regs_t *) TIMER0_BASE;

/******************************************************************************/

/*
 * Function definitions
 */
void
timer_init(void)
{
    t0_regs->tccrb  = TCCR0B_PRESCALER_256;
    t0_regs->tcnt   = 0;
    sbi(TIMSK0, TOIE0);

    DDRD = TCCR0A;

    timer.n_funcs  = 0;
    timer.tcnt0    = TCNT0_RATE_1MS;
    timer.ticks    = 0;

    return;
}

/******************************************************************************/

void
timer_poll_funcs(void)
{
    u08 i;
    
    for (i = 0; i < timer.n_funcs; i++) {
        timer_poll_func(&timer.funcs[i]);
    }
}

/******************************************************************************/

static u08
timer_poll_func(timer_funcs_t * funcs)
{
    if (funcs->active) {
        if (funcs->func(funcs->data)) {
            funcs->active = false;
        }
    }

    return 0;
}

/******************************************************************************/

void
timer_attach_func(u16 timeout, timer_func_t func, void * data)
{
    u08 index = 0;

    timer.n_funcs++;

    index = timer.n_funcs - 1;

    timer.funcs[index].data    = data;
    timer.funcs[index].func    = func;
    timer.funcs[index].active  = true;
    timer.funcs[index].timeout = timeout;
    
    return;
}

/******************************************************************************/

ISR(TIMER0_OVF_vect)
{
    register u08 i = 0;

    t0_regs->tcnt = timer.tcnt0;
    timer.ticks++;

    for (i = 0; i < timer.n_funcs; i++) {
        if (!(timer.ticks % timer.funcs[i].timeout))
            timer.funcs[i].active = true;
    }
}

/******************************************************************************/
