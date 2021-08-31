#ifndef PTP_CLIENT_H
#define PTP_CLIENT_H

/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>            // bool
#include <stddef.h>             // NULL
#include <stdint.h>             // uint16_t

// NRF
#include "ble_db_discovery.h"   // ble_db_discovery_evt_t
#include "nrf_sdh_ble.h"        // NRF_SDH_BLE_OBSERVER

// SOFTDEVICE
#include "ble.h"                // ble_evt_t
#include "ble_types.h"          // ble_uuid_t

// CUSTOM
#include "ptp_service.h"        // ptp_char_value_t

/*      CONSTANTS                                                   */

// PTP client BLE observer priority.
#define PTP_CLIENT_BLE_OBS_PRIO 3

// Defines a PTP client instance and assigns its BLE observer.
#define PTP_CLIENT_DEF(_instance_name)                  \
    static ptp_client_t _instance_name;                 \
    NRF_SDH_BLE_OBSERVER(_instance_name ## _ble_obs,    \
        PTP_CLIENT_BLE_OBS_PRIO,                        \
        ptp_client_on_ble_evt,                          \
        &_instance_name                                 \
    )/*;*/

// PTP client event type.
typedef enum
{
    // Database discovery completed.
    PTP_C_DB_DISCOVERY_COMPLETE,

    // Notification received from server.
    PTP_C_NOTIFICATION_RECEIVED,
} ptp_client_evt_type_t;

// Result of a DB Discovery.
typedef struct
{
    // Handle for the PTP value attribute.
    uint16_t    ptp_value_handle;

    // Handle for the PTP CCCD attribute.
    uint16_t    ptp_cccd_handle;
} ptp_client_db_t;

// PTP Client event.
typedef struct
{
    // Event type.
    ptp_client_evt_type_t   evt_type;

    // Content of the event.
    union
    {
        // PTP_C_DB_DISCOVERY_COMPLETE: Discovered database.
        ptp_client_db_t     disc_db;

        // PTP_C_NOTIFICATION_RECEIVED: Value received in notification.
        ptp_char_value_t    value;
    }                       content;

} ptp_client_evt_t;

// Forward declaration.
typedef struct ptp_client_s ptp_client_t;

// PTP client event handler.
typedef void(*ptp_client_evt_handler_t)(const ptp_client_evt_t* event,
                                        ptp_client_t* instance);

// Parameters needed to initialize a PTP client instance.
typedef struct
{
    // PTP client event handler.
    ptp_client_evt_handler_t    evt_handler;
} ptp_client_init_t;

// PTP client instance.
struct ptp_client_s
{
    // PTP server connection handle.
    uint16_t                    conn_handle;

    // FIXME Maybe these two fields could be put in a struct...

    // Value handle for the PTP characteristic.
    uint16_t                    ptp_value_handle;

    // CCCD handle for the PTP characteristic.
    uint16_t                    ptp_cccd_handle;

    // Event handler for this instance.
    ptp_client_evt_handler_t    evt_handler;
};

// Initializes the given PTP client instance with the given parameters.
void ptp_client_init(ptp_client_t* instance,
                     const ptp_client_init_t* parameters);

/* Connection:      Stores the connection handle.
** Disconnection:   Resets the connection handle.
** Notification:    Creates a PTP_C_NOTIFICATION_RECEIVED event.
*/
void ptp_client_on_ble_evt(ble_evt_t const* event, void* context);

/* DB Discovery complete:   If conditions are met, retrieves required
**                          handles.
*/
void ptp_client_on_db_discovery_evt(ble_db_discovery_evt_t* event,
                                    ptp_client_t* instance);

/* Assigns the internal handles of the given instance with those present
** in the given DB.
*/
void ptp_client_handles_assign(ptp_client_t* instance,
                               const ptp_client_db_t* ptp_db);

// Writes the given value on the server connected to the given instance.
void ptp_client_ptp_char_write(ptp_client_t* instance,
                               ptp_char_value_t val);

// Enable notification for the PTP characteristic.
void ptp_client_ptp_notification_enable(ptp_client_t* instance,
                                        bool enable);

// UUID of the PTP service.
extern ble_uuid_t g_ptp_service_uuid;

#endif /* ! PTP_CLIENT_H */
