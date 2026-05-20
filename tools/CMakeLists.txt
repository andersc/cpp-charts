cmake_minimum_required(VERSION 3.28)
project(gif_capture)

set(CMAKE_CXX_STANDARD 20)

find_package(raylib 6.0 REQUIRED)
find_package(Threads REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(CHARTS_DIR "${SRC_DIR}/charts")

include_directories(${CHARTS_DIR})
include_directories(${SRC_DIR})
include_directories(${raylib_INCLUDE_DIRS})

add_executable(gif_capture
    gif_capture.cpp
    ${CHARTS_DIR}/RLAreaChart.cpp
    ${CHARTS_DIR}/RLLinearGauge.cpp
    ${CHARTS_DIR}/RLRadarChart.cpp
    ${CHARTS_DIR}/RLSankey.cpp
)

target_link_libraries(gif_capture
    raylib
    Threads::Threads
)

# Copy font file to build directory
set(FONT_SOURCE "${SRC_DIR}/examples/base.ttf")
file(COPY ${FONT_SOURCE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
