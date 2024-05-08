cmake_minimum_required(VERSION 3.27)
project(SO2)

set(CMAKE_CXX_STANDARD 17)

add_executable(SO2 main.cpp)

find_package( Curses REQUIRED )
include_directories( ${CURSES_INCLUDE_DIRS} )
target_link_libraries( SO2 ${CURSES_LIBRARIES} )
