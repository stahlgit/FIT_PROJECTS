cmake_minimum_required(VERSION 3.10)
project(isa-top)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PCAP REQUIRED libpcap)
pkg_check_modules(NCURSES REQUIRED ncurses)

include_directories(${PCAP_INCLUDE_DIRS} ${NCURSES_INCLUDE_DIRS} include/)
link_directories(${PCAP_LIBRARY_DIRS} ${NCURSES_LIBRARY_DIRS})

add_executable(isa-top src/main.cpp src/packetcapture.cpp src/stats.cpp src/display.cpp src/utils.cpp)

target_link_libraries(isa-top ${PCAP_LIBRARIES} ${NCURSES_LIBRARIES})
