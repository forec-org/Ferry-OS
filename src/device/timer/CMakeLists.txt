set(SRC_LIST timer.cpp)
include_directories(
        ${PROJECT_SOURCE_DIR}/include/device/timer
        ${PROJECT_SOURCE_DIR}/include/device/interrupter
        ${PROJECT_SOURCE_DIR}/include/config
)

add_library(timer STATIC ${SRC_LIST})