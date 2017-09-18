set(SRC_LIST config.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/config
        ${PROJECT_SOURCE_DIR}/include/device/memory
)
add_library(config STATIC ${SRC_LIST})
