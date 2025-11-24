cmake_minimum_required(VERSION 3.13)
project(microlink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
file(MAKE_DIRECTORY tmp)

add_executable(unit-test
  src/tests/unit-tests.cpp
  src/codec.cpp
)

target_include_directories(unit-test PRIVATE src)
