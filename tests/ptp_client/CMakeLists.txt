cmake_minimum_required( VERSION 3.13 )

project( ptp_client LANGUAGES C ASM )

include( "nrf5" )

set( RESOURCES_PATH "../../resources")
set( SERVICES_PATH "${RESOURCES_PATH}/services" )

set( HAL_SOURCE_PATH "${RESOURCES_PATH}/HAL" )
set( PTP_SERVICE_PATH "${SERVICES_PATH}/ptp" )

set( NODE_ROLE "client" )

add_executable( ${CMAKE_PROJECT_NAME}
    "main.c"

    "${HAL_SOURCE_PATH}/ble/common/luos_hal_ble_common.c"
    "${HAL_SOURCE_PATH}/ble/${NODE_ROLE}/luos_hal_ble.c"
    "${HAL_SOURCE_PATH}/board/luos_hal_board.c"

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
    nrf5_nrfx_gpiote
    nrf5_nrfx_prs
    nrf5_nrfx_systick
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
    nrf5_app_button
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
    # BLE Services
    nrf5_ble_srv_nus_c
)

target_include_directories( ${CMAKE_PROJECT_NAME} PRIVATE
    "${HAL_SOURCE_PATH}/ble"
    "${HAL_SOURCE_PATH}/ble/common"
    "${HAL_SOURCE_PATH}/ble/${NODE_ROLE}"
    "${HAL_SOURCE_PATH}/board"

    "${PTP_SERVICE_PATH}/common"
    "${PTP_SERVICE_PATH}/${NODE_ROLE}"
)
