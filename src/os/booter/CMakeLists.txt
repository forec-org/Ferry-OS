set(
        SRC_LIST
        booter.cpp
        ../shell/shell.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/os/booter
        ${PROJECT_SOURCE_DIR}/include/config
        ${PROJECT_SOURCE_DIR}/include/device/fs
        ${PROJECT_SOURCE_DIR}/include/device/memory
        ${PROJECT_SOURCE_DIR}/include/os/shell
)

add_library(booter STATIC ${SRC_LIST})