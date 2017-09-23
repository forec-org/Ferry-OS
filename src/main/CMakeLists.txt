set(SRC_LIST main.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/config
        ${PROJECT_SOURCE_DIR}/include/os/booter
)

link_directories(
        ${LIBRARY_OUTPUT_PATH}
)

add_executable(ferry ${SRC_LIST})
target_link_libraries(ferry config ${Boost_LIBRARIES})
target_link_libraries(
        ferry
        memory
        fs
        booter
        shell
        printer
        voicer
        timer
        processor
        oscore
        interrupter
        scheduler
)
