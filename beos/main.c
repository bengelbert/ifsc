#include <stdio.h>

#ifdef __AVR
#include <avr/common.h>
#include <avr/interrupt.h>
#endif

#include "beos.h"

int main()
{
#ifdef __AVR
    sei();
#endif
    beos_init();

    return 0;
}
