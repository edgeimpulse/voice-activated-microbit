project(codal-nrf52)

string(FIND "${device.processor}" "NRF52840" pos40)
string(FIND "${device.processor}" "NRF52833" pos33)
if (NOT "${pos40}" STREQUAL "-1")
    set(TEMPLATE "nRF52840")
    set(DEV "nrf52840")
else()
if (NOT "${pos33}" STREQUAL "-1")
    set(TEMPLATE "nRF52833")
    set(DEV "nrf52833")
else()
    set(TEMPLATE "nRF52832")
    set(DEV "nrf52")
endif()
endif()

include("${CODAL_UTILS_LOCATION}")

# find sources and headers
RECURSIVE_FIND_FILE(SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source" "*.c??")

list(APPEND INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/cmsis"
    "${CMAKE_CURRENT_SOURCE_DIR}/nrfx/mdk"
    "${CMAKE_CURRENT_SOURCE_DIR}/nrfx"
    "${CMAKE_CURRENT_SOURCE_DIR}/nrfx/templates/"
    "${CMAKE_CURRENT_SOURCE_DIR}/nrfx/templates/${TEMPLATE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/nrfx/drivers/include"
)

list(APPEND SOURCE_FILES "asm/CortexContextSwitch.s")
list(APPEND SOURCE_FILES "nrfx/mdk/system_${DEV}.c")
list(APPEND SOURCE_FILES "nrfx/mdk/gcc_startup_${DEV}.S")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNRFX_PRS_ENABLED=0")

# add them
include_directories(${INCLUDE_DIRS})

# create our target
add_library(codal-nrf52 ${SOURCE_FILES})

target_link_libraries(
    codal-nrf52
    codal-core
)

# expose it to parent cmake.
target_include_directories(codal-nrf52 PUBLIC ${INCLUDE_DIRS})
