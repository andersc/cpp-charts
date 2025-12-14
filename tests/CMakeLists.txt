# tests/CMakeLists.txt
# Unit tests for cpp-charts using doctest

include(FetchContent)

# Fetch doctest (single header, lightweight test framework)
# Using master branch for CMake 3.28+ compatibility
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG master
)
FetchContent_MakeAvailable(doctest)

# Collect all chart source files
set(CHART_SOURCES
    ${CMAKE_SOURCE_DIR}/src/charts/RLBarChart.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLBubble.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLGauge.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLHeatMap.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLHeatMap3D.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLLogPlot.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLOrderBookVis.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLPieChart.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLScatterPlot.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLTimeSeries.cpp
    ${CMAKE_SOURCE_DIR}/src/charts/RLTreeMap.cpp
)

# Test executable
add_executable(cpp_charts_tests
    test_main.cpp
    test_common.cpp
    test_charts.cpp
    ${CHART_SOURCES}
)

target_include_directories(cpp_charts_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/charts
)

target_link_libraries(cpp_charts_tests PRIVATE
    doctest::doctest
    raylib
    Threads::Threads
)

if(WIN32)
    target_link_libraries(cpp_charts_tests PRIVATE winmm)
endif()

# Register with CTest
add_test(NAME cpp_charts_tests COMMAND cpp_charts_tests)

