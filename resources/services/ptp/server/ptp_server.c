#include "ptp_server.h"

/*      INCLUDES                                                    */

// C STANDARD
#include <stdint.h>         // uint16_t
#include <string.h>         // memset

// NRF
#include "nrf_log.h"        // NRF_LOG_INFO
#include "sdk_errors.h"     // ret_code_t

// NRF APPS
#include "app_error.h"      // APP_ERROR_CHECK

// SOFTDEVICE
#include "ble.h"            // ble_evt_t
#include "ble_err.h"        // 
#include "ble_gap.h"        // BLE_GAP_EVT_*, ble_gap_evt_t
#include "ble_gatts.h"      /* BLE_GATTS_SRVC_TYPE_PRIMARY,
                            ** sd_ble_gatts_service_add,
                            ** ble_gatts_evt_write_t, BLE_GATTS_EVT_*
                            ** ble_gatts_attr_md_t, ble_gatts_attr_t,
                            ** ble_gatts_char_md_t,
                            ** BLE_GATTS_VLOC_STACK,
                            ** sd_ble_gatts_characteristic_add
                            */
#include "ble_types.h"      // ble_uuid_t, BLE_CONN_HANDLE_INVALID

// CUSTOM
#include "ptp_service.h"    /* PTP_SERVICE_UUID,
                            ** ptp_service_uuid_register,
                            ** ptp_char_value_t
                            */

/*      GLOBAL/STATIC VARIABLES AND CONSTANTS                       */

// Type of the PTP service.
static const uint8_t PTP_SERVICE_TYPE = BLE_GATTS_SRVC_TYPE_PRIMARY;

// Global PTP server UUID initialization.
ble_uuid_t g_ptp_server_uuid;

/* PTP characteristic parameters. Static because documentation does not
** state if they are copied or if there is a risk of the stack
** overriding them...
*/
struct
{
    // PTP characteristic UUID.
    ble_uuid_t          uuid;

    // Value attribute metadata.
    ble_gatts_attr_md_t attr_md;

    // Value attribute.
    ble_gatts_attr_t    attr;

    // CCCD attribute metadata.
    ble_gatts_attr_md_t cccd_md;

    // Characteristic metadata.
    ble_gatts_char_md_t char_md;

    // Characteristic value.
    ptp_char_value_t    value;
} s_ptp_char;

/*      STATIC FUNCTIONS                                            */

/* Registers the PTP service in the BLE stack using the UUID of the
** given instance and update its handle.
*/
static void ptp_service_register(ptp_server_t* instance);

// Stores the connection handle from the event in the given instance.
static void ptp_server_on_connect_evt(const ble_gap_evt_t* event,
                                      ptp_server_t* instance);

// Resets the given instance's connection handle.
static void ptp_server_on_disconnect_evt(ptp_server_t* instance);

/* Calls the write callback of the given instance with the data from the
** given event.
*/
static void ptp_server_on_write_evt(const ble_gatts_evt_write_t* event,
                                    ptp_server_t* instance);

// Sets up and registers the PTP characteristic.
static void ptp_char_register(ptp_server_t* instance);

void ptp_server_init(ptp_server_t* instance,
                     const ptp_server_init_t* parameters)
{
    // Register service UUID in BLE stack.
    ptp_service_uuid_register(PTP_SERVICE_UUID, &g_ptp_server_uuid);

    // Set instance fields.
    instance->ptp_write_evt_handler = parameters->ptp_write_evt_handler;
    instance->conn_handle           = BLE_CONN_HANDLE_INVALID;

    // Register service in BLE stack.
    ptp_service_register(instance);

    // Register characteristic in BLE stack.
    ptp_char_register(instance);
}

void ptp_server_on_ble_evt(ble_evt_t const* event, void* context)
{
    ptp_server_t* instance = (ptp_server_t*)context;

    switch (event->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        ptp_server_on_connect_evt(&(event->evt.gap_evt), instance);
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        ptp_server_on_disconnect_evt(instance);
        break;
    case BLE_GATTS_EVT_WRITE:
        ptp_server_on_write_evt(&(event->evt.gatts_evt.params.write),
                                instance);
        break;
    default:
        break;
    }
}

void ptp_server_on_ptp_update(ptp_server_t* instance,
                              ptp_char_value_t value)
{
    if (instance->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        NRF_LOG_INFO("Connection handle not assigned: leaving...");
        return;
    }

    ble_gatts_hvx_params_t params;
    memset(&params, 0, sizeof(ble_gatts_hvx_params_t));

    uint16_t len = sizeof(ptp_char_value_t);

    params.handle   = instance->ptp_char_handles.value_handle;
    params.type     = BLE_GATT_HVX_NOTIFICATION;
    params.p_len    = &len;
    params.p_data   = (uint8_t*)(&value);

    ret_code_t err_code = sd_ble_gatts_hvx(instance->conn_handle,
                                           &params);
    if (err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    {
        NRF_LOG_INFO("CCCD not configured yet: leaving...");
        return;
    }
    APP_ERROR_CHECK(err_code);

    /* Exit after here: since len is only one byte, we assume everything
    ** was sent.
    */
}

static void ptp_service_register(ptp_server_t* instance)
{
    ret_code_t err_code = sd_ble_gatts_service_add(PTP_SERVICE_TYPE,
        &g_ptp_server_uuid, &(instance->service_handle));
    APP_ERROR_CHECK(err_code);
}

static void ptp_server_on_connect_evt(const ble_gap_evt_t* event,
                                      ptp_server_t* instance)
{
    instance->conn_handle = event->conn_handle;
}

static void ptp_server_on_disconnect_evt(ptp_server_t* instance)
{
    instance->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void ptp_server_on_write_evt(const ble_gatts_evt_write_t* event,
                                    ptp_server_t* instance)
{
    ble_uuid_t uuid = event->uuid;

    if (uuid.uuid == PTP_CHAR_UUID && uuid.type == s_ptp_char.uuid.type)
    {
        if (event->handle != instance->ptp_char_handles.value_handle)
        {
            return;
        }
        if (event->len == sizeof(ptp_char_value_t))
        {
            if (instance->ptp_write_evt_handler == NULL)
            {
                NRF_LOG_INFO("No PTP write event handler: leaving!");
                return;
            }
            ptp_char_value_t* value_ptr = (ptp_char_value_t*)(event->data);
            instance->ptp_write_evt_handler(*value_ptr, instance);
        }
    }
}

static void ptp_char_register(ptp_server_t* instance)
{
    // Register characteristic UUID.
    ptp_service_uuid_register(PTP_CHAR_UUID, &(s_ptp_char.uuid));

    // Initialize value attribute metadata.
    memset(&(s_ptp_char.attr_md), 0, sizeof(ble_gatts_attr_md_t));
    s_ptp_char.attr_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(s_ptp_char.attr_md.read_perm));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(s_ptp_char.attr_md.write_perm));

    // Initialize value attribute.
    memset(&(s_ptp_char.attr), 0, sizeof(ble_gatts_attr_t));
    s_ptp_char.attr.p_uuid      = &(s_ptp_char.uuid);
    s_ptp_char.attr.p_attr_md   = &(s_ptp_char.attr_md);
    s_ptp_char.attr.init_len    = sizeof(ptp_char_value_t);
    s_ptp_char.attr.max_len     = sizeof(ptp_char_value_t);
    s_ptp_char.attr.p_value     = (uint8_t*)(&(s_ptp_char.value));

    // Initialize CCCD attribute metadata.
    memset(&(s_ptp_char.cccd_md), 0, sizeof(ble_gatts_attr_md_t));
    s_ptp_char.cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(s_ptp_char.cccd_md.read_perm));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(s_ptp_char.cccd_md.write_perm));

    memset(&(s_ptp_char.char_md), 0, sizeof(ble_gatts_char_md_t));
    s_ptp_char.char_md.p_cccd_md                = &(s_ptp_char.cccd_md);
    s_ptp_char.char_md.char_props.write_wo_resp = 1;
    s_ptp_char.char_md.char_props.notify        = 1;

    ret_code_t err_code;
    err_code = sd_ble_gatts_characteristic_add(instance->service_handle,
        &(s_ptp_char.char_md), &(s_ptp_char.attr),
        &(instance->ptp_char_handles));
    APP_ERROR_CHECK(err_code);
}
