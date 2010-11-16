/**
 * ====================================================================
 *  Filename  : hello.h
 *  Abstract  :
 *  Settings  :
 *  Contents  :
 *    Public:
 *    Private:
 *  Author    : bruno
 *  Http      :
 *  Mail      :
 *  Create on : 27 de Setembro de 2009, 15:19
 * ====================================================================
 */
#ifndef _HELLO_H_
#define _HELLO_H_

#ifdef	__cplusplus
extern "C" {
#endif
  /**
   * ====================================================================
   *  1   INCLUDE FILES
   *  1.1 Standart include files
   * ====================================================================
   */

  /**
   * ====================================================================
   *  1.2 Application include files
   * ====================================================================
   */
#include "uipopt.h"
#include "psock.h"

  /**
   * ====================================================================
   *  2   DECLARATIONS
   *  2.1 External constants
   * ====================================================================
   */

  /**
   * ====================================================================
   *  2.2 Global type definitions
   * ====================================================================
   */

  /*
   * Next, we define the uip_tcp_appstate_t datatype. This is the state
   * of our application, and the memory required for this state is
   * allocated together with each TCP connection. One application state
   * for each TCP connection.
   */
  typedef struct Hello_s {
    struct psock p;
    char inputbuffer[10];
    char name[40];
  } uip_tcp_appstate_t;

  /**
   * ====================================================================
   *  2.3 Global macros
   * ====================================================================
   */

  /**
   * ====================================================================
   *  2.4 Global variables
   * ====================================================================
   */

  /**
   * ====================================================================
   *  2.6 Public function prototypes
   * ====================================================================
   */
  /* Finally we define the application function to be called by uIP. */
  void hello_appCall (void);

#ifndef UIP_APPCALL
#define UIP_APPCALL hello_appCall
#endif /* UIP_APPCALL */

  void hello_init (void);

  /**
   * ====================================================================
   *  END OF FILE
   * ====================================================================
   */
#ifdef	__cplusplus
}
#endif
#endif /* _HELLO_H_ */

