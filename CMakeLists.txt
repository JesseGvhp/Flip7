cmake_minimum_required(VERSION 3.20)
project(Flip7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -O3)
endif()

# Core library (renamed to flip7_core to reflect all game logic)
add_library(flip7_core STATIC
    src/Deck.cpp
    src/GameState.cpp
)
target_include_directories(flip7_core PUBLIC include)

# Main executable
add_executable(flip7_sim src/main.cpp)
target_link_libraries(flip7_sim PRIVATE flip7_core)

# ----------------- Unit Testing Setup -----------------
include(CTest)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # For Windows/MSVC runtime compatibility
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)

    # Deck tests
    add_executable(deck_tests tests/test_deck.cpp)
    target_link_libraries(deck_tests PRIVATE flip7_core GTest::gtest_main)
    gtest_discover_tests(deck_tests)

    # GameState tests
    add_executable(game_state_tests tests/test_game_state.cpp)
    target_link_libraries(game_state_tests PRIVATE flip7_core GTest::gtest_main)
    gtest_discover_tests(game_state_tests)
endif()