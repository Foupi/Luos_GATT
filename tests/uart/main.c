/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>        // bool
#include <unistd.h>         // read

// NRF
#include "nrf_log.h"        // NRF_LOG_INFO

// NRF APPS
#include "app_uart.h"       // app_uart_*

// LUOS
#include "luos_hal_board.h" // LuosHAL_BoardInit

// CUSTOM
#include "uart_helpers.h"   // uart_init

/*      STATIC VARIABLES & CONSTANTS*/

// Size of a read operation.
#define READ_SIZE   8

/*      CALLBACKS                                                   */

// Prints the received characters.
static void uart_cb(app_uart_evt_t* event);

int main(void)
{
    LuosHAL_BoardInit();

    uart_init(uart_cb);

    while (true);
}

static void uart_cb(app_uart_evt_t* event)
{
    switch(event->evt_type)
    {
    case APP_UART_DATA_READY:
    {
        char received[READ_SIZE] = { 0 };
        ssize_t read_bytes = read(0, received, READ_SIZE);
        if (read_bytes == -1)
        {
            NRF_LOG_INFO("Read error!");
            break;
        }

        if (read_bytes == 0)
            break;

        NRF_LOG_INFO("Received %d bytes: %s!", read_bytes, received);
    }
        break;
    case APP_UART_TX_EMPTY:
        NRF_LOG_INFO("TX empty (\?\?\?)");
        break;
    case APP_UART_DATA:
        NRF_LOG_INFO("Non-FIFO data received (\?\?\?)");
        break;
    case APP_UART_FIFO_ERROR:
        NRF_LOG_INFO("Fifo error!");
        break;
    case APP_UART_COMMUNICATION_ERROR:
        NRF_LOG_INFO("Communication error!");
        break;
    default:
        NRF_LOG_INFO("Unknown type!");
        break;
    }
}
