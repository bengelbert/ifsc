/******************************************************************************
 **
 ** Filename    : lcd.h
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
#ifndef _LCD_H_
#define _LCD_H_

#ifdef	__cplusplus
extern "C" {
#endif
/******************************************************************************
 **
 ** 1   INCLUDE FILES
 ** 1.1 Standart include files
 **
 ******************************************************************************/
/******************************************************************************
 ** 1.2 Application include files
 ******************************************************************************/
#include "commom.h"
#include "queue.h"

/******************************************************************************
 **
 ** 2   DECLARATIONS
 ** 2.1 Global constants
 **
 ******************************************************************************/
#define LCD_FIRST_COLUMN  (1)
#define LCD_SECOND_COLUMN (2)

#define LCD_FIRST_LINE  (1)
#define LCD_SECOND_LINE (2)

#define LCD_QUEUE_SIZE  (3)

/******************************************************************************
 ** 2.2 Global type definitions
 ******************************************************************************/
typedef struct {
  BYTE byColumn;
  BYTE byRow;
  BYTE * Message;
} lcd_setup_t;

/******************************************************************************
 ** 2.3 Global macros
 ******************************************************************************/
/******************************************************************************
 ** 2.4 Global variables
 ******************************************************************************/
/**
 ** The queue used to send messages to the LCD task.
 **/
extern xQueueHandle lcd_xQueue;

/******************************************************************************
 ** 2.5 Public function prototypes
 ******************************************************************************/
extern DWORD lcd_barGraph(DWORD dwValue, DWORD dwSize);
extern DWORD lcd_clear(void);
extern DWORD lcd_cursorOff(void);
extern DWORD lcd_goToXY(BYTE byColumn, BYTE byRow);
extern DWORD lcd_init(void);
extern DWORD lcd_loadCGRAM(BYTE *fp, DWORD dwCnt);
extern DWORD lcd_putc(BYTE byChar);
extern DWORD lcd_puts(BYTE * Message);
extern DWORD lcd_dwSendToQueue(BYTE * sLcdMessage, WORD wLine, WORD wColumn);
extern void lcd_task (void *pvParameters);

/******************************************************************************
 **
 ** END OF FILE
 **
 ******************************************************************************/
#ifdef	__cplusplus
}
#endif
#endif /* _LCD_H_ */
