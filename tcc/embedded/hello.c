/**
 * ====================================================================
 *  Filename  : hello.c
 *  Abstract  :
 *  Settings  :
 *  Contents  :
 *    Public:
 *    Private:
 *  Author    : bruno
 *  Http      :
 *  Mail      :
 *  Create on : 27 de Setembro de 2009, 15:17
 * ====================================================================
 */

/**
 * ====================================================================
 *  1   INCLUDE FILES
 *  1.1 Standart include files
 * ====================================================================
 */
#include <string.h>

/**
 * ====================================================================
 *  1.2 Application include files
 * ====================================================================
 */
#include "hello.h"
#include "uip.h"

/**
 * ====================================================================
 *  2   DECLARATIONS
 *  2.1 Internal constants
 * ====================================================================
 */

/**
 * ====================================================================
 *  2.2 Internal type definitions
 * ====================================================================
 */

/**
 * ====================================================================
 *  2.3 Internal macros
 * ====================================================================
 */

/**
 * ====================================================================
 *  2.4 Internal variables
 * ====================================================================
 */

/**
 * ====================================================================
 *  2.5 Global variables (declared as 'extern' in some header files)
 * ====================================================================
 */

/**
 * ====================================================================
 *  2.6 Private function prototypes (defined in Section 5)
 * ====================================================================
 */
/*
 * Declaration of the protosocket function that handles the connection
 * (defined at the end of the code).
 */
static int handle_connection (struct Hello_s *s);

/**
 * ====================================================================
 *  3 PUBLIC FUNCTIONS (declared in Section 2.6 on hello.h)
 * ====================================================================
 */

/*
 * The initialization function. We must explicitly call this function
 * from the system initialization code, some time after uip_init() is
 * called.
 */
void hello_init (void)
{
  /* We start to listen for connections on TCP port 1000. */
  uip_listen(HTONS(1000));
}

/*----------------------------------------------------------------------------*/

/*
 * In hello-world.h we have defined the UIP_APPCALL macro to
 * hello_world_appcall so that this funcion is uIP’s application
 * function. This function is called whenever an uIP event occurs
 * (e.g. when a new connection is established, new data arrives, sent
 * data is acknowledged, data needs to be retransmitted, etc.).
 */
void hello_appCall (void)
{
  /*
   * The uip_conn structure has a field called "appstate" that holds
   * the application state of the connection. We make a pointer to
   * this to access it easier.
   */
  struct Hello_s *s = &(uip_conn->appstate);
  /*
   * If a new connection was just established, we should initialize
   * the protosocket in our applications’ state structure.
   */
  if (uip_connected()) {
    PSOCK_INIT(&s->p, s->inputbuffer, sizeof (s->inputbuffer));
  }
  /*
   * Finally, we run the protosocket function that actually handles
   * the communication. We pass it a pointer to the application state
   * of the current connection.
   */
  handle_connection(s);
}

/*----------------------------------------------------------------------------*/

/*
 * This is the protosocket function that handles the communication. A
 * protosocket function must always return an int, but must never
 * explicitly return - all return statements are hidden in the PSOCK
 * macros.
 */
static int handle_connection (struct Hello_s *s)
{
  PSOCK_BEGIN(&s->p);
  PSOCK_SEND_STR(&s->p, "Hello. What is your name?\n");
  PSOCK_READTO(&s->p, '\n');
  strncpy(s->name, s->inputbuffer, sizeof (s->name));
  PSOCK_SEND_STR(&s->p, "Hello ");
  PSOCK_SEND_STR(&s->p, s->name);
  PSOCK_CLOSE(&s->p);
  PSOCK_END(&s->p);
}

/**
 * ====================================================================
 *  4 PRIVATE FUNCTIONS (declared on Section 2.6)
 * ====================================================================
 */

/**
 * ====================================================================
 *  END OF FILE
 * ====================================================================
 */
