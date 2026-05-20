cmake_minimum_required(VERSION 3.28)
project(single_guage)

set(CMAKE_CXX_STANDARD 20)

find_package(raylib 6.0 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
        GIT_SHALLOW TRUE
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)
else()
    include_directories(${raylib_INCLUDE_DIRS})
endif()

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
        cpp_charts
        GIT_REPOSITORY https://github.com/andersc/cpp-charts.git
        GIT_TAG main  # specify a version or branch if needed
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
)
FetchContent_MakeAvailable(cpp_charts)

message("Including cpp.charts from: " ${cpp_charts_SOURCE_DIR}/src/charts)
message("and RLCommon.h from: " ${cpp_charts_SOURCE_DIR}/src)
include_directories(${cpp_charts_SOURCE_DIR}/src/charts)
include_directories(${cpp_charts_SOURCE_DIR}/src)

add_executable(single_guage
        ${CMAKE_CURRENT_SOURCE_DIR}/single_guage.cpp
        ${cpp_charts_SOURCE_DIR}/src/charts/RLGauge.cpp
)
target_link_libraries(single_guage
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(single_guage winmm)
endif()


