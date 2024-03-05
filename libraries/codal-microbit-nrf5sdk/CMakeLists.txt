project(codal-microbit-nrf5sdk)

# Set SUPPORT_NRF_LOG to True to compile files needed only for NRF_LOG_XXX
set( SUPPORT_NRF_LOG True)

# codal-nrf52 includes

list(APPEND INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/../codal-nrf52/nrfx/hal"
)


# nRF5SDK_mods includes

list(APPEND INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk_config"
    "${CMAKE_CURRENT_SOURCE_DIR}/nRF5SDK_mods"
)


set(SDK_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
if(REGENERATE)
set(SDK_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../")
endif()


# nRF5SDK includes

list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/pwr_mgmt"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/sortlist"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/strerror"
    "${SDK_DIRECTORY}/nRF5SDK/components/softdevice/common"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/crc32"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/bootloader/dfu"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/util"
    "${SDK_DIRECTORY}/nRF5SDK/components/ble/common"
)


if (SUPPORT_NRF_LOG)
list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/balloc"
)
endif()


list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager"
)


if (SUPPORT_NRF_LOG)
list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/ringbuf"
)
endif()


list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/timer"
)

list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/log"
    "${SDK_DIRECTORY}/nRF5SDK/components/ble/nrf_ble_gatt"
)

list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/bootloader"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/fstorage"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/experimental_section_vars"
    "${SDK_DIRECTORY}/nRF5SDK/components/softdevice/s113/headers"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/mutex"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/delay"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/bootloader/ble_dfu"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/atomic_fifo"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/atomic"
)


if (SUPPORT_NRF_LOG)
list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/memobj"
)
endif()


list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/fds"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/atomic_flags"
    "${SDK_DIRECTORY}/nRF5SDK/components/softdevice/s113/headers/nrf52"
    "${SDK_DIRECTORY}/nRF5SDK/components/ble/ble_services/ble_dfu"
)


if (SUPPORT_NRF_LOG)
list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/external/fprintf"
)
endif()


list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/svc"
    "${SDK_DIRECTORY}/nRF5SDK/components/libraries/log/src"
)

list(APPEND INCLUDE_SDK_DIRS
    "${SDK_DIRECTORY}/nRF5SDK/components/ble/ble_services/ble_dis"
)


if (SUPPORT_NRF_LOG)
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/nRF5SDK_mods/nrf_log_backend_dmesg.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/log/src/nrf_log_backend_serial.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/log/src/nrf_log_frontend.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/log/src/nrf_log_str_formatter.c")
endif()


list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/util/app_error.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/util/app_error_handler_gcc.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/util/app_error_weak.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/util/app_util_platform.c")
#list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/util/nrf_assert.c")

#list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/scheduler/app_scheduler.c")

list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/nRF5SDK_mods/app_timer2.c")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/nRF5SDK_mods/drv_rtc.c")

#list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/crc16/crc16.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/crc32/crc32.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/fds/fds.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/atomic_fifo/nrf_atfifo.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/atomic_flags/nrf_atflags.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/atomic/nrf_atomic.c")


if (SUPPORT_NRF_LOG)
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/balloc/nrf_balloc.c")
endif()


if (SUPPORT_NRF_LOG)
# needed for NRF_LOG_XXX
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/external/fprintf/nrf_fprintf.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/external/fprintf/nrf_fprintf_format.c")
endif()


list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/fstorage/nrf_fstorage.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/fstorage/nrf_fstorage_sd.c")


if (SUPPORT_NRF_LOG)
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/memobj/nrf_memobj.c")
endif()


list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/nRF5SDK_mods/nrf_pwr_mgmt.c")


if (SUPPORT_NRF_LOG)
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/ringbuf/nrf_ringbuf.c")
endif()


list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/experimental_section_vars/nrf_section_iter.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/sortlist/nrf_sortlist.c")


if (SUPPORT_NRF_LOG)
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/strerror/nrf_strerror.c")
endif()


list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/libraries/bootloader/dfu/nrf_dfu_svci.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/common/ble_advdata.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/common/ble_conn_params.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/common/ble_conn_state.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/common/ble_srv_common.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/nrf_ble_gatt/nrf_ble_gatt.c")

#list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/nrf_ble_qwr/nrf_ble_qwr.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/gatt_cache_manager.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/gatts_cache_manager.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/id_manager.c")
#list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/auth_status_tracker.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/peer_data_storage.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/peer_database.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/peer_id.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/peer_manager.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/peer_manager_handler.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/pm_buffer.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/security_dispatcher.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/peer_manager/security_manager.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/softdevice/common/nrf_sdh.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/softdevice/common/nrf_sdh_ble.c")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/nRF5SDK_mods/nrf_sdh_soc.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/ble_services/ble_dfu/ble_dfu.c")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/nRF5SDK_mods/ble_dfu_bonded.c")
list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/ble_services/ble_dfu/ble_dfu_unbonded.c")

list(APPEND SOURCE_FILES "${SDK_DIRECTORY}/nRF5SDK/components/ble/ble_services/ble_dis/ble_dis.c")


if( REGENERATE )
message("Regenerating nRF5SDK folder")

set(DIRECTORY "./nRF5SDK")
file(MAKE_DIRECTORY "${DIRECTORY}")

# copy source files
foreach ( file ${SOURCE_FILES} )
    get_filename_component( dir ${file} DIRECTORY )
    string(REPLACE "../" "" copy_dir ${dir})
    file(INSTALL ${file} DESTINATION ${copy_dir} )
endforeach()

# copy header files
foreach ( header_dir ${INCLUDE_SDK_DIRS} )
    string(REPLACE "../" "" copy_dir ${header_dir})
    get_filename_component( dir ${copy_dir} DIRECTORY )
    file(INSTALL ${header_dir} DESTINATION ${dir})
endforeach()

endif()


# add them
include_directories(${INCLUDE_DIRS})
include_directories(${INCLUDE_SDK_DIRS})

# create our target
add_library(codal-microbit-nrf5sdk ${SOURCE_FILES})

target_link_libraries(
    codal-microbit-nrf5sdk
    codal-nrf52
    codal-core
)

# expose it to parent cmake.
target_include_directories(codal-microbit-nrf5sdk PUBLIC ${INCLUDE_DIRS})
target_include_directories(codal-microbit-nrf5sdk PUBLIC ${INCLUDE_SDK_DIRS})
