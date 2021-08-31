#include "msg_queue.h"

/*      INCLUDES                                                    */

// C STANDARD
#include <stdint.h>     // uint*_t
#include <string.h>     // memcpy

// NRF
#include "ble_nus.h"    // BLE_NUS_MAX_DATA_LEN

#ifdef DEBUG
#include "nrf_log.h"    // NRF_LOG_INFO
#endif /* DEBUG */

/*      STATIC VARIABLES & CONSTANTS                                */

// Number of buffers.
#define NB_TX_BUFFERS   8

// Buffers table.
static tx_buffer_t  s_msg_queue[NB_TX_BUFFERS]  = { 0 };

// Current buffer to send.
static uint16_t     s_current_buffer_idx        = 0;

// Index of next enqueued buffer.
static uint16_t     s_msg_queue_idx             = 0;

bool msg_queue_enqueue(const uint8_t* data, uint16_t size)
{
    tx_buffer_t* new_head = s_msg_queue + s_msg_queue_idx;

    if (new_head->size != 0)
    {
        // Current insertion buffer is not available.
        return false;
    }

    memcpy(new_head->buffer, data, size);
    new_head->size = size;

    s_msg_queue_idx++;
    s_msg_queue_idx %= NB_TX_BUFFERS;

    return true;
}

tx_buffer_t* msg_queue_peek(void)
{
    tx_buffer_t* tail = s_msg_queue + s_current_buffer_idx;

    if (tail->size == 0)
    {
        // Buffer is not in use: queue is empty.
        return NULL;
    }

    return tail;
}

void msg_queue_pop(void)
{
    // Buffer is not used anymore.
    s_msg_queue[s_current_buffer_idx].size = 0;

    s_current_buffer_idx++;
    s_current_buffer_idx %= NB_TX_BUFFERS;
}
