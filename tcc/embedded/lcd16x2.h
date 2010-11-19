/******************************************************************************
 **
 ** Filename    : lcd16x2.h
 ** Abstract    :
 ** Settings    :
 ** Contents    :
 **     Public:
 **     Private:
 ** Author      : bruno
 ** Http        :
 ** Mail        :
 ** Create on   : 25 de Outubro de 2009, 13:02
 **
 ******************************************************************************/
#ifndef _LCD16X2_H_
#define _LCD16X2_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "commom.h"

#define LCD_FIRST_COLUMN  (1)
#define LCD_SECOND_COLUMN (2)

#define LCD_FIRST_LINE  (1)
#define LCD_SECOND_LINE (2)

/******************************************************************************
 ** 2.2 Global type definitions
 ******************************************************************************/
typedef struct lcd16x2_s lcd16x2_t;

/******************************************************************************/
/*
 * Function prototypes
 */

/**
 *
 * @param self
 * @param message
 * @param x
 * @param y
 */
void
lcd16x2_async_queue_push(lcd16x2_t *self,
        uint8_t *message,
        uint8_t x,
        uint8_t y);

/**
 * 
 * @param self
 * @param value
 * @param size
 */
void
lcd16x2_bar_graph(lcd16x2_t *self,
        uint32_t value,
        size_t size);

/**
 *
 * @desc Clear LCD display, move cursor to home position.
 * @param self
 */
void
lcd16x2_clear(lcd16x2_t *self);

extern DWORD lcd_cursorOff(void);

/**
 * 
 * @desc Set cursor position on LCD display. Left corner: 1,1, right: 16,2
 * @param self
 * @param x
 * @param y
 */
void
lcd16x2_goto_xy(lcd16x2_t *self,
        uint8_t x,
        uint8_t y);

/**
 * 
 * @param self
 */
void
lcd16x2_init_4bit(lcd16x2_t *self);

extern DWORD lcd_loadCGRAM(BYTE *fp, DWORD dwCnt);

/**
 * 
 * @return 
 */
lcd16x2_t *
lcd16x2_new(void);

/**
 *
 * @desc Print a character to LCD at current cursor position.
 * @param self
 * @param data
 */
void
lcd16x2_putc(lcd16x2_t *self,
        uint8_t data);

/**
 *
 * @desc Print a string to LCD display.
 * @param self
 * @param message
 */
void
lcd16x2_puts(lcd16x2_t *self,
        uint8_t *message);

extern void lcd_task (void *pvParameters);

/******************************************************************************
 **
 ** END OF FILE
 **
 ******************************************************************************/
#ifdef	__cplusplus
}
#endif
#endif /* _LCD16X2_H_ */
