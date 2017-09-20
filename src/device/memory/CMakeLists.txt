set(
        SRC_LIST
        mmu.cpp
        process_table_item.cpp
        page_table_item.cpp
        frame_table_item.cpp
        page_table.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/config
        ${PROJECT_SOURCE_DIR}/include/device/memory
)

add_library(memory STATIC ${SRC_LIST})
