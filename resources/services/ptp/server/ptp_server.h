#ifndef PTP_SERVER_H
#define PTP_SERVER_H

/*      INCLUDES                                                    */

// C STANDARD
#include <stdint.h>         // uint16_t

// NRF
#include "nrf_sdh_ble.h"    // NRF_SDH_BLE_OBSERVER

// SOFTDEVICE
#include "ble.h"            // ble_evt_t
#include "ble_types.h"      // ble_uuid_t

// CUSTOM
#include "ptp_service.h"    // ptp_char_value_t

/*      CONSTANTS                                                   */

// PTP server BLE observer priority.
#define PTP_SERVER_BLE_OBS_PRIO 3

// Defines a PTP server instance and assigns its BLE observer.
#define PTP_SERVER_DEF(_instance_name)                  \
    static ptp_server_t _instance_name;                 \
    NRF_SDH_BLE_OBSERVER(_instance_name ## _ble_obs,    \
        PTP_SERVER_BLE_OBS_PRIO,                        \
        ptp_server_on_ble_evt,                          \
        &_instance_name                                 \
    )/*;*/

// Forward declaration.
typedef struct ptp_server_s ptp_server_t;

// Callback for write event on PTP characteristic.
typedef void(*ptp_server_ptp_write_evt_handler_t)(ptp_char_value_t ptp_val,
                                                  ptp_server_t* instance);

// Parameters needed to initialize a PTP server instance.
typedef struct
{
    // Callback for write event on PTP characteristic.
    ptp_server_ptp_write_evt_handler_t  ptp_write_evt_handler;

} ptp_server_init_t;

// PTP server instance.
struct ptp_server_s
{
    // Handle to the service.
    uint16_t                            service_handle;

    // Handle to the client connection.
    uint16_t                            conn_handle;

    // Handles to the attributes defining the PTP characteristic.
    ble_gatts_char_handles_t            ptp_char_handles;

    // Callback for write event on PTP characteristic.
    ptp_server_ptp_write_evt_handler_t  ptp_write_evt_handler;

};

// Initializes the given PTP server instance with the given parameters.
void ptp_server_init(ptp_server_t* instance,
                     const ptp_server_init_t* parameters);

/* Connection:      Stores the connection handle.
** Disconnection:   Resets the connection handle.
** Write event:     Calls the stored callback.
*/
void ptp_server_on_ble_evt(ble_evt_t const* event, void* context);

// Notifies the PTP client on PTP characteristic update.
void ptp_server_on_ptp_update(ptp_server_t* instance,
                              ptp_char_value_t value);

// UUID of the PTP server.
extern ble_uuid_t g_ptp_server_uuid;

#endif /* ! PTP_SERVER_H */
