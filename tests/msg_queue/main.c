/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>        // bool
#include <string.h>         // strncmp
#include <unistd.h>         // read

// NRF
#include "nrf_log.h"        // NRF_LOG_INFO

// NRF APPS
#include "app_uart.h"       // app_uart_*

// LUOS
#include "luos_hal_board.h" // LuosHAL_BoardInit

// CUSTOM
#include "msg_queue.h"      // msg_queue_*, TX_BUF_SIZE, tx_buffer_t
#include "uart_helpers.h"   // uart_init

/*      STATIC VARIABLES & CONSTANTS                                */

// Internal buffer
static uint8_t          s_internal_buffer[TX_BUF_SIZE + 1]  = { 0 };

// Internal index
static uint16_t         s_internal_index                    = 0;

// Stop char
static const char       STOP_CHAR                           = '\r';

// Dequeue string
#define                 DEQUEUE_STRING                      "dequeue"
static const uint16_t   DEQUEUE_STRING_SIZE                 = sizeof(DEQUEUE_STRING) - 1;

/*      STATIC FUNCTIONS                                            */

/* If the received message is complete (stop char received):
** -    If the received message corresponds to the dequeue word, logs
**      the last word in the queue and pops it.
** -    If the queue is full, logs the last word in the queue and pops
**      it, then enqueues the received word.
** -    Else, enqueues the received word.
*/
static void manage_received_data(void);

// Enqueues the given amount of bytes from the internal buffer.
static bool enqueue_internal_buffer(uint16_t nb_chars);

// Pops the last message from the queue and prints it.
static bool dequeue_print(void);

/*      CALLBACKS                                                   */

/* Data ready:  Calls the data management function.
** TX empty:    Not supposed to happen.
** UART data:   Not supposed to happen.
** FIFO error:  Logs error.
** Com error:   Logs error.
*/
static void uart_cb(app_uart_evt_t* event);

int main(void)
{
    LuosHAL_BoardInit();

    uart_init(uart_cb);

    while (true);
}

static void manage_received_data(void)
{
    ssize_t read_bytes = read(0, s_internal_buffer + s_internal_index,
                              TX_BUF_SIZE - s_internal_index);
    if (read_bytes <= 0)
    {
        if (read_bytes == -1)
        {
            NRF_LOG_INFO("Read error!");
        }

        return;
    }

    s_internal_index += read_bytes;
    uint16_t last_index = s_internal_index - 1;

    if ((s_internal_index < TX_BUF_SIZE)
        && (s_internal_buffer[last_index] != STOP_CHAR))
    {
        // Message is incomplete: we need to receive more.
        NRF_LOG_INFO("Message incomplete! (current buffer: %s)",
                     (char*)s_internal_buffer);

        return;
    }

    s_internal_index = 0;

    uint16_t sent_size;
    if (s_internal_index >= TX_BUF_SIZE)
    {
        NRF_LOG_INFO("Truncating received message!");
        sent_size = TX_BUF_SIZE;
    }
    else
    {
        NRF_LOG_INFO("Message complete!");

        s_internal_buffer[last_index] = '\0';

        int cmp_res = strncmp((char*)s_internal_buffer, DEQUEUE_STRING,
                             DEQUEUE_STRING_SIZE);

        if (cmp_res == 0)
        {
            dequeue_print();
            return;
        }

        sent_size = last_index;
    }

    s_internal_index = 0;

    bool enqueue_success = enqueue_internal_buffer(sent_size);
    if (enqueue_success)
    {
        // Nothing left to do.
        return;
    }

    NRF_LOG_INFO("Enqueue failed!");

    bool dequeue_success = dequeue_print();
    if (!dequeue_success)
    {
        NRF_LOG_INFO("Both enqueue and dequeue failed! (\?\?\?)");
        return;
    }

    // Once a message was popped, another one can fit in.
    enqueue_success = enqueue_internal_buffer(sent_size);
    if (!enqueue_success)
    {
        NRF_LOG_INFO("Enqueue failed after pop! (\?\?\?)");
        return;
    }
}

static bool enqueue_internal_buffer(uint16_t nb_chars)
{
    bool enqueue_success = msg_queue_enqueue(s_internal_buffer,
                                             nb_chars);

    if (enqueue_success)
    {
        NRF_LOG_INFO("Enqueuing %s (size: %u)!",
                     (char*)s_internal_buffer, nb_chars);
    }
    else
    {
        NRF_LOG_INFO("Message queue full!");
    }
    return enqueue_success;
}

static bool dequeue_print(void)
{
    const tx_buffer_t* queue_head = msg_queue_peek();
    if (queue_head == NULL)
    {
        NRF_LOG_INFO("Message queue empty!");
        return false;
    }

    printf("Dequeued message: \"%s\" (size: %u)!\r\n",
           (char*)(queue_head->buffer), queue_head->size);

    msg_queue_pop();

    return true;
}

static void uart_cb(app_uart_evt_t* event)
{
    switch(event->evt_type)
    {
    case APP_UART_DATA_READY:
        manage_received_data();
        break;
    case APP_UART_TX_EMPTY:
        NRF_LOG_INFO("printf complete!");
        break;
    case APP_UART_DATA:
        NRF_LOG_INFO("Non-FIFO data received (\?\?\?)");
        break;
    case APP_UART_FIFO_ERROR:
        NRF_LOG_INFO("Fifo error!");
        break;
    case APP_UART_COMMUNICATION_ERROR:
        NRF_LOG_INFO("Communication error!");
        break;
    default:
        NRF_LOG_INFO("Unknown type!");
        break;
    }
}
