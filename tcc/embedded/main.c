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

/* Environment includes. */
#include <targets/LPC2368.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "lcd16x2.h"

/* Demo application definitions. */
#define mainQUEUE_SIZE            (3)
#define mainCHECK_DELAY           ((portTickType) 5000 / portTICK_RATE_MS)
#define mainBASIC_WEB_STACK_SIZE  (configMINIMAL_STACK_SIZE * 2)

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY   (tskIDLE_PRIORITY + 2)
#define mainCHECK_TASK_PRIORITY   (tskIDLE_PRIORITY + 3)
#define mainSEM_TEST_PRIORITY     (tskIDLE_PRIORITY + 1)
#define mainBLOCK_Q_PRIORITY      (tskIDLE_PRIORITY + 2)
#define mainFLASH_PRIORITY        (tskIDLE_PRIORITY + 2)
#define mainCREATOR_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define mainINTEGER_TASK_PRIORITY (tskIDLE_PRIORITY)

/*
 * Checks the status of all the demo tasks then prints a message to the
 * CrossStudio terminal IO windows.  The message will be either PASS or FAIL
 * depending on the status of the demo applications tasks.  A FAIL status will
 * be latched.
 *
 * Messages are not written directly to the terminal, but passed to vPrintTask
 * via a queue.
 */
static void vCheckTask(void *pvParameters);

/* 
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task(void *pvParameters);

int main(void)
{
    lcd16x2_t *lcd = NULL;

    lcd = lcd16x2_new();

    /* Setup the led's on the MCB2300 board */
    vParTestInitialise();

    /* Create the lwIP task.  This uses the lwIP RTOS abstraction layer.*/
    xTaskCreate(vuIP_Task, (signed portCHAR *) "uIP", mainBASIC_WEB_STACK_SIZE, lcd, mainCHECK_TASK_PRIORITY - 1, NULL);
    //xTaskCreate(mmc_vTask, (signed portCHAR *) "MMC", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL);
    xTaskCreate(lcd_task, (signed portCHAR *) "LCD", configMINIMAL_STACK_SIZE, lcd, mainCHECK_TASK_PRIORITY - 1, NULL);

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
    return 0;
}


/*-----------------------------------------------------------*/

/* Keep the compiler quiet. */
#include <stdio.h>

int __putchar(int c)
{
    return EOF;
}





