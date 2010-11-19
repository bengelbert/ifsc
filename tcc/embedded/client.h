/******************************************************************************
 **
 ** Filename    : client.h
 ** Abstract    :
 ** Settings    :
 ** Contents    :
 **     Public:
 **     Private:
 ** Author      : bruno
 ** Http        :
 ** Mail        :
 ** Create on   : 11 de Outubro de 2009, 12:27
 **
 ******************************************************************************/
#ifndef _CLIENT_H_
#define _CLIENT_H_

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
#include <stdint.h>

#include "commom.h"
#include "psock.h"
#include "uipopt.h"
#include "lcd16x2.h"

/******************************************************************************
 **
 ** 2   DECLARATIONS
 ** 2.1 Global constants
 **
 ******************************************************************************/
#define CLIENT_CMD_FILE_INITIALIZATION  (0x00)

/******************************************************************************/
/*
 * Type definitions
 */
typedef struct client_s client_t;
typedef struct client_state_s uip_tcp_appstate_t, client_state_t;

struct client_state_s {
    uint8_t connected;
    uint8_t state;
    struct psock psock;
};

/**
 * 
 * @param data
 */
void
client_appcall(void *data);

/**
 * 
 * @param self
 */
void
client_connect(client_t *self);

/**
 * 
 * @param lcd
 * @return
 */
client_t *
client_new(lcd16x2_t *lcd);


/******************************************************************************
 **
 ** END OF FILE
 **
 ******************************************************************************/
#ifdef	__cplusplus
}
#endif
#endif /* _CLIENT_H_ */
