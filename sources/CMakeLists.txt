cmake_minimum_required(VERSION 3.5)

project(my_millions VERSION 1.0.0)

include(eosio.cdt.cmake)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   set(TEST_BUILD_TYPE "Debug")
   set(CMAKE_BUILD_TYPE "Release")
else()
   set(TEST_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()

set(OUTPUT_ROOT ${CMAKE_BINARY_DIR}/output)
set(SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()

add_subdirectory(mymillions)
add_subdirectory(tests/eosfactory)

if (APPLE)
   set(OPENSSL_ROOT "/usr/local/opt/openssl")
elseif (UNIX)
   set(OPENSSL_ROOT "/usr/include/openssl")
endif()
set(SECP256K1_ROOT "/usr/local")

# just a stub
#include(UnitTestsExternalProject.cmake)