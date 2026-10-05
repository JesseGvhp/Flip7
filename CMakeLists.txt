cmake_minimum_required(VERSION 3.20)
project(Flip7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -O3)
endif()

# Core library (add other game logic cpp files here as you build them)
add_library(deck_lib STATIC
    src/Deck.cpp
)
target_include_directories(deck_lib PUBLIC include)

# Main executable
add_executable(flip7_sim src/main.cpp)
target_link_libraries(flip7_sim PRIVATE deck_lib)

# ----------------- Unit Testing Setup -----------------
include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows/MSVC runtime compatibility (safe on Linux too)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(deck_tests tests/test_deck.cpp)
target_link_libraries(deck_tests PRIVATE deck_lib GTest::gtest_main)

# Register with CTest
include(GoogleTest)
gtest_discover_tests(deck_tests)