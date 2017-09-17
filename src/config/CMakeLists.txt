set(SRC_LIST config.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/config
)
add_library(config STATIC ${SRC_LIST})
