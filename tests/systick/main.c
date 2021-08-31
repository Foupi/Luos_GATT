/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>            // bool
#include <stdint.h>             // uint32_t

// NRF
#include "nrf_log.h"            // NRF_LOG_INFO

// LUOS
#include "luos_hal.h"           // LuosHAL_GetSystick
#include "luos_hal_board.h"     // LuosHAL_BoardInit
#include "luos_hal_systick.h"   // LuosHAL_SystickInit

int main(void)
{
    LuosHAL_BoardInit();
    LuosHAL_SystickInit();

    while(true)
    {
        uint32_t curr_tick = LuosHAL_GetSystick();
        NRF_LOG_INFO("Current tick: %u!", curr_tick);
    }
}
