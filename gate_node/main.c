/*      INCLUDES                                                    */

// C STANDARD LIBRARY
#include <stdbool.h>        // bool

// LUOS
#include "luos.h"           // Luos_Init, Luos_Loop
#include "gate.h"           // Gate_Init, Gate_Loop
#include "led_toggler.h"    // LedToggler_Init, LedToggler_Loop


int main(void)
{
    Luos_Init();
    Gate_Init();
    LedToggler_Init();

    while (true)
    {
        Luos_Loop();
        Gate_Loop();
        LedToggler_Loop();
    }
}

