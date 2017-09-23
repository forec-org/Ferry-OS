set(SRC_LIST processor.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/device/processor
        ${PROJECT_SOURCE_DIR}/include/os/oscore
        ${PROJECT_SOURCE_DIR}/include/device/memory
        ${PROJECT_SOURCE_DIR}/include/device/interrupter
)

add_library(processor STATIC ${SRC_LIST})