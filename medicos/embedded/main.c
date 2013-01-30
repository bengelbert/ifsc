/**
 * @file    main.c
 */
#include "teste01.h"
#include "wrapper.h"

typedef struct {
    uint8_t *name;
    uint8_t *description;
    CmdlineFuncPtrType func;
    void *data;
} main_cmdline;

/**
 * 
 * @param cmds
 */
static void
main_cmdline_help(void *arg);

/**
 * 
 * @param data
 */
static void
main_cmdline_histogram(void *data);

/**
 * 
 * @param data
 */
static void
main_cmdline_movavg(void *data);

/******************************************************************************/

static main_cmdline cmds[] = {
    {(uint8_t *) "help", (uint8_t *) "      - help!", main_cmdline_help, NULL},
    {(uint8_t *) "histogram", (uint8_t *) " - print the histogram.", main_cmdline_histogram, NULL},
    {(uint8_t *) "moving-avg", (uint8_t *) "- print the moving averaging.", main_cmdline_movavg, NULL},
    {NULL}
};

/******************************************************************************/

static void
main_cmdline_help(void *arg)
{
    uint32_t i;
    
    for (i = 0; cmds[i].name != NULL; i++) {
        rprintf("%s %s\n", cmds[i].name, cmds[i].description);
    }
}

/******************************************************************************/

static void
main_cmdline_histogram(void *data)
{
    histogram_t *t = NULL;
    
    t = histogram_get_type();

    histogram_set_n_baselines(t, 48);
    histogram_set_precision(t, 10000);
    histogram_do(t, teste01_samples, TESTE01_N_SAMPLES);
    histogram_print(t);
}

/******************************************************************************/

static void
main_cmdline_movavg(void *data)
{
    movavg_do(teste01_samples, TESTE01_N_SAMPLES);
}

/******************************************************************************/

int
main(void)
{
    uint8_t c;
    uint32_t i;

    outb(DDRD, 0xFF);
    outb(PORTD, 0xFE);

    uart_init();
    uart_set_baud_rate(57600);
    rprintfInit(uartSendByte);

    cmdlineInit();
    cmdlineSetOutputFunc(uartSendByte);
    for (i = 0; cmds[i].name != NULL; i++) {
        cmdlineAddCommand(cmds[i].name, cmds[i].func, cmds[i].data);
    }
    cmdlineInputFunc('\r');

    for (;;) {
        if (uartReceiveByte(&c)) {
            cmdlineInputFunc(c);
        }
        
        cmdlineMainLoop();
    }
    
    return 0;
}
