#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

/*      INCLUDES                                                    */

// C STANDARD
#include <stdint.h>     // uint*_t

// NRF
#include "ble_nus.h"    // BLE_NUS_MAX_DATA_LEN

/*      DEFINES                                                     */

// TX buffer max size
#define TX_BUF_SIZE     BLE_NUS_MAX_DATA_LEN - 1

// TX buffer and size.
typedef struct
{
    // Data to send.
    uint8_t     buffer[TX_BUF_SIZE];

    // Size in bytes.
    uint16_t    size;

} tx_buffer_t;

/* Enqueues the given message. Returns true if the operation could be
** performed, false otherwise.
*/
bool msg_queue_enqueue(const uint8_t* data, uint16_t size);

// Returns the last TX buffer in the queue, or NULL if it is empty.
tx_buffer_t* msg_queue_peek(void);

// Pops the last TX buffer from the queue.
void msg_queue_pop(void);

#endif /* ! MSG_QUEUE_H */
