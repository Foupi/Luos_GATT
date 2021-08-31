#ifndef PTP_SERVICE_H
#define PTP_SERVICE_H

/*      INCLUDES                                                    */

// C STANDARD
#include <stdint.h>     // uint16_t

// SOFTDEVICE
#include "ble_types.h"  // ble_uuid_t

/*      CONSTANTS                                                   */

/* Base UUID for the PTP service: 62A1XXXX-6806-4B01-BFAA-2104556C6508.
** Stored in big endian, with bytes 12 and 13 set to 0, will be filled
** with service or characteristic UUID.
*/
#define PTP_SERVICE_BASE_UUID   { 0x08, 0x65, 0x6C, 0x55, 0x04, 0x21,   \
                                  0xAA, 0xBF, 0x01, 0x4B, 0x06, 0x68,   \
                                /* Bytes 12 and 13. */                  \
                                  0x00, 0x00,                           \
                                /* ---------------- */                  \
                                  0xA1, 0x62,                           \
                                }

// 16-bit UUID for the PTP service.
#define PTP_SERVICE_UUID        0x0001

// 16-bit UUID for the PTP characteristic.
#define PTP_CHAR_UUID           0x0002

// Type alias for the PTP value characteristic.
typedef uint8_t ptp_char_value_t;

/* Registers the given UUID value with the PTP service base UUID in the
** BLE stack, and updates the value and type of the given UUID.
*/
void ptp_service_uuid_register(uint16_t uuid_val, ble_uuid_t* uuid);

#endif /* ! PTP_SERVICE_H */
