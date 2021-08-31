cmake_minimum_required( VERSION 3.13 )

project( gate_node LANGUAGES C ASM )

include( "nrf5" )

set( RESOURCES_PATH     "../resources")
set( UTILS_PATH         "${RESOURCES_PATH}/utils" )
set( CONTAINERS_PATH    "${RESOURCES_PATH}/containers" )
set( SERVICES_PATH      "${RESOURCES_PATH}/services" )

set( LUOS_SOURCE_PATH   "${RESOURCES_PATH}/Luos" )
set( HAL_SOURCE_PATH    "${RESOURCES_PATH}/HAL" )
set( LED_SOURCE_PATH    "${CONTAINERS_PATH}/led_toggler" )
set( GATE_SOURCE_PATH   "${CONTAINERS_PATH}/gate" )
set( PTP_SERVICE_PATH   "${SERVICES_PATH}/ptp" )

set( NODE_ROLE "client" )

add_executable( ${CMAKE_PROJECT_NAME}
    "main.c"

    "${UTILS_PATH}/msg_queue/msg_queue.c"
    "${UTILS_PATH}/uart/uart_helpers.c"

    "${HAL_SOURCE_PATH}/board/luos_hal_board.c"
    "${HAL_SOURCE_PATH}/flash/luos_hal_flash.c"
    "${HAL_SOURCE_PATH}/systick/luos_hal_systick.c"
    "${HAL_SOURCE_PATH}/timer/luos_hal_timer.c"
    "${HAL_SOURCE_PATH}/ble/common/luos_hal_ble_common.c"
    "${HAL_SOURCE_PATH}/ble/${NODE_ROLE}/luos_hal_ble.c"
    "${HAL_SOURCE_PATH}/com/${NODE_ROLE}/luos_hal_com.c"
    "${HAL_SOURCE_PATH}/ptp/${NODE_ROLE}/luos_hal_ptp.c"
    "${HAL_SOURCE_PATH}/luos_hal.c"

    "${LED_SOURCE_PATH}/led_toggler.c"

    "${LUOS_SOURCE_PATH}/src/luos.c"
    "${LUOS_SOURCE_PATH}/src/luos_utils.c"
    "${LUOS_SOURCE_PATH}/src/routing_table.c"
    "${LUOS_SOURCE_PATH}/src/streaming.c"
    "${LUOS_SOURCE_PATH}/Robus/src/msg_alloc.c"
    "${LUOS_SOURCE_PATH}/Robus/src/port_manager.c"
    "${LUOS_SOURCE_PATH}/Robus/src/reception.c"
    "${LUOS_SOURCE_PATH}/Robus/src/robus.c"
    "${LUOS_SOURCE_PATH}/Robus/src/target.c"
    "${LUOS_SOURCE_PATH}/Robus/src/transmission.c"

    "${GATE_SOURCE_PATH}/cmd.c"
    "${GATE_SOURCE_PATH}/convert.c"
    "${GATE_SOURCE_PATH}/gate.c"
    "${GATE_SOURCE_PATH}/json_mnger.c"
    "${GATE_SOURCE_PATH}/cJSON/cJSON.c"

    "${PTP_SERVICE_PATH}/common/ptp_service.c"
    "${PTP_SERVICE_PATH}/${NODE_ROLE}/ptp_${NODE_ROLE}.c"
)

add_compile_definitions(
    BSP_DEFINES_ONLY
    CONFIG_GPIO_AS_PINRESET
#    DEBUG
)

nrf5_target( ${CMAKE_PROJECT_NAME} )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g" )

target_link_libraries( ${CMAKE_PROJECT_NAME} PRIVATE
    # Common
    nrf5_mdk
    nrf5_section
    nrf5_strerror
    nrf5_atomic
    nrf5_atfifo
    nrf5_atflags
    nrf5_memobj
    nrf5_balloc
    nrf5_ringbuf
    nrf5_pwr_mgmt
    # Drivers
    nrf5_nrfx_prs
    nrf5_nrfx_uarte
    nrf5_nrfx_uart
    nrf5_nrfx_systick
    nrf5_drv_uart
    # External
    nrf5_ext_fprintf
    nrf5_ext_segger_rtt
    # Logger
    nrf5_log
    nrf5_log_backend_serial
    nrf5_log_backend_rtt
    nrf5_log_default_backends
    # Application
    nrf5_app_error
    nrf5_app_util_platform
    nrf5_app_timer
    nrf5_app_fifo
    nrf5_app_uart_fifo
    # Flash storage
    nrf5_fstorage
    # BSP
    nrf5_boards
    nrf5_bsp_defs
    nrf5_sdh
    nrf5_fstorage_sd
    # BLE
    nrf5_ble_common
    nrf5_ble_scan
    nrf5_ble_gatt
    nrf5_ble_db_discovery
    # BLE services
    nrf5_ble_srv_nus_c
    nrf5_ble_srv_nus
)

target_include_directories( ${CMAKE_PROJECT_NAME} PRIVATE
    "${UTILS_PATH}/msg_queue/"
    "${UTILS_PATH}/uart/"

    "${HAL_SOURCE_PATH}/ble"
    "${HAL_SOURCE_PATH}/ble/common"
    "${HAL_SOURCE_PATH}/ble/${NODE_ROLE}"
    "${HAL_SOURCE_PATH}/board"
    "${HAL_SOURCE_PATH}/com"
    "${HAL_SOURCE_PATH}/flash"
    "${HAL_SOURCE_PATH}/ptp"
    "${HAL_SOURCE_PATH}/systick"
    "${HAL_SOURCE_PATH}/timer"
    "${HAL_SOURCE_PATH}"

    "${LED_SOURCE_PATH}"

    "${LUOS_SOURCE_PATH}/inc"
    "${LUOS_SOURCE_PATH}/OD/"
    "${LUOS_SOURCE_PATH}/Robus/inc/"

    "${GATE_SOURCE_PATH}"
    "${GATE_SOURCE_PATH}/cJSON"

    "${PTP_SERVICE_PATH}/common"
    "${PTP_SERVICE_PATH}/${NODE_ROLE}"
)
