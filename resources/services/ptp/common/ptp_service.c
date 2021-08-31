#include "ptp_service.h"

/*      INCLUDES                                                    */

// C STANDARD
#include <stdint.h>     // uint16_t

// NRF
#include "sdk_errors.h" // ret_code_t

// NRF APPS
#include "app_error.h"  // APP_ERROR_CHECK

// SOFTDEVICE
#include "ble.h"        // sd_ble_uuid_vs_add
#include "ble_types.h"  // ble_uuid_t, ble_uuid128_t

void ptp_service_uuid_register(uint16_t uuid_val, ble_uuid_t* uuid)
{
    ble_uuid128_t base_uuid =
    {
        .uuid128 = PTP_SERVICE_BASE_UUID,
    };

    uuid->uuid = uuid_val;
    ret_code_t err_code = sd_ble_uuid_vs_add(&base_uuid, &(uuid->type));
    APP_ERROR_CHECK(err_code);
}
