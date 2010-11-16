/******************************************************************************
 **
 ** Filename    : lcd.c
 ** Abstract    :
 ** Settings    :
 ** Contents    :
 **     Public:
 **     Private:
 ** Author      : bruno
 ** Http        :
 ** Mail        :
 ** Create on   : 25 de Outubro de 2009, 11:42
 **
 ******************************************************************************/
/******************************************************************************
 **
 ** 1   INCLUDE FILES
 ** 1.1 Standart include files
 **
 ******************************************************************************/
#include <targets/LPC2368.h>

/******************************************************************************
 ** 1.2 Application include files
 ******************************************************************************/
#include "commom.h"
#include "FreeRTOS.h"
#include "lcd.h"
#include "queue.h"
#include "task.h"
#include "webserver/uip-conf.h"

/******************************************************************************
 **
 ** 2   DECLARATIONS
 ** 2.1 Internal constants
 **
 ******************************************************************************/
/**
 ** LCD IO definitions
 **/
#define LCD_E     (0x80000000) // Enable control pin
#define LCD_RW    (0x20000000) // Read/Write control pin
#define LCD_RS    (0x10000000) // Data/Instruction control
#define LCD_CTRL  (0xB0000000) // Control lines mask
#define LCD_DATA  (0x0F000000) // Data lines mask

#define BUSY_FLAG (0x80)

/**
 **  LCD Commands
 **/
#define CMD_CLEAR_DISPLAY       (0x01)
#define CMD_CURSOR_AT_HOME      (0x02)
#define CMD_ENTRY_MODE_SET      (0x04)
#define CMD_DISPLAY_CONTROL     (0x08)
#define CMD_SHIFT_CONTROL       (0x10)
#define CMD_FUNCTION_SET        (0x20)
#define CMD_CGRAM_ADDR_SET      (0x40)
#define CMD_DDRAM_ADDR_SET      (0x80)

/**
 **  CMD_ENTRY_MODE_SET
 **/
#define EMODE_CURSOR        (0)
#define EMODE_MESSAGE       (1)
#define EMODE_SHIFT_LEFT    (0 << 1)
#define EMODE_SHIFT_RIGHT   (1 << 1)

/**
 **  CMD_DISPLAY_CONTROL
 **/
#define DCTRL_BLINK_OFF     (0 << 0)
#define DCTRL_BLINK_ON      (1 << 0)
#define DCTRL_CURSOR_OFF    (0 << 1)
#define DCTRL_CURSOR_ON     (1 << 1)
#define DCTRL_DISPLAY_OFF   (0 << 2)
#define DCTRL_DISPLAY_ON    (1 << 2)

/**
 **  CMD_SHIFT_CONTROL
 **/
#define SCTRL_SHIFT_LEFT    (0 << 2)
#define SCTRL_SHIFT_RIGHT   (1 << 2)
#define SCTRL_CURSOR        (0 << 3)
#define SCTRL_MESSAGE       (1 << 3)

/**
 **  CMD_FUNCTION_SET
 **/
#define FSET_MATRIX_5_7    (0 << 2)
#define FSET_MATRIX_5_10   (1 << 2)
#define FSET_ONE_LINES     (0 << 3)
#define FSET_TWO_LINES     (1 << 3)
#define FSET_4BITS         (0 << 4)
#define FSET_8BITS         (1 << 4)

/**
 **  CMD_DDRAM_ADDR_SET
 **/
#define DDRAM_LINE_1    (0 << 6)
#define DDRAM_LINE_2    (1 << 6)

/**
 ** 8 user defined characters to be loaded into CGRAM (used for bargraph)
 **/
static const BYTE UserFont[8][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
    { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C},
    { 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E},
    { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

/******************************************************************************
 ** 2.2 Internal type definitions
 ******************************************************************************/
/******************************************************************************
 ** 2.3 Internal macros
 ******************************************************************************/
/******************************************************************************
 ** 2.4 Internal variables
 ******************************************************************************/
static DWORD lcdPtr;

/******************************************************************************
 ** 2.5 Global variables (declared as 'extern' in some header files)
 ******************************************************************************/
xQueueHandle lcd_xQueue;
static lcd_setup_t xMessage;

/******************************************************************************
 ** 2.6 Private function prototypes (defined in Section 5)
 ******************************************************************************/
static DWORD dwReadStat(void);
static uint32_t lcd_wait_busy(void);
static void lcd_write_byte(uint8_t data);
static DWORD dwWriteByteNibble(BYTE byNibble);

/**
 * 
 * @desc Write command to LCD controller.
 * @param command byte
 */
static void
lcd_write_command(uint8_t command);

/**
 * 
 * @desc  Write data to LCD controller
 * @param data byte
 */
static void
lcd_write_data(uint8_t data);

/******************************************************************************
 ** Function    : lcd_barGraph
 **
 ** Descriptions: Print a bargraph to LCD display.
 **
 ** Parameters  : value and size
 **     - dwValue   : value 0..100 %
 **     - dwSize    : size of bargraph 1..16
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_barGraph(DWORD dwValue, DWORD dwSize)
{
    unsigned int i;

    dwValue = dwValue * dwSize / 20; // Display matrix 5 x 8 pixels
    for (i = 0; i < dwSize; i++) {
        if (dwValue > 5) {
            lcd_putc(5);
            dwValue -= 5;
        } else {
            lcd_putc(dwValue);
            break;
        }
    }
    return 0;
}

/******************************************************************************
 ** Function    : lcd_clear
 **
 ** Descriptions: Clear LCD display, move cursor to home position.
 **
 ** Parameters  : None
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_clear(void)
{
    lcd_write_command(CMD_CLEAR_DISPLAY);
    lcd_goToXY(1, 1);
    return 0;
}

/******************************************************************************
 ** Function    : lcd_cursorOff
 **
 ** Descriptions: Switch off LCD cursor.
 **
 ** Parameters  : None
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_cursorOff(void)
{
    lcd_write_command(CMD_DISPLAY_CONTROL | DCTRL_DISPLAY_ON | DCTRL_CURSOR_OFF);
    return 0;
}

/******************************************************************************
 ** Function    : lcd_goToXY
 **
 ** Descriptions: Set cursor position on LCD display. Left corner: 1,1,
 **     right: 16,2
 **
 ** Parameters  : Pixel X and Y
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_goToXY(BYTE byColumn, BYTE byRow)
{
    BYTE x;

    x = --byColumn;
    if (--byRow) {
        lcd_write_command(CMD_DDRAM_ADDR_SET | DDRAM_LINE_2 | x);
    } else {
        lcd_write_command(CMD_DDRAM_ADDR_SET | DDRAM_LINE_1 | x);
    }
    lcdPtr = byRow * 16 + byColumn;
    return 0;
}

/******************************************************************************
 ** Function    : lcd_init
 **
 ** Descriptions: Initialize the ST7066 LCD controller to 4-bit mode.
 **
 ** Parameters  : None
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_init(void)
{
    PINSEL3 = 0x00000000;
#if USE_FIO
    SCS |= 0x00000001; /* set GPIOx to use Fast I/O */
#endif
    FIO1DIR |= LCD_CTRL | LCD_DATA;
    FIO1CLR = LCD_RW | LCD_RS | LCD_DATA;

    vTaskDelay(COMMOM_DELAY_15MS);
    dwWriteByteNibble(0x3); /* Select 4-bit interface            */
    dwWriteByteNibble(0x3);
    dwWriteByteNibble(0x3);
    dwWriteByteNibble(0x2);

    lcd_write_command(CMD_FUNCTION_SET | FSET_4BITS | FSET_TWO_LINES | FSET_MATRIX_5_7);
    lcd_write_command(CMD_ENTRY_MODE_SET | EMODE_CURSOR | EMODE_SHIFT_RIGHT);
    lcd_write_command(CMD_DISPLAY_CONTROL | DCTRL_DISPLAY_ON | DCTRL_CURSOR_OFF | DCTRL_BLINK_ON);

    lcd_clear();
    return 0;
}

/******************************************************************************
 ** Function    : lcd_loadCGRAM
 **
 ** Descriptions: Load user-specific characters into CGRAM
 **
 ** Parameters  : Pointer to the buffer and counter
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_loadCGRAM(BYTE *fp, DWORD dwCnt)
{
    DWORD i;

    lcd_write_command(CMD_CGRAM_ADDR_SET); /* Set CGRAM address counter to 0    */
    for (i = 0; i < dwCnt; i++, fp++) {
        lcd_write_data(*fp);
    }
    return 0;
}

/******************************************************************************
 ** Function    : lcd_putc
 **
 ** Descriptions: Print a character to LCD at current cursor position.
 **
 ** Parameters  : Byte character
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_putc(BYTE byChar)
{
    if (lcdPtr == 16) {
        lcd_goToXY(1, 2);
    }

    lcd_write_data(byChar);
    lcdPtr++;

    return 0;
}

/******************************************************************************
 ** Function    : LCD_puts
 **
 ** Descriptions: Print a string to LCD display.
 **
 ** Parameters  : Pointer to the buffer
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_puts(BYTE * Message)
{
    while (*Message)
        lcd_putc(*Message++);

    return 0;
}

/******************************************************************************
 ** Function    : vLCDTask
 **
 ** Descriptions: Print a string to LCD display.
 **
 ** Parameters  : Pointer to the buffer
 ** Return      : None
 **
 ******************************************************************************/
DWORD lcd_dwSendToQueue(BYTE * sLcdMessage, WORD wLine, WORD wColumn)
{
    xMessage.Message = sLcdMessage;
    xMessage.byColumn = wColumn;
    xMessage.byRow = wLine;
    xQueueSend(lcd_xQueue, &xMessage, portMAX_DELAY);

    return 0;
}

/******************************************************************************
 ** Function    : vLCDTask
 **
 ** Descriptions: Print a string to LCD display.
 **
 ** Parameters  : Pointer to the buffer
 ** Return      : None
 **
 ******************************************************************************/
void lcd_vTask(void *pvParameters)
{
    lcd_setup_t LCDSetup;

    lcd_xQueue = xQueueCreate(LCD_QUEUE_SIZE, sizeof (lcd_setup_t));
    lcd_init();
    for (;;) {
        /* Wait for a message to arrive that requires displaying. */
        while (xQueueReceive(lcd_xQueue, &LCDSetup, portMAX_DELAY) != pdPASS);

        /* Display the message.  Print each message to a different position. */
        lcd_clear();
        lcd_goToXY((LCDSetup.byColumn & 0x07), (LCDSetup.byRow & 0x01));
        lcd_puts(LCDSetup.Message);
    }
}


/******************************************************************************
 **
 ** 4 PRIVATE FUNCTIONS (declared on Section 2.6)
 **
 ******************************************************************************/

/******************************************************************************
 ** Function    : dwReadStat
 **
 ** Descriptions:
 **
 ** Parameters  : None
 ** Return      : status
 **
 ******************************************************************************/
static DWORD dwReadStat(void)
{
    /* Read status of LCD controller (ST7066) */
    DWORD dwStat;

    FIO1DIR &= ~LCD_DATA;
    FIO1CLR = LCD_RS;
    FIO1SET = LCD_RW;
    vTaskDelay(0);
    FIO1SET = LCD_E;
    vTaskDelay(0);
    dwStat = (FIO1PIN >> 20) & 0xF0;
    FIO1CLR = LCD_E;
    vTaskDelay(0);
    FIO1SET = LCD_E;
    vTaskDelay(0);
    dwStat |= (FIO1PIN >> 24) & 0xF;
    FIO1CLR = LCD_E;
    return (dwStat);
}

/******************************************************************************
 ** Function    : dwWaitBusy
 **
 ** Descriptions: Wait until LCD controller (ST7066) is busy.
 **
 ** Parameters  : None
 ** Return      : None
 **
 ******************************************************************************/
static u32
lcd_wait_busy(void)
{
    u32 status;

    do {
        status = dwReadStat();
    } while (status & BUSY_FLAG); /* Wait for busy flag */
    
    return 0;
}

/******************************************************************************
 ** Function    : dwWriteByte
 **
 ** Descriptions: Write data/command to LCD controller.
 **
 ** Parameters  : Byte to write
 ** Return      : None
 **
 ******************************************************************************/
static void
lcd_write_byte(uint8_t data)
{
    CommomNibbles_t * Nibble;

    Nibble = (CommomNibbles_t *) & data;
    dwWriteByteNibble(Nibble->byteH);
    dwWriteByteNibble(Nibble->byteL);
}

/******************************************************************************
 ** Function    :  dwWriteByteNibble
 **
 ** Descriptions: Write a 4-bit command to LCD controller.
 **
 ** Parameters  : Four bits to write
 ** Return      : None
 **
 ******************************************************************************/
static DWORD dwWriteByteNibble(BYTE byNibble)
{
    FIO1DIR |= LCD_DATA | LCD_CTRL;
    FIO1CLR = LCD_RW | LCD_DATA;
    FIO1SET = (byNibble & 0xF) << 24;
    FIO1SET = LCD_E;
    vTaskDelay(COMMOM_DELAY_1MS);
    FIO1CLR = LCD_E;
    vTaskDelay(COMMOM_DELAY_1MS);
    return;
}

/******************************************************************************/

static void
lcd_write_command(uint8_t command)
{
    lcd_wait_busy();

    FIO1CLR = LCD_RS;
    
    lcd_write_byte(command);
}

/******************************************************************************/

static void
lcd_write_data(uint8_t data)
{
    lcd_wait_busy();

    FIO1SET = LCD_RS;
    
    lcd_write_byte(data);
}

/******************************************************************************/
