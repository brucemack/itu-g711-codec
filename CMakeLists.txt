cmake_minimum_required(VERSION 3.13)
project(microlink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

add_executable(unit-test
  src/tests/unit-tests.cpp
  src/codec.cpp
  src/Plc.cpp
)
target_include_directories(unit-test PRIVATE src)

add_executable(demo-1
  src/tests/demo-1.cpp
  src/Plc.cpp
)
target_include_directories(demo-1 PRIVATE src)
