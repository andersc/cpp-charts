cmake_minimum_required(VERSION 3.31)
project(cpp_charts)

set(CMAKE_CXX_STANDARD 20)

find_package (raylib 2.0 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (Threads REQUIRED)

message("Including raylib from: " ${raylib_INCLUDE_DIRS})
message("Including ZLIB from: " ${ZLIB_INCLUDE_DIRS})

include_directories(${raylib_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})

#gauges header files location
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/charts)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Build the gauges demo
add_executable(raylib_gauges
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/gauges.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLGauge.cpp
)
target_link_libraries(raylib_gauges
        raylib
        Threads::Threads
)

# Build the bubble chart demo
add_executable(raylib_bubble
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/bubble.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBubble.cpp
)
target_link_libraries(raylib_bubble
        raylib
        Threads::Threads
)

# Build the bar chart demo
add_executable(raylib_barchart
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/barchart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBarChart.cpp
)
target_link_libraries(raylib_barchart
        raylib
        Threads::Threads
)


# Build the candlestick chart demo
add_executable(raylib_candlestick
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/candlestick.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
)
target_link_libraries(raylib_candlestick
        raylib
        Threads::Threads
)

# Build the candlestick chart demo 2 (1 value per candle)
add_executable(raylib_candlestick2
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/candlestick2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
)
target_link_libraries(raylib_candlestick2
        raylib
        Threads::Threads
)


# Build the heat map demo
add_executable(raylib_heatmap
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/heatmap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLHeatMap.cpp
)
target_link_libraries(raylib_heatmap
        raylib
        Threads::Threads
)


# Build the scatter plot demo
add_executable(raylib_scatter
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/scatterplot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLScatterPlot.cpp
)
target_link_libraries(raylib_scatter
        raylib
        Threads::Threads
)

# Build the pie chart demo
add_executable(raylib_piechart
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/piechart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLPieChart.cpp
)
target_link_libraries(raylib_piechart
        raylib
        Threads::Threads
)

# Build the main demo with all charts
add_executable(raylib_all_charts
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBarChart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBubble.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLGauge.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLHeatMap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLPieChart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLScatterPlot.cpp
)
target_link_libraries(raylib_all_charts
        raylib
        Threads::Threads
)
