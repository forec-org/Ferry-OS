cmake_minimum_required(VERSION 3.6)

include(CTest)
enable_testing()


# Enable the following part if GoogleTest is not configured global.
#set(REPO ${PROJECT_SOURCE_DIR}/.repo)
#include(${REPO}/DownloadProject/DownloadProject.cmake)
#download_project(
#        PROJ                googletest
#        GIT_REPOSITORY      https://github.com/google/googletest.git
#        GIT_TAG             master
#        UPDATE_DISCONNECTED 1
#)
#add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)

set(
        test_SRCS
        gtest.cpp
        test_config/test_config_parser.cpp
        ../include/config/macro.h ../include/device/memory/page_table_item.h ../include/device/memory/process_table_item.h ../include/device/memory/frame_table_item.h ../include/device/memory/page_table.h)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/config
)

link_directories(
        ${LIBRARY_OUTPUT_PATH}
)

add_executable(runUnitTests gtest.cpp ${test_SRCS})

# add all library below
target_link_libraries(runUnitTests config ${Boost_LIBRARIES})
target_link_libraries(runUnitTests memory)
target_link_libraries(runUnitTests gtest gtest_main)
target_link_libraries(runUnitTests gmock gmock_main)
#add_test(runUnitTests runUnitTests)