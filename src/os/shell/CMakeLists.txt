set(SRC_LIST shell.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/os/shell
        ${PROJECT_SOURCE_DIR}/include/config
        ${PROJECT_SOURCE_DIR}/include/device/memory
        ${PROJECT_SOURCE_DIR}/include/device/fs
        ${PROJECT_SOURCE_DIR}/include/os/booter
)

add_library(shell STATIC ${SRC_LIST})