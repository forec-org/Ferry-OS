cmake_minimum_required(VERSION 3.6)
project(SDOS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        main.cpp
        device/fs/fs.h
        device/memory/memory.h
        device/printer/printer.h
        device/processor/processor.h
        device/timer/timer.h
        os/interpreter/interpreter.h
        os/monitor/monitor.h
        os/scheduler/scheduler.h
        os/oscore/oscore.h)

add_executable(sdos ${SOURCE_FILES})