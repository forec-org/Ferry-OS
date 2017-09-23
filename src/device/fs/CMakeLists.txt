set(
        SRC_LIST
        fs.cpp
)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/device/fs
        ${PROJECT_SOURCE_DIR}/include/config
        ${PROJECT_SOURCE_DIR}/include/device/memory
)

add_library(fs STATIC ${SRC_LIST})