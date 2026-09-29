cmake_minimum_required(VERSION 3.16)
project(evolution_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(evolution_core
    src/world.cpp
    src/simulation.cpp
    src/statistics.cpp
)

target_include_directories(evolution_core PUBLIC include)
target_compile_options(evolution_core PRIVATE -Wall -Wextra -Wpedantic -O2)

add_executable(evolution_sim src/main.cpp)
target_link_libraries(evolution_sim PRIVATE evolution_core)

enable_testing()
add_executable(smoke_test tests/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE evolution_core)
add_test(NAME smoke_test COMMAND smoke_test)
