#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "beos.h"

/******************************************************************************/

struct _beos
{
    int16_t (*get_ms_tick)(void);
};

/******************************************************************************/

int16_t beos_get_ms_tick(beos_t *this)
{
    if (this == NULL) return -BEOS_NULL_OBJ;
    
    if (this->get_ms_tick)
        return this->get_ms_tick();
    else
        return -BEOS_NULL_VTABLE;
}

/******************************************************************************/

void beos_init()
{
    debug("");
}

/******************************************************************************/

void beos_main_loop(beos_t *this)
{
    for (;;)
    {
#ifndef __AVR
        beos_get_ms_tick(this);
#else
        
#endif
    }
}

/******************************************************************************/

beos_t *beos_new(void)
{
#ifndef __AVR

    beos_t *this;

    this = (beos_t *) calloc(1, sizeof(beos_t));

    if (this == NULL) return NULL;
    
    debug("beos was allocated!!");

#else    
    
#endif

    
    
    
    return this;
}

/******************************************************************************/

beos_errno_t beos_vtable_insert_get_ms_tick(beos_t* this, int16_t (*get_ms_tick)(void))
{
    debug("");
    
    if (this == NULL) return BEOS_NULL_OBJ;
    
    this->get_ms_tick = get_ms_tick;
}

/******************************************************************************/
