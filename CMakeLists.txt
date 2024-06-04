cmake_minimum_required(VERSION 3.8)
project(test)
set (CMAKE_CXX_STANDARD 20)

add_executable(battletech_trainer fractional_hex.cpp hex_grid.cpp utils.cpp hex.cpp)
