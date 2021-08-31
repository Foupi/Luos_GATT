#include "ptp_client.h"

/*      INCLUDES                                                    */

// NRF
#include "ble_db_discovery.h"   // ble_db_discovery_evt_register
#include "ble_gatt_db.h"        // ble_gatt_db_srv_t, ble_gatt_db_char_t
#include "nrf_log.h"            // NRF_LOG_INFO
#include "sdk_errors.h"         // ret_code_t

// NRF APPS
#include "app_error.h"          // APP_ERROR_CHECK

// SOFTDEVICE
#include "ble.h"                // ble_evt_t
#include "ble_gap.h"            // ble_gap_evt_t, BLE_GAP_EVT_*
#include "ble_gatt.h"           // BLE_GATT_*
#include "ble_gattc.h"          /* ble_gattc_*, BLE_GATTC_EVT_*,
                                ** sd_ble_gattc_write
                                */
#include "ble_types.h"          // ble_uuid_t, BLE_CONN_HANDLE_INVALID

// CUSTOM
#include "ptp_service.h"        /* ptp_service_uuid_register,
                                ** PTP_SERVICE_UUID, g_ptp_service_uuid
                                */

/*      GLOBAL/STATIC VARIABLES AND CONSTANTS                       */

// Global PTP server UUID initialization.
ble_uuid_t g_ptp_service_uuid;

/*      STATIC FUNCTIONS                                            */

// Stores the connection handle from the event in the given instance.
static void ptp_client_on_connect_evt(const ble_gap_evt_t* event,
                                      ptp_client_t* instance);

// Resets the given instance's connection handle.
static void ptp_client_on_disconnect_evt(ptp_client_t* instance);

/* Calls the notification callback of the given instance with the data
** from the given event.
*/
static void ptp_client_on_hvx_evt(const ble_gattc_evt_hvx_t* event,
                                  ptp_client_t* instance);

void ptp_client_init(ptp_client_t* instance,
                     const ptp_client_init_t* parameters)
{
    // Register service UUID in BLE stack.
    ptp_service_uuid_register(PTP_SERVICE_UUID, &g_ptp_service_uuid);

    // Initialize instance fields.
    instance->evt_handler       = parameters->evt_handler;
    instance->conn_handle       = BLE_CONN_HANDLE_INVALID;
    instance->ptp_value_handle  = BLE_GATT_HANDLE_INVALID;
    instance->ptp_cccd_handle   = BLE_GATT_HANDLE_INVALID;

    // Register in DB Discovery module.
    ret_code_t err_code = ble_db_discovery_evt_register(&g_ptp_service_uuid);
    APP_ERROR_CHECK(err_code);
}

void ptp_client_on_ble_evt(ble_evt_t const* event, void* context)
{
    ptp_client_t* instance = (ptp_client_t*)context;

    switch (event->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        ptp_client_on_connect_evt(&(event->evt.gap_evt), instance);
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        ptp_client_on_disconnect_evt(instance);
        break;
    case BLE_GATTC_EVT_HVX:
        ptp_client_on_hvx_evt(&(event->evt.gattc_evt.params.hvx),
                              instance);
        break;
    default:
        break;
    }
}

void ptp_client_on_db_discovery_evt(ble_db_discovery_evt_t* event,
                                    ptp_client_t* instance)
{
    if (event->evt_type != BLE_DB_DISCOVERY_COMPLETE)
    {
        NRF_LOG_INFO("PTP client: DB Discovery incomplete: leaving...");
    }

    ble_gatt_db_srv_t disc_db = event->params.discovered_db;

    if (disc_db.srv_uuid.uuid != PTP_SERVICE_UUID
        || disc_db.srv_uuid.type != g_ptp_service_uuid.type)
    {
        return;
    }


    uint8_t nb_chars = disc_db.char_count;

    ptp_client_db_t event_db;
    memset(&event_db, 0, sizeof(ptp_client_db_t));

    for (uint8_t char_idx = 0; char_idx < nb_chars; char_idx++)
    {
        ble_gatt_db_char_t char_db = disc_db.charateristics[char_idx];
        ble_gattc_char_t gatt_char = char_db.characteristic;

        switch(gatt_char.uuid.uuid)
        {
        case PTP_CHAR_UUID:
            event_db.ptp_value_handle   = gatt_char.handle_value;
            event_db.ptp_cccd_handle    = char_db.cccd_handle;
            break;
        default:
            break;
        }
    }

    ptp_client_evt_t ptp_event;
    memset(&ptp_event, 0, sizeof(ptp_client_evt_t));

    ptp_event.evt_type          = PTP_C_DB_DISCOVERY_COMPLETE;
    ptp_event.content.disc_db   = event_db;

    if (instance->evt_handler != NULL)
    {
        instance->evt_handler(&ptp_event, instance);
    }
    else
    {
        NRF_LOG_INFO("No PTP event handler!");
    }
}

void ptp_client_handles_assign(ptp_client_t* instance,
                               const ptp_client_db_t* ptp_db)
{
    instance->ptp_value_handle  = ptp_db->ptp_value_handle;
    instance->ptp_cccd_handle   = ptp_db->ptp_cccd_handle;

    NRF_LOG_INFO("PTP characteristic handles assigned!");
}

void ptp_client_ptp_char_write(ptp_client_t* instance,
                               ptp_char_value_t val)
{
    if (instance->ptp_value_handle == BLE_GATT_HANDLE_INVALID)
    {
        NRF_LOG_INFO("Value handle is not initialized: leaving...");
        return;
    }

    ble_gattc_write_params_t params;
    memset(&params, 0, sizeof(ble_gattc_write_params_t));

    params.write_op = BLE_GATT_OP_WRITE_CMD;
    params.handle   = instance->ptp_value_handle;
    params.len      = sizeof(ptp_char_value_t);
    params.p_value  = (uint8_t*)(&val);

    ret_code_t err_code = sd_ble_gattc_write(instance->conn_handle,
                                             &params);
    APP_ERROR_CHECK(err_code);
}

void ptp_client_ptp_notification_enable(ptp_client_t* instance,
                                        bool enable)
{
    ble_gattc_write_params_t params;
    memset(&params, 0, sizeof(ble_gattc_write_params_t));

    uint8_t value;
    if (enable)
    {
        value = BLE_GATT_HVX_NOTIFICATION;
    }
    else
    {
        value = BLE_GATT_HVX_INVALID;
    }

    params.write_op = BLE_GATT_OP_WRITE_CMD;
    params.handle   = instance->ptp_cccd_handle;
    params.len      = sizeof(value);
    params.p_value  = (uint8_t*)(&value);

    ret_code_t err_code = sd_ble_gattc_write(instance->conn_handle,
                                             &params);
    APP_ERROR_CHECK(err_code);
}

static void ptp_client_on_connect_evt(const ble_gap_evt_t* event,
                                      ptp_client_t* instance)
{
    instance->conn_handle = event->conn_handle;
}

static void ptp_client_on_disconnect_evt(ptp_client_t* instance)
{
    instance->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void ptp_client_on_hvx_evt(const ble_gattc_evt_hvx_t* event,
                                  ptp_client_t* instance)
{
    if (event->handle == instance->ptp_value_handle)
    {
        if (event->len != sizeof(ptp_char_value_t))
        {
            return;
        }

        ptp_char_value_t* value_ptr = (ptp_char_value_t*)event->data;

        ptp_client_evt_t ptp_evt;
        memset(&ptp_evt, 0, sizeof(ptp_client_evt_t));

        ptp_evt.evt_type        = PTP_C_NOTIFICATION_RECEIVED;
        ptp_evt.content.value   = *value_ptr;

        if (instance->evt_handler != NULL)
        {
            instance->evt_handler(&ptp_evt, instance);
        }
        else
        {
            NRF_LOG_INFO("No PTP event handler!");
        }
    }
}
