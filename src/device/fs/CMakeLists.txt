set(
        SRC_LIST
        handler/handler.cpp
)
include_directories(
        ${PROJECT_SOURCE_DIR}/include/device/fs
        ${PROJECT_SOURCE_DIR}/include/device/fs/handler
)

add_library(fs STATIC ${SRC_LIST})