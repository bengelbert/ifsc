#ifndef __BEOS__
#define __BEOS__

/******************************************************************************/

#include <stdint.h>

/******************************************************************************/

typedef struct _beos beos_t;

typedef enum beos_errno {
    BEOS_OK,
    
    BEOS_NULL_OBJ,
    BEOS_NULL_VTABLE
} beos_errno_t;

/******************************************************************************/

void    beos_init(void);
beos_t *beos_new(void);
beos_errno_t beos_vtable_insert_get_ms_tick(beos_t* this, int16_t (*get_ms_tick)(void));

#define debug(format, var...)  printf("%s(): "format"\n", __FUNCTION__, ##var)

#endif
