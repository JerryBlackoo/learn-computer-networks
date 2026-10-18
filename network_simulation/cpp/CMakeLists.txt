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
  src/csma_cd.cpp
  src/csma_ca.cpp
  src/token_ring.cpp
  src/ethernet.cpp
  src/mac_switch.cpp
  src/flow_table.cpp
  src/sdn_switch.cpp
  src/sdn_controller.cpp
  src/sliding_window.cpp
  src/arp.cpp
  src/route.cpp
  src/routing_algo.cpp
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

add_executable(netsim_sdn_tests tests/test_sdn.cpp)
target_link_libraries(netsim_sdn_tests PRIVATE netsim)
add_test(NAME netsim_sdn_tests COMMAND netsim_sdn_tests)

add_executable(netsim_sdn_demo src/main_sdn_demo.cpp)
target_link_libraries(netsim_sdn_demo PRIVATE netsim)
add_test(NAME netsim_sdn_e2e COMMAND netsim_sdn_demo)

# Optional layer walkthrough demo (not required for ctest).
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main_demo.cpp")
  add_executable(netsim_demo src/main_demo.cpp)
  target_link_libraries(netsim_demo PRIVATE netsim)
endif()
