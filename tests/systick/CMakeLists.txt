cmake_minimum_required( VERSION 3.13 )

project( systick LANGUAGES C ASM )

include( "nrf5" )

set( RESOURCES_PATH "../../resources")
set( HAL_SOURCE_PATH "${RESOURCES_PATH}/HAL" )

add_executable( ${CMAKE_PROJECT_NAME}
    "main.c"

    "${HAL_SOURCE_PATH}/board/luos_hal_board.c"
    "${HAL_SOURCE_PATH}/systick/luos_hal_systick.c"
)

add_compile_definitions(
    BSP_DEFINES_ONLY
    CONFIG_GPIO_AS_PINRESET
    DEBUG
)

nrf5_target( ${CMAKE_PROJECT_NAME} )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g" )

target_link_libraries( ${CMAKE_PROJECT_NAME} PRIVATE
    # Common
    nrf5_memobj
    nrf5_balloc
    nrf5_atomic
    nrf5_ringbuf
    nrf5_section
    nrf5_strerror
    # Drivers
    nrf5_nrfx_gpiote
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
    # BSP
    nrf5_boards
    nrf5_bsp_defs
    nrf5_sdh
)

target_include_directories( ${CMAKE_PROJECT_NAME} PRIVATE
    "${HAL_SOURCE_PATH}"
    "${HAL_SOURCE_PATH}/board"
    "${HAL_SOURCE_PATH}/systick"

    # Needed for other includes...
    "${HAL_SOURCE_PATH}/com"
    "${HAL_SOURCE_PATH}/flash"
    "${HAL_SOURCE_PATH}/ptp"
    "${HAL_SOURCE_PATH}/timer"
)
