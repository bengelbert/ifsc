/******************************************************************************
 *
 * @Filename    : dgt.c
 * @Abstract    :
 * @Settings    :
 * @Contents    :
 *     @Public:
 *          extern DWORD dgt_dwHandleMessage(BYTE * Message);
 *          extern DWORD dgt_dwSendRequestInitFile(void);
 *
 *     @Private:
 *          static WORD wMountHeader(BYTE byCmd, BYTE * Packet, WORD wPayloadLen);
 *          static DWORD dwLcdWrite(void);
 *
 * @Author      : bruno engelbert
 * @Http        :
 * @Mail        : bruno.engelbert@gmail.com
 * @Create on   : 15 de Novembro de 2009, 17:46
 *
 ******************************************************************************/

/******************************************************************************
 *
 * 1   INCLUDE FILES
 * 1.1 Standart include files
 *
 ******************************************************************************/
#include <string.h>

/******************************************************************************
 * 1.2 Application include files
 ******************************************************************************/
#include "commom.h"
#include "FreeRTOS.h"
#include "lcd.h"
#include "uip.h"
#include "task.h"

/******************************************************************************
 *
 * 2   DECLARATIONS
 * 2.1 Internal constants
 *
 ******************************************************************************/
/*
 * ARM to linux commands
 */
#define CMD_POLLING_ARM_TO_LINUX    (0x00)
#define CMD_REQUEST_INIT_FILE       (0x01)

/*
 * Linux to ARM commands
 */
#define CMD_POLLING_RESPONSE    (0x20)
#define CMD_INIT_FILE_RESPONSE  (0x21)

#define CMD_VERSION (0x01)
#define PACKET_LEN  (32)

/******************************************************************************
 * 2.2 Internal type definitions
 ******************************************************************************/

/******************************************************************************
 * @struct  Header_t
 *
 * @desc    Header used on dgt protocol.
 *
 * @format  +-------------+----------+-------+
 *          | wPayloadLen | wVersion | byCmd |
 *          +-------------+----------+-------+
 *
 * @field   wPayloadLen
 *              Size of message's payload.
 *
 * @field   wVersion
 *              Protocol version.
 *
 * @field   byCmd
 *              Command's index (ARM to Linux):
 *                  [0x00] Polling.
 *                  [0x01] Request init file.
 *              Command's index (Linux to ARM):
 *                  [0x20] Polling.
 *                  [0x21] Send init file.
 ******************************************************************************/
typedef struct {
    WORD wPayloadLen;
    WORD wVersion;
    BYTE byCmd;
} PACKED Header_t;

typedef struct {
    BYTE sLcdMessage[32];
    WORD wPollingTime;
} PACKED InitFile_t;

typedef struct {
    WORD wPollingIsSet;
    WORD wReceivedInitConfig;
} Flags_t;

/******************************************************************************
 * 2.3 Internal macros
 ******************************************************************************/
#define HEADER_LEN  sizeof(Header_t)

/******************************************************************************
 * 2.4 Internal variables
 ******************************************************************************/
static BYTE sLcdMessage[32];
static lcd_setup_t xMessage;

static Flags_t Flags = {
    .wPollingIsSet = 0,
    .wReceivedInitConfig = 0
};

static InitFile_t InitConfig;

/******************************************************************************
 * 2.5 Global variables (declared as 'extern' in some header files)
 ******************************************************************************/
extern xQueueHandle lcd_xQueue;

/******************************************************************************
 * 2.6 Private function prototypes (defined in Section 5)
 ******************************************************************************/
static WORD wMountHeader(BYTE byCmd, BYTE * Packet, WORD wPayloadLen);
static DWORD dwLcdWrite(void);
static DWORD dwReceivedInitFile(BYTE * Message);

/******************************************************************************
 *
 * 3 PUBLIC FUNCTIONS (declared in Section 2.5 on dgt.h)
 *
 ******************************************************************************/

/******************************************************************************
 * @func    dgt_wGetPollingFlag
 *
 * @desc
 *
 * @arg     Nothing
 *
 * @ret     wDataSize
 ******************************************************************************/
WORD dgt_wGetPollingFlag(void)
{
    WORD flag;

    flag = Flags.wPollingIsSet;

    return flag;
}

/******************************************************************************
 * @func    dgt_wGetPollingTime
 *
 * @desc
 *
 * @arg     Nothing
 *
 * @ret     wDataSize
 ******************************************************************************/
WORD dgt_wGetPollingTime(void)
{
    WORD wPollingTime;

    wPollingTime = InitConfig.wPollingTime;

    return wPollingTime;
}

/******************************************************************************
 * @func    dgt_wGetPollingFlag
 *
 * @desc
 *
 * @arg     Nothing
 *
 * @ret     wDataSize
 ******************************************************************************/
WORD dgt_wGetInitConfigFlag(void)
{
    WORD flag;

    flag = Flags.wReceivedInitConfig;

    return flag;
}

/******************************************************************************
 * @func    dgt_dwHandleMessage
 *
 * @desc
 *
 * @arg     Nothing
 *
 * @ret     wDataSize
 ******************************************************************************/
DWORD dgt_dwHandleMessage(BYTE * Message)
{
    BYTE byCmd;
    WORD wPayloadLen;
    WORD wVersion;
    CommomPointer_t msg;

    if (Message == NULL)
        return -1;

    msg.b = Message;
    wPayloadLen = HTONS(*msg.w++);
    wVersion = HTONS(*msg.w++);
    byCmd = *msg.b++;

    switch (byCmd) {

    case CMD_INIT_FILE_RESPONSE:
        dwReceivedInitFile(msg.b);
        break;
    }
    
    return 0;
}

/******************************************************************************
 * @func    dgt_dwSendPolling
 *
 * @desc
 *
 * @arg     None
 *
 * @ret     Zero
 ******************************************************************************/
DWORD dgt_dwSendPolling(void)
{
    BYTE Packet[PACKET_LEN];
    WORD wPayLoadLen;

    wPayLoadLen = 0;

    wMountHeader(CMD_POLLING_ARM_TO_LINUX, Packet, wPayLoadLen);

    uip_send(Packet, HEADER_LEN + wPayLoadLen);

    return 0;
}

/******************************************************************************
 * @func    dgt_dwSendRequestInitFile
 *
 * @desc
 *
 * @arg     None
 *
 * @ret     Zero
 ******************************************************************************/
DWORD dgt_dwSendRequestInitFile(void)
{
    BYTE Packet[PACKET_LEN];
    WORD wPayLoadLen;

    wPayLoadLen = 0;

    wMountHeader(CMD_REQUEST_INIT_FILE, Packet, wPayLoadLen);

    uip_send(Packet, HEADER_LEN + wPayLoadLen);

    return 0;
}

/******************************************************************************
 *
 * 4 PRIVATE FUNCTIONS (declared on Section 2.6)
 *
 ******************************************************************************/

/******************************************************************************
 * @func    wMountHeader
 *
 * @desc    Mount the packet's header.
 *
 * @arg     None
 *
 * @ret     Zero
 ******************************************************************************/
static WORD wMountHeader(BYTE byCmd, BYTE * Packet, WORD wPayloadLen)
{
    CommomPointer_t Message;

    Message.b = (BYTE *) Packet;

    *Message.w++ = HTONS(wPayloadLen);
    *Message.w++ = HTONS(CMD_VERSION);
    *Message.b++ = byCmd;

    return 0;
}

/******************************************************************************
 * @func    dwLcdWrite
 *
 * @desc    Send message to lcd's queue.
 *
 * @arg     None
 *
 * @ret     Zero
 ******************************************************************************/
static DWORD dwLcdWrite(void)
{
    xMessage.Message = sLcdMessage;
    xMessage.byColumn = LCD_FIRST_COLUMN;
    xMessage.byRow = LCD_FIRST_LINE;

    xQueueSend(lcd_xQueue, &xMessage, portMAX_DELAY);

    vTaskDelay(2000 / portTICK_RATE_MS);

    return 0;
}

/******************************************************************************
 * @func    dwReceivedInitFile
 *
 * @desc    Send message to lcd's queue.
 *
 * @arg     None
 *
 * @ret     Zero
 ******************************************************************************/
static DWORD dwReceivedInitFile(BYTE * PayLoad)
{
    if (PayLoad == NULL)
        return -1;

    memcpy((BYTE *) & InitConfig, PayLoad, sizeof (InitConfig));

    memcpy(sLcdMessage, InitConfig.sLcdMessage, sizeof (sLcdMessage));

    InitConfig.wPollingTime = HTONS(InitConfig.wPollingTime);

    Flags.wReceivedInitConfig = 1;

    //lcd_dwSendToQueue(InitConfig.sLcdMessage, LCD_FIRST_LINE, LCD_FIRST_COLUMN);

    return 0;
}

/******************************************************************************
 *
 * END OF FILE
 *
 ******************************************************************************/
