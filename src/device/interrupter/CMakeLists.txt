set(SRC_LIST interrupter.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/device/interrupter
        ${PROJECT_SOURCE_DIR}/include/config
)

add_library(interrupter STATIC ${SRC_LIST})