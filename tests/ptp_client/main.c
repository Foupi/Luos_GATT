/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>        // bool
#include <string.h>         // memset

// NRF
#include "ble_nus_c.h"      // ble_nus_c_t
#include "boards.h"         // bsp_board_led_*
#include "nrf_log.h"        // NRF_LOG_INFO
#include "sdk_errors.h"     // ret_code_t

// NRF APPS
#include "app_button.h"     // app_button_*
#include "app_error.h"      // APP_ERROR_CHECK
#include "app_timer.h"      // APP_TIMER_TICKS

// HAL
#include "luos_hal_board.h" // LuosHAL_BoardInit
#include "luos_hal_ble.h"   /* LuosHAL_BleInit, LuosHAL_BleSetup,
                            ** LuosHAL_BleConnect
                            */

// CUSTOM
#include "luos_hal_ble_client_ctx.h"    // g_ptp_client_ptr
#include "ptp_client.h"     // PTP_CLIENT_DEF, ptp_client_*

/*      GLOBAL/STATIC VARIABLES & CONSTANTS                         */

// PTP Client instance.
PTP_CLIENT_DEF(s_ptp_client);

// Global accessor to PTP instance.
ptp_client_t* g_ptp_client_ptr;

// Undefined-reference-avoider.
ble_nus_c_t*    g_nus_c_ptr;

// Button to press and LED to toggle.
#define                 BUTTON_IDX  BSP_BUTTON_0
static const uint8_t    LED_IDX     = BSP_BOARD_LED_0;

// Button detection delay.
static const uint32_t BTN_DTX_DELAY = APP_TIMER_TICKS(50);

/*      STATIC FUNCTIONS                                            */

// Initializes the static PTP client instance.
static void init_ptp_client(void);

// Initializes the button functionality.
static void init_button(void);

// Sets the state of the app LED: on if `true`, off if `false`.
static void set_led_state(bool state);

/*      CALLBACKS                                                   */

/* DB Discovery complete:   Assigns the internal handles.
** Notification:            Sets the LED state accordingly.
*/
static void ptp_client_evt_handler(const ptp_client_evt_t* event,
                                   ptp_client_t* instance);

/* If the index is the defined one, toggles the LED and writes on
** server.
*/
static void button_evt_handler(uint8_t btn_idx, uint8_t event);

int main(void)
{
    LuosHAL_BoardInit();

    LuosHAL_BleInit();

    init_ptp_client();
    init_button();

    LuosHAL_BleSetup();
    LuosHAL_BleConnect();

    while (true);
}

static void init_ptp_client(void)
{
    ptp_client_init_t params;
    memset(&params, 0, sizeof(ptp_client_init_t));

    params.evt_handler = ptp_client_evt_handler;

    ptp_client_init(&s_ptp_client, &params);

    g_ptp_client_ptr = &s_ptp_client;
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

static void ptp_client_evt_handler(const ptp_client_evt_t* event,
                                   ptp_client_t* instance)
{
    switch (event->evt_type)
    {
    case PTP_C_DB_DISCOVERY_COMPLETE:
        ptp_client_handles_assign(instance, &(event->content.disc_db));
        ptp_client_ptp_notification_enable(instance, true);
        app_button_enable();
        break;
    case PTP_C_NOTIFICATION_RECEIVED:
        set_led_state((bool)(event->content.value));
        break;
    default:
        NRF_LOG_INFO("PTP client: Unknown event!");
        break;
    }
}

static void button_evt_handler(uint8_t btn_idx, uint8_t event)
{
    if (btn_idx != BUTTON_IDX)
    {
        return;
    }

    bool state = (event == APP_BUTTON_PUSH);
    set_led_state(state);
    ptp_client_ptp_char_write(&s_ptp_client, (ptp_char_value_t)state);
}
