cmake_minimum_required( VERSION 3.5 )

project(eos.test VERSION 1.0.0)

include(eosio.cmake)

#configure_file(${CMAKE_SOURCE_DIR}/contracts.hpp.in ${CMAKE_BINARY_DIR}/contracts.hpp)

include_directories(${CMAKE_BINARY_DIR})

file(GLOB UNIT_TESTS "*.cpp" "*.hpp")

add_eosio_test(eos.test ${UNIT_TESTS})