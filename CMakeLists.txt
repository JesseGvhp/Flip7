cmake_minimum_required(VERSION 3.20)
project(Flip7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for Monte Carlo workloads
add_compile_options(-Wall -Wextra -O3)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")
add_executable(flip7_sim ${SOURCES})

# Enable OpenMP for trivial multi-threaded trial execution
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(flip7_sim PRIVATE OpenMP::OpenMP_CXX)
endif()