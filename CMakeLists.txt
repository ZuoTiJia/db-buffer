cmake_minimum_required(VERSION 3.21)
project(db-buffer)

set(CMAKE_CXX_STANDARD 20)
find_package(fmt)
add_executable(db-buffer main.cpp
        Replacer.h Type.h BufferPoolManager.h Page.h BPMIInstance.h  LRUReplacer.cpp LRUReplacer.h DiskManager.h TestBPM.h)
target_link_libraries(db-buffer fmt::fmt pthread)