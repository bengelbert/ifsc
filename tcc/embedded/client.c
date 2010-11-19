#include <string.h>
#include <stdlib.h>

#include "client.h"
#include "dgt.h"
#include "FreeRTOS.h"
#include "lcd16x2.h"
#include "psock.h"
#include "pt.h"
#include "queue.h"
#include "uip.h"
#include "uip_arch.h"
#include "task.h"
#include "uip-conf.h"
#include "rtos/portable/GCC/CORTUS_APS3/portmacro.h"

/******************************************************************************/

/*
 * Type definitions
 */
struct client_s {
    lcd16x2_t *lcd;
    uip_ipaddr_t addr_local;
    uip_ipaddr_t addr_serv;
};

/******************************************************************************/

typedef struct {
    u8_t IPAddr[4];
    u8_t IPServAddr[4];
} PACK_STRUCT_END FileInitialization_t;

/******************************************************************************
 ** 2.3 Internal macros
 ******************************************************************************/

/******************************************************************************
 ** 2.4 Internal variables
 ******************************************************************************/
client_state_t s;
uip_ipaddr_t IPAddr;
uip_ipaddr_t IPServAddr;
static BYTE sLcdMessage[32];

static lcd16x2_t *lcd = NULL;

/******************************************************************************
 ** 2.5 Global variables (declared as 'extern' in some header files)
 ******************************************************************************/
/******************************************************************************
 ** 2.6 Private function prototypes (defined in Section 5)
 ******************************************************************************/

/******************************************************************************/
/*
 * Function prototypes
 */
/**
 * 
 * @param self
 */
static void
client_uip_closed(client_t *self);

/**
 *
 * @param self
 */
static void
client_uip_connected(client_t *self);

/**
 *
 * @param self
 */
static void
client_uip_newdata(client_t *self);

/******************************************************************************/

/*
 * Function definitions
 */
void
client_connect(client_t *self)
{
    lcd16x2_async_queue_push(self->lcd,
            "Try to connect..",
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);

    uip_connect(&(self->addr_serv), HTONS(50000));
}

/******************************************************************************/

void
client_appcall(void *data)
{
    char buffer[128] = {};
    client_t *obj = data;

    if (uip_connected()) {
        client_uip_connected(obj);
    } else if (uip_closed() || uip_aborted() || uip_timedout()) {
        client_uip_closed(obj);
    } else if (uip_newdata()) {
        client_uip_newdata(obj);
    } else if (uip_rexmit() || uip_acked() || uip_poll()) {
        // if (dgt_wGetInitConfigFlag() == 1) {
        //   vTaskDelay(dgt_wGetPollingTime() * 1000 / portTICK_RATE_MS);
        // dgt_dwSendPolling();
        // }
    }
}

/******************************************************************************/

client_t *
client_new(lcd16x2_t *lcd)
{
    client_t *obj = NULL;

    obj = calloc(1, sizeof (client_t));

    obj->lcd = lcd;

    uip_ipaddr(obj->addr_serv, 192, 168, 0, 1);
    uip_ipaddr(obj->addr_local, 192, 168, 0, 2);
    uip_sethostaddr(obj->addr_local);

    return obj;
}

/******************************************************************************/

static void
client_uip_closed(client_t *self)
{
    lcd16x2_async_queue_push(self->lcd,
            "Closed!",
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);

    s.connected = 0;

    vTaskDelay(5 * configTICK_RATE_HZ);

    client_connect(self);
}

/******************************************************************************/

static void
client_uip_connected(client_t *self)
{
    lcd16x2_async_queue_push(self->lcd,
            "Connected!",
            LCD_FIRST_COLUMN,
            LCD_FIRST_LINE);
}

/******************************************************************************/

static void
client_uip_newdata(client_t *self)
{
    if (uip_datalen() > 0) {
        lcd16x2_async_queue_push(self->lcd,
                "New data!",
                LCD_FIRST_COLUMN,
                LCD_FIRST_LINE);
    }
}

/******************************************************************************/
