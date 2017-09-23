set(SRC_LIST timer.cpp)
include_directories(${PROJECT_SOURCE_DIR}/include/device/timer)

add_library(timer STATIC ${SRC_LIST})