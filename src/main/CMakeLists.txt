set(SRC_LIST main.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/config
)

link_directories(
        ${LIBRARY_OUTPUT_PATH}
)

add_executable(sdos ${SRC_LIST})
target_link_libraries(sdos config ${Boost_LIBRARIES})
target_link_libraries(sdos memory)
