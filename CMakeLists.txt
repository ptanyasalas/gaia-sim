cmake_minimum_required(VERSION 3.16)
project(evolution_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_library(evolution_core
    src/world.cpp
    src/simulation.cpp
    src/statistics.cpp
)

target_include_directories(evolution_core PUBLIC include)
target_compile_options(evolution_core PRIVATE -Wall -Wextra -Wpedantic -O2)

add_executable(evolution_sim src/main.cpp)
target_link_libraries(evolution_sim PRIVATE evolution_core)
set_target_properties(evolution_sim PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

enable_testing()
add_executable(smoke_test tests/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE evolution_core)
set_target_properties(smoke_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_test(NAME smoke_test COMMAND smoke_test)
