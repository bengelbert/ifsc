#include <stdio.h>

#ifndef __AVR
#include <sys/time.h>
#else
#include <avr/common.h>
#include <avr/interrupt.h>
#endif

#include "beos.h"

/******************************************************************************/

#ifndef __AVR
uint16_t main_unix_init(void)
{
    
}

#else

uint16_t main_avr_init(void)
{
    
}

#endif

/******************************************************************************/

int16_t main_get_ms_tick(void)
{
    int16_t time_in_mill;
    struct timeval tv;
    
    gettimeofday(&tv, NULL);

    time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;    
    
    return time_in_mill;
}

/******************************************************************************/

int main()
{
    beos_t *beos;

    beos = beos_new();
    beos_vtable_insert_get_ms_tick(beos, main_get_ms_tick);
    
    debug("counter: %d", beos_get_ms_tick(beos));
    
    beos_main_loop(beos);        
    
    return 0;
}
