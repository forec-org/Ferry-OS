set(SRC_LIST voicer.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/os/booter
        ${PROJECT_SOURCE_DIR}/include/device/voicer
)

add_library(voicer STATIC ${SRC_LIST})
