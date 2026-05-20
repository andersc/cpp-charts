cmake_minimum_required(VERSION 3.28)
project(cpp_charts_headless)

set(CMAKE_CXX_STANDARD 20)

# Build raylib with Memory platform (headless, no GPU/display)
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
    GIT_SHALLOW TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(PLATFORM "Memory" CACHE STRING "" FORCE)
FetchContent_MakeAvailable(raylib)
message(STATUS "raylib 6.0 Memory platform (headless)")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/charts)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Threads REQUIRED)

add_executable(headless_charts
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/examples/headless.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/charts/RLBarChart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/charts/RLPieChart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/charts/RLTimeSeries.cpp
)
target_link_libraries(headless_charts
    raylib
    Threads::Threads
)
if(WIN32)
    target_link_libraries(headless_charts winmm)
endif()
