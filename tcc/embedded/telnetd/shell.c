/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: shell.c,v 1.1 2006/06/07 09:43:54 adam Exp $
 *
 */

#include "shell.h"

#include <string.h>

#include "FreeRTOS.h"
#include "portlcd.h"
#include "queue.h"

struct ptentry {
  char *commandstr;
  void (* pfunc)(char *str);
};

#define SHELL_PROMPT "DGT20307> "

/*---------------------------------------------------------------------------*/
static void parse (register char *str, struct ptentry *t)
{
  struct ptentry *p;
  for (p = t; p->commandstr != NULL; ++p) {
    if (strncmp(p->commandstr, str, strlen(p->commandstr)) == 0) {
      break;
    }
  }
  p->pfunc(str);
}

/*---------------------------------------------------------------------------*/
static void inttostr (register char *str, unsigned int i)
{
  str[0] = '0' + i / 100;
  if (str[0] == '0') {
    str[0] = ' ';
  }
  str[1] = '0' + (i / 10) % 10;
  if (str[0] == ' ' && str[1] == '0') {
    str[1] = ' ';
  }
  str[2] = '0' + i % 10;
  str[3] = ' ';
  str[4] = 0;
}

/*---------------------------------------------------------------------------*/
static void help (char *str)
{
  shell_output("Available commands:", "");
  shell_output("stats      - show network statistics", "");
  shell_output("conn       - show TCP connections", "");
  shell_output("led_on x   - LED 'x' on (0 - 3)", "");
  shell_output("lcd 'text' - text to print on lcd", "");
  shell_output("help, ?    - show help", "");
  shell_output("exit       - exit shell", "");
}

/*---------------------------------------------------------------------------*/
static void unknown (char *str)
{
  if (strlen(str) > 0) {
    shell_output("Unknown command: ", str);
  }
}

/*---------------------------------------------------------------------------*/
static void lcd (char *str)
{
  short wStrLen;
  lcd_setup_t LCDMessage;
  extern xQueueHandle xLCDQueue;

  LCDMessage.byColumn = 0;
  wStrLen = strlen("lcd ");
  if (strlen(str) > wStrLen) {
    LCD_cls();
    LCD_gotoxy(1, 1);
    LCDMessage.Message = &str[wStrLen];
    xQueueSend(xLCDQueue, &LCDMessage, portMAX_DELAY);
    shell_output("LCD OK!", "");
    shell_output("Message: ", LCDMessage.Message);
  } else {
    shell_output("LCD ERROR: String null!", "");
  }
}

/*---------------------------------------------------------------------------*/
static struct ptentry parsetab[] = {
  {"stats", help},
  {"conn", help},
  {"help", help},
  {"lcd ", lcd},
  {"exit", shell_quit},
  {"?", help},
  {NULL, unknown} /* Default action */
};

/*---------------------------------------------------------------------------*/
void shell_init (void) { }

/*---------------------------------------------------------------------------*/
void shell_start (void)
{
  shell_output("uIP command shell", "");
  shell_output("Type '?' and return for help", "");
  shell_prompt(SHELL_PROMPT);
}

/*---------------------------------------------------------------------------*/
void shell_input (char *cmd)
{
  parse(cmd, parsetab);
  shell_prompt(SHELL_PROMPT);
}
/*---------------------------------------------------------------------------*/
