cmake_minimum_required(VERSION 3.16)
project(netsim_five_layer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(netsim
  src/types.cpp
  src/checksum.cpp
  src/crc.cpp
  src/phy.cpp
  src/ethernet.cpp
  src/sliding_window.cpp
  src/arp.cpp
  src/route.cpp
  src/ipv4.cpp
  src/udp.cpp
  src/tcp.cpp
  src/dns.cpp
  src/http.cpp
  src/host.cpp
)
target_include_directories(netsim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
add_executable(netsim_tests tests/test_all.cpp)
target_link_libraries(netsim_tests PRIVATE netsim)
add_test(NAME netsim_tests COMMAND netsim_tests)

add_executable(netsim_e2e tests/e2e_http.cpp)
target_link_libraries(netsim_e2e PRIVATE netsim)
add_test(NAME netsim_e2e COMMAND netsim_e2e)
