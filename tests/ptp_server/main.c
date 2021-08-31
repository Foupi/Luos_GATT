/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>        // bool
#include <string.h>         // memset

// NRF
#include "boards.h"         // bsp_board_led_*

// NRF APPS
#include "app_button.h"     // app_button_*
#include "app_error.h"      // APP_ERROR_CHECK
#include "app_timer.h"      // APP_TIMER_TICKS

// SOFTDEVICE
#include "ble_types.h"      // BLE_CONN_HANDLE_INVALID

// HAL
#include "luos_hal_board.h" // LuosHAL_BoardInit
#include "luos_hal_ble.h"   /* LuosHAL_BleInit, LuosHAL_BleSetup,
                            ** LuosHAL_BleConnect
                            */

// CUSTOM
#include "ptp_server.h"     // PTP_SERVER_DEF, ptp_server_*
#include "ptp_service.h"    // ptp_char_value_t

/*      STATIC VARIABLES AND CONSTANTS                              */

// PTP server instance.
PTP_SERVER_DEF(s_ptp_server);

// Button to press and LED to toggle.
#define                 BUTTON_IDX  BSP_BUTTON_0
static const uint8_t    LED_IDX     = BSP_BOARD_LED_0;

// Button detection delay.
static const uint32_t BTN_DTX_DELAY = APP_TIMER_TICKS(50);

/*      STATIC FUNCTIONS                                            */

// Initializes the static PTP server.
static void init_ptp_server(void);

// Initializes the button functionality.
static void init_button(void);

// Sets the state of the app LED: on if `true`, off if `false`.
static void set_led_state(bool state);

/*      CALLBACKS                                                   */

// Toggles the LED according to the received state.
static void ptp_server_on_ptp_write_evt(ptp_char_value_t value,
                                        ptp_server_t* instance);

/* If the index is the defined one, toggles the LED and writes on
** server.
*/
static void button_evt_handler(uint8_t btn_idx, uint8_t event);

int main(void)
{
    LuosHAL_BoardInit();

    LuosHAL_BleInit();

    init_ptp_server();
    init_button();

    LuosHAL_BleSetup();
    LuosHAL_BleConnect();

    while (s_ptp_server.conn_handle == BLE_CONN_HANDLE_INVALID);
    app_button_enable();

    while (true);
}

static void init_ptp_server(void)
{
    ptp_server_init_t params;
    memset(&params, 0, sizeof(ptp_server_init_t));

    params.ptp_write_evt_handler =  ptp_server_on_ptp_write_evt;

    ptp_server_init(&s_ptp_server, &params);
}

static void init_button(void)
{
    static app_button_cfg_t buttons[] =
    {
        {
            BUTTON_IDX,
            APP_BUTTON_ACTIVE_LOW,
            BUTTON_PULL,
            button_evt_handler,
        },
    };

    ret_code_t err_code = app_button_init(buttons, 1, BTN_DTX_DELAY);
    APP_ERROR_CHECK(err_code);
}

static void set_led_state(bool state)
{
    if (state)
    {
        bsp_board_led_on(LED_IDX);
    }
    else
    {
        bsp_board_led_off(LED_IDX);
    }
}

static void ptp_server_on_ptp_write_evt(ptp_char_value_t value,
                                        ptp_server_t* instance)
{
    bool state = (bool)value;

    set_led_state(state);
}

static void button_evt_handler(uint8_t btn_idx, uint8_t event)
{
    if (btn_idx != BUTTON_IDX)
    {
        return;
    }

    bool state = (event == APP_BUTTON_PUSH);
    set_led_state(state);

    ptp_server_on_ptp_update(&s_ptp_server, (ptp_char_value_t)state);
}
