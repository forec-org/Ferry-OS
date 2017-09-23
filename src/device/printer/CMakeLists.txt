set(SRC_LIST printer.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/os/booter
        ${PROJECT_SOURCE_DIR}/include/device/printer
        ${PROJECT_SOURCE_DIR}/include/device/fs
)

add_library(printer STATIC ${SRC_LIST})
