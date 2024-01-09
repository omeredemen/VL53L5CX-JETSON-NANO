cmake_minimum_required(VERSION 3.14.1)
project(myProject VERSION 1.0)
file(GLOB_RECURSE SRC_FILE src/*.cpp)

add_executable(i2c_address_change test/i2c_address_change.cpp ${SRC_FILE})
target_include_directories(i2c_address_change PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc/)

add_executable(main test/main.cpp ${SRC_FILE}) # ranging just one sensor
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc/)
