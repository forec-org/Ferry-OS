set(SRC_LIST main.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/config
        ${PROJECT_SOURCE_DIR}/include/os/booter
)

link_directories(
        ${LIBRARY_OUTPUT_PATH}
)

add_executable(sdos ${SRC_LIST})
target_link_libraries(sdos config ${Boost_LIBRARIES})
target_link_libraries(sdos memory fs booter shell)
