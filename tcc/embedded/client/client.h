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

/******************************************************************************
 **
 ** 2   DECLARATIONS
 ** 2.1 Global constants
 **
 ******************************************************************************/
#define CLIENT_CMD_FILE_INITIALIZATION  (0x00)

/******************************************************************************
 ** 2.2 Global type definitions
 ******************************************************************************/
typedef struct client_state_s   uip_tcp_appstate_t, client_state_t;

struct client_state_s {
    uint8_t connected;
    uint8_t state;
    struct psock psock;
};

/******************************************************************************
 ** 2.3 Global macros
 ******************************************************************************/
/******************************************************************************
 ** 2.4 Global variables
 ******************************************************************************/
/******************************************************************************
 ** 2.5 Public function prototypes
 ******************************************************************************/
void client_appcall (void);
void client_connect (void);
void client_init    (void);

#ifndef UIP_APPCALL
#define UIP_APPCALL client_appcall
#endif

/******************************************************************************
 **
 ** END OF FILE
 **
 ******************************************************************************/
#ifdef	__cplusplus
}
#endif
#endif /* _CLIENT_H_ */