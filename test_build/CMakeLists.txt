cmake_minimum_required(VERSION 3.28)
project(single_guage)

set(CMAKE_CXX_STANDARD 20)

find_package (raylib 5.0 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (Threads REQUIRED)

message("Including raylib from: " ${raylib_INCLUDE_DIRS})
message("Including ZLIB from: " ${ZLIB_INCLUDE_DIRS})

include_directories(${raylib_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})

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
    target_link_libraries(raylib_bubble winmm)
endif()


