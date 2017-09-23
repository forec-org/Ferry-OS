set(SRC_LIST OSScheduler.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/os/oscore
        ${PROJECT_SOURCE_DIR}/include/os/scheduler
        ${PROJECT_SOURCE_DIR}/include/config
)

add_library(scheduler STATIC ${SRC_LIST})