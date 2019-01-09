cmake_minimum_required( VERSION 3.5 )

project(eosfactory.test VERSION 1.0.0)

include_directories(${CMAKE_BINARY_DIR})

file(GLOB UNIT_TESTS "*.py")

message("tests: ${UNIT_TESTS}")

add_test(NAME eosfactory.test.1 COMMAND python3 ${UNIT_TESTS})