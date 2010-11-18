/******************************************************************************
 **
 ** Filename    : client.c
 ** Abstract    :
 ** Settings    :
 ** Contents    :
 **     Public:
 **     Private:
 ** Author      : bruno
 ** Http        :
 ** Mail        :
 ** Create on   : 11 de Outubro de 2009, 12:23
 **
 ******************************************************************************/
/******************************************************************************
 **
 ** 1   INCLUDE FILES
 ** 1.1 Standart include files
 **
 ******************************************************************************/
#include <string.h>
#include <stdlib.h>

/******************************************************************************
 ** 1.2 Application include files
 ******************************************************************************/
#include "client.h"
#include "dgt.h"
#include "FreeRTOS.h"
#include "lcd16x2.h"
#include "psock.h"
#include "pt.h"
#include "queue.h"
#include "uip.h"
#include "uip_arch.h"
#include "webserver/uip-conf.h"

/******************************************************************************
 **
 ** 2   DECLARATIONS
 ** 2.1 Internal constants
 **
 ******************************************************************************/
/******************************************************************************
 ** 2.2 Internal type definitions
 ******************************************************************************/

/**
 ** File Initialization type
 **/
typedef struct {
    u8_t IPAddr[4];
    u8_t IPServAddr[4];
} __attribute__((packed)) FileInitialization_t;

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
void newData(void);

/******************************************************************************
 **
 ** 3 PUBLIC FUNCTIONS (declared in Section 2.5 on client.h)
 **
 ******************************************************************************/

/******************************************************************************/

void
client_connect(void)
{
    uip_connect(&IPServAddr, HTONS(9099));
}

/******************************************************************************/

void
client_appcall(void)
{
    static BYTE sLcdMessage[] = "Opaaaa";

    if (uip_connected()) {
        //(sLcdMessage, LCD_FIRST_LINE, LCD_FIRST_COLUMN);
        //dgt_dwSendRequestInitFile();
    }
    if (uip_closed()) {
        s.connected = 0;
        client_connect();
        return;
    }
    if (uip_aborted() || uip_timedout()) {
        s.connected = 0;
        client_connect();
        return;
    }
    if (uip_newdata()) {
        //if (uip_datalen() > 0)
        //dgt_dwHandleMessage((BYTE *) uip_appdata);
    }

    if (uip_rexmit() || uip_acked() || uip_poll()) {
        // if (dgt_wGetInitConfigFlag() == 1) {
        //   vTaskDelay(dgt_wGetPollingTime() * 1000 / portTICK_RATE_MS);
        // dgt_dwSendPolling();
        // }
    }
}

/******************************************************************************
 ** Function    : client_init
 **
 ** Descriptions:
 **
 ** Parameters  :
 ** Return      :
 **
 ******************************************************************************/
void
client_init(void *user_data)
{
    lcd = user_data;

    uip_ipaddr(IPAddr, uipIP_ADDR0, uipIP_ADDR1, uipIP_ADDR2, uipIP_ADDR3);
    uip_sethostaddr(IPAddr);
    uip_ipaddr(IPServAddr, 192, 168, 0, 1);

    lcd16x2_async_queue_push(lcd, "Try to connect..IP: 192.168.0.1", 1, 1);

    client_connect();
}

/******************************************************************************
 **
 ** 4 PRIVATE FUNCTIONS (declared on Section 2.6)
 **
 ******************************************************************************/

/******************************************************************************
 ** Function    : newData
 **
 ** Descriptions:
 **
 ** Parameters  :
 ** Return      :
 **
 ******************************************************************************/
void newData(void)
{

}
