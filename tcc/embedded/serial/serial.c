/*
  FreeRTOS V5.4.0 - Copyright (C) 2003-2009 Richard Barry.

  This file is part of the FreeRTOS distribution.

  FreeRTOS is free software; you can redistribute it and/or modify it	under
  the terms of the GNU General Public License (version 2) as published by the
  Free Software Foundation and modified by the FreeRTOS exception.
 **NOTE** The exception to the GPL is included to allow you to distribute a
  combined work that includes FreeRTOS without being obliged to provide the
  source code for proprietary components outside of the FreeRTOS kernel.
  Alternative commercial license and support terms are also available upon
  request.  See the licensing section of http://www.FreeRTOS.org for full
  license details.

  FreeRTOS is distributed in the hope that it will be useful,	but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with FreeRTOS; if not, write to the Free Software Foundation, Inc., 59
  Temple Place, Suite 330, Boston, MA  02111-1307  USA.


 ***************************************************************************
 *                                                                         *
 * Looking for a quick start?  Then check out the FreeRTOS eBook!          *
 * See http://www.FreeRTOS.org/Documentation for details                   *
 *                                                                         *
 ***************************************************************************

  1 tab == 4 spaces!

  Please ensure to read the configuration and relevant port sections of the
  online documentation.

  http://www.FreeRTOS.org - Documentation, latest information, license and
  contact details.

  http://www.SafeRTOS.com - A version that is certified for use in safety
  critical systems.

  http://www.OpenRTOS.com - Commercial support, development, porting,
  licensing and training services.
 */

/*
  Changes from V2.4.0

    + Made serial ISR handling more complete and robust.

  Changes from V2.4.1

    + Split serial.c into serial.c and serialISR.c.  serial.c can be
      compiled using ARM or THUMB modes.  serialISR.c must always be
      compiled in ARM mode.
    + Another small change to cSerialPutChar().

  Changed from V2.5.1

    + In cSerialPutChar() an extra check is made to ensure the post to
      the queue was successful if then attempting to retrieve the posted
      character.

 */

/* 
  BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER FOR UART0.

  This file contains all the serial port components that can be compiled to
  either ARM or THUMB mode.  Components that must be compiled to ARM mode are
  contained in serialISR.c.
 */

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Demo application includes. */
#include "serial.h"

/*-----------------------------------------------------------*/

/* Constants to setup and access the UART. */
#define serDLAB                   ((unsigned portCHAR) 0x80)
#define serENABLE_INTERRUPTS      ((unsigned portCHAR) 0x03)
#define serNO_PARITY              ((unsigned portCHAR) 0x00)
#define ser1_STOP_BIT             ((unsigned portCHAR) 0x00)
#define ser8_BIT_CHARS            ((unsigned portCHAR) 0x03)
#define serFIFO_ON                ((unsigned portCHAR) 0x01)
#define serCLEAR_FIFO             ((unsigned portCHAR) 0x06)
#define serWANTED_CLOCK_SCALING   ((unsigned portLONG) 16)

/* Constants to setup and access the VIC. */
#define serUART0_VIC_CHANNEL      ((unsigned portLONG) 0x0006)
#define serUART0_VIC_CHANNEL_BIT  ((unsigned portLONG) 0x0040)
#define serUART0_VIC_ENABLE       ((unsigned portLONG) 0x0020)
#define serCLEAR_VIC_INTERRUPT    ((unsigned portLONG) 0)

#define serINVALID_QUEUE  ((xQueueHandle) 0)
#define serHANDLE         ((xComPortHandle) 1)
#define serNO_BLOCK       ((portTickType) 0)

/*-----------------------------------------------------------*/

/* Queues used to hold received characters, and characters waiting to be
transmitted. */
static xQueueHandle xRxedChars;
static xQueueHandle xCharsForTx;

/*-----------------------------------------------------------*/

/* Communication flag between the interrupt service routine and serial API. */
static volatile portLONG *plTHREEmpty;

/* 
 * The queues are created in serialISR.c as they are used from the ISR.
 * Obtain references to the queues and THRE Empty flag. 
 */
extern void vSerialISRCreateQueues (unsigned portBASE_TYPE uxQueueLength,
                                    xQueueHandle *pxRxedChars,
                                    xQueueHandle *pxCharsForTx,
                                    portLONG volatile **pplTHREEmptyFlag);

/*-----------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal (unsigned portLONG ulWantedBaud,
                                       unsigned portBASE_TYPE uxQueueLength)
{
  unsigned portLONG ulDivisor, ulWantedClock;
  xComPortHandle xReturn = serHANDLE;
  extern void (vUART_ISR_Wrapper) (void);

  /* The queues are used in the serial ISR routine, so are created from
  serialISR.c (which is always compiled to ARM mode. */
  vSerialISRCreateQueues(uxQueueLength, &xRxedChars, &xCharsForTx, &plTHREEmpty);

  if ((xRxedChars != serINVALID_QUEUE) &&
      (xCharsForTx != serINVALID_QUEUE) &&
      (ulWantedBaud != (unsigned portLONG) 0)) {
    portENTER_CRITICAL();
    {
      /* Setup the baud rate:  Calculate the divisor value. */
      ulWantedClock = ulWantedBaud * serWANTED_CLOCK_SCALING;
      ulDivisor = (configCPU_CLOCK_HZ / 4) / ulWantedClock;

      /* Set the DLAB bit so we can access the divisor. */
      U0LCR |= serDLAB;

      /* Setup the divisor. */
      U0DLL = (unsigned portCHAR) (ulDivisor & (unsigned portLONG) 0xff);
      ulDivisor >>= 8;
      U0DLM = (unsigned portCHAR) (ulDivisor & (unsigned portLONG) 0xff);

      /* Turn on the FIFO's and clear the buffers. */
      U0FCR = (serFIFO_ON | serCLEAR_FIFO);

      /* Setup transmission format. */
      U0LCR = serNO_PARITY | ser1_STOP_BIT | ser8_BIT_CHARS;

      /* Setup the VIC for the UART. */
      VICIntSelect &= ~(serUART0_VIC_CHANNEL_BIT);
      VICIntEnable |= serUART0_VIC_CHANNEL_BIT;
      VICVectAddr1 = (portLONG) vUART_ISR_Wrapper;
      VICVectCntl1 = serUART0_VIC_CHANNEL | serUART0_VIC_ENABLE;

      /* Enable UART0 interrupts. */
      U0IER |= serENABLE_INTERRUPTS;
    }
    portEXIT_CRITICAL();
  } else {
    xReturn = (xComPortHandle) 0;
  }

  return xReturn;
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar (xComPortHandle pxPort,
                                     signed portCHAR *pcRxedChar,
                                     portTickType xBlockTime)
{
  /* The port handle is not required as this driver only supports UART0. */
  (void) pxPort;

  /* Get the next character from the buffer.  Return false if no characters
  are available, or arrive before xBlockTime expires. */
  if (xQueueReceive(xRxedChars, pcRxedChar, xBlockTime)) {
    return pdTRUE;
  } else {
    return pdFALSE;
  }
}

/*-----------------------------------------------------------*/

void vSerialPutString (xComPortHandle pxPort,
                       const signed portCHAR * const pcString,
                       unsigned portSHORT usStringLength)
{
  signed portCHAR *pxNext;

  /* NOTE: This implementation does not handle the queue being full as no
  block time is used! */

  /* The port handle is not required as this driver only supports UART0. */
  (void) pxPort;
  (void) usStringLength;

  /* Send each character in the string, one at a time. */
  pxNext = (signed portCHAR *) pcString;
  while (*pxNext) {
    xSerialPutChar(pxPort, *pxNext, serNO_BLOCK);
    pxNext++;
  }
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar (xComPortHandle pxPort,
                                     signed portCHAR cOutChar,
                                     portTickType xBlockTime)
{
  signed portBASE_TYPE xReturn;

  /* This demo driver only supports one port so the parameter is not used. */
  (void) pxPort;

  portENTER_CRITICAL();
  {
    /* Is there space to write directly to the UART? */
    if (*plTHREEmpty == (portLONG) pdTRUE) {
      /* We wrote the character directly to the UART, so was
      successful. */
      *plTHREEmpty = pdFALSE;
      U0THR = cOutChar;
      xReturn = pdPASS;
    } else {
      /* We cannot write directly to the UART, so queue the character.
      Block for a maximum of xBlockTime if there is no space in the
      queue. */
      xReturn = xQueueSend(xCharsForTx, &cOutChar, xBlockTime);

      /* Depending on queue sizing and task prioritisation:  While we
      were blocked waiting to post interrupts were not disabled.  It is
      possible that the serial ISR has emptied the Tx queue, in which
      case we need to start the Tx off again. */
      if ((*plTHREEmpty == (portLONG) pdTRUE) && (xReturn == pdPASS)) {
        xQueueReceive(xCharsForTx, &cOutChar, serNO_BLOCK);
        *plTHREEmpty = pdFALSE;
        U0THR = cOutChar;
      }
    }
  }
  portEXIT_CRITICAL();

  return xReturn;
}

/*-----------------------------------------------------------*/

void vSerialClose (xComPortHandle xPort)
{
  /* Not supported as not required by the demo application. */
  (void) xPort;
}

/*-----------------------------------------------------------*/

void vSerialTask (void *pvParameters)
{
  signed portCHAR xSerialMessage[] = "Teste da Serial\n";
  signed portCHAR xSerialReceived[10];
  lcd_setup_t LCDMessage;
  extern xQueueHandle xLCDQueue;

  LCDMessage.Message = xSerialReceived;
  LCDMessage.byColumn = 0;

  /*
   * Initalize the serial COM port 0;
   */
  xSerialPortInitMinimal(ser115200, 10);

  vSerialPutString(serCOM1, xSerialMessage, sizeof (xSerialMessage));

  for (;;) {

  }
}






