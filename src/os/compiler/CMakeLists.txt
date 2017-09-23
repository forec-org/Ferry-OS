set(SRC_LIST compiler.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/os/compiler
        ${PROJECT_SOURCE_DIR}/include/device/fs
        ${PROJECT_SOURCE_DIR}/include/config
)

add_library(compiler STATIC ${SRC_LIST})