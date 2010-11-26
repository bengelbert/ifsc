#include <FreeRTOS.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <queue.h>

#include "commom.h"
#include "lcd16x2.h"

/*
 * Constants
 */
#define LCD16X2_ASYNC_QUEUE_LEN             3
#define LCD16X2_ASYNC_QUEUE_MESSAGE_LEN     32

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

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct lcd16x2_async_queue_data_s lcd16x2_async_queue_data_t;

struct lcd16x2_s {
    uint32_t n_write;
    xQueueHandle queue;
};

/******************************************************************************/

struct lcd16x2_async_queue_data_s {
    uint32_t x;
    uint32_t y;
    uint8_t message[LCD16X2_ASYNC_QUEUE_MESSAGE_LEN];
};

/******************************************************************************/
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

/******************************************************************************/
/*
 * Function prototypes
 */
/**
 * 
 * @return
 */
static xQueueHandle
lcd16x2_async_queue_new(void);

/**
 * 
 * @param self
 * @return
 */
static lcd16x2_async_queue_data_t *
lcd16x2_async_queue_pop(lcd16x2_t *self);

static DWORD dwReadStat(void);
static uint32_t lcd_wait_busy(void);

/**
 * 
 * @param data
 */
static void
lcd_write_byte(uint8_t data);

/**
 * 
 * @desc Write a 4-bit command to LCD controller.
 * @param nibble
 */
static void
lcd_write_nibble(uint8_t nibble);

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

/******************************************************************************/

static lcd16x2_async_queue_data_t *
lcd16x2_async_data_queue_new(uint8_t *message,
        uint32_t x,
        uint32_t y)
{
    lcd16x2_async_queue_data_t *data = NULL;

    data = calloc(1, sizeof (lcd16x2_async_queue_data_t));

    strncpy(data->message, message, LCD16X2_ASYNC_QUEUE_MESSAGE_LEN);

    data->x = x;
    data->y = y;

    return data;
}

/******************************************************************************/

static xQueueHandle
lcd16x2_async_queue_new(void)
{
    xQueueHandle queue;

    queue = xQueueCreate(LCD16X2_ASYNC_QUEUE_LEN,
            sizeof (lcd16x2_async_queue_data_t));

    return queue;
}

/******************************************************************************/

static lcd16x2_async_queue_data_t *
lcd16x2_async_queue_pop(lcd16x2_t *self)
{
    lcd16x2_async_queue_data_t *data = NULL;

    while (xQueueReceive(self->queue, &data, portMAX_DELAY) != pdPASS);

    return data;
}

/******************************************************************************/

void
lcd16x2_async_queue_push(lcd16x2_t *self,
        uint8_t *message,
        uint8_t x,
        uint8_t y)
{
    lcd16x2_async_queue_data_t *data = NULL;

    data = lcd16x2_async_data_queue_new(message, x, y);

    xQueueSend(self->queue, &data, portMAX_DELAY);
}

/******************************************************************************/

void
lcd16x2_bar_graph(lcd16x2_t *self,
        uint32_t value,
        size_t size)
{
    unsigned int i;

    value = value * size / 20; // Display matrix 5 x 8 pixels
    for (i = 0; i < size; i++) {
        if (value > 5) {
            lcd16x2_putc(self, 5);
            value -= 5;
        } else {
            lcd16x2_putc(self, value);
            break;
        }
    }
}

/******************************************************************************/

void
lcd16x2_clear(lcd16x2_t *self)
{
    lcd_write_command(CMD_CLEAR_DISPLAY);
    lcd16x2_goto_xy(self, 1, 1);
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

/******************************************************************************/

void
lcd16x2_goto_xy(lcd16x2_t *self,
        uint8_t x,
        uint8_t y)
{
    BYTE c;

    c = --x;
    if (--y) {
        lcd_write_command(CMD_DDRAM_ADDR_SET | DDRAM_LINE_2 | c);
    } else {
        lcd_write_command(CMD_DDRAM_ADDR_SET | DDRAM_LINE_1 | c);
    }

    self->n_write = y * 16 + x;
}

/******************************************************************************/

void
lcd16x2_init_4bit(lcd16x2_t *self)
{
    PINSEL3 = 0x00000000;
#if USE_FIO
    SCS |= 0x00000001; /* set GPIOx to use Fast I/O */
#endif
    FIO1DIR |= LCD_CTRL | LCD_DATA;
    FIO1CLR = LCD_RW | LCD_RS | LCD_DATA;

    vTaskDelay(COMMOM_DELAY_15MS);
    lcd_write_nibble(0x3); /* Select 4-bit interface            */
    lcd_write_nibble(0x3);
    lcd_write_nibble(0x3);
    lcd_write_nibble(0x2);

    lcd_write_command(CMD_FUNCTION_SET | FSET_4BITS | FSET_TWO_LINES | FSET_MATRIX_5_7);
    lcd_write_command(CMD_ENTRY_MODE_SET | EMODE_CURSOR | EMODE_SHIFT_RIGHT);
    lcd_write_command(CMD_DISPLAY_CONTROL | DCTRL_DISPLAY_ON | DCTRL_CURSOR_OFF | DCTRL_BLINK_ON);

    lcd16x2_clear(self);
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

/******************************************************************************/

lcd16x2_t *
lcd16x2_new(void)
{
    lcd16x2_t *obj = NULL;

    obj = calloc(1, sizeof (lcd16x2_t));

    obj->queue = xQueueCreate(LCD16X2_ASYNC_QUEUE_LEN,
            sizeof (lcd16x2_async_queue_data_t *));

    return obj;
}

/******************************************************************************/

void
lcd16x2_putc(lcd16x2_t *self,
        uint8_t data)
{
    if (self->n_write == 16) {

        lcd16x2_goto_xy(self, 1, 2);
    }

    lcd_write_data(data);
    self->n_write++;
}

/******************************************************************************/

void
lcd16x2_puts(lcd16x2_t *self,
        uint8_t *message)
{
    while (*message) {

        lcd16x2_putc(self, *message++);
    }
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
void
lcd_task(void *user_data)
{
    lcd16x2_t *obj = user_data;
    lcd16x2_async_queue_data_t *data = NULL;

    lcd16x2_init_4bit(obj);

    for (;;) {
        data = lcd16x2_async_queue_pop(obj);

        lcd16x2_clear(obj);
        lcd16x2_goto_xy(obj, data->x, data->y);
        lcd16x2_puts(obj, data->message);
        free(data);
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

    CommomNibbles_t * nibble;

    nibble = (CommomNibbles_t *) & data;
    lcd_write_nibble(nibble->byteH);
    lcd_write_nibble(nibble->byteL);
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

static void
lcd_write_nibble(uint8_t nibble)
{
    FIO1DIR |= LCD_DATA | LCD_CTRL;
    FIO1CLR = LCD_RW | LCD_DATA;
    FIO1SET = (nibble & 0xF) << 24;
    FIO1SET = LCD_E;
    vTaskDelay(COMMOM_DELAY_1MS);
    FIO1CLR = LCD_E;
    vTaskDelay(COMMOM_DELAY_1MS);
}

/******************************************************************************/

