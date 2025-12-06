cmake_minimum_required(VERSION 3.28)
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
if(WIN32)
    target_link_libraries(raylib_gauges winmm)
endif()

# Build the bubble chart demo
add_executable(raylib_bubble
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/bubble.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBubble.cpp
)
target_link_libraries(raylib_bubble
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_bubble winmm)
endif()

# Build the bar chart demo
add_executable(raylib_barchart
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/barchart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBarChart.cpp
)
target_link_libraries(raylib_barchart
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_barchart winmm)
endif()


# Build the candlestick chart demo
add_executable(raylib_candlestick
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/candlestick.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
)
target_link_libraries(raylib_candlestick
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_candlestick winmm)
endif()

# Build the candlestick chart demo 2 (1 value per candle)
add_executable(raylib_candlestick2
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/candlestick2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
)
target_link_libraries(raylib_candlestick2
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_candlestick2 winmm)
endif()


# Build the heat map demo
add_executable(raylib_heatmap
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/heatmap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLHeatMap.cpp
)
target_link_libraries(raylib_heatmap
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_heatmap winmm)
endif()


# Build the scatter plot demo
add_executable(raylib_scatter
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/scatterplot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLScatterPlot.cpp
)
target_link_libraries(raylib_scatter
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_scatter winmm)
endif()

# Build the pie chart demo
add_executable(raylib_piechart
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/piechart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLPieChart.cpp
)
target_link_libraries(raylib_piechart
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_piechart winmm)
endif()

# Build the log-log plot demo (time series + Allan variance analysis)
add_executable(raylib_logplot
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/logplot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLLogPlot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLTimeSeries.cpp
)
target_link_libraries(raylib_logplot
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_logplot winmm)
endif()

# Build the time series streaming demo
add_executable(raylib_timeseries
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/timeseries.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLTimeSeries.cpp
)
target_link_libraries(raylib_timeseries
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_timeseries winmm)
endif()

# Build the order book visualization demo
add_executable(raylib_orderbook
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/orderbook.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLOrderBookVis.cpp
)
target_link_libraries(raylib_orderbook
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_orderbook winmm)
endif()

# Build the tree map visualization demo
add_executable(raylib_treemap
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/treemap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLTreeMap.cpp
)
target_link_libraries(raylib_treemap
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_treemap winmm)
endif()

# Build the main demo with all charts
add_executable(raylib_all_charts
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBarChart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLBubble.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLGauge.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLHeatMap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLOrderBookVis.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLPieChart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLScatterPlot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLLogPlot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLTimeSeries.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLTreeMap.cpp
)
target_link_libraries(raylib_all_charts
        raylib
        Threads::Threads
)
if(WIN32)
    target_link_libraries(raylib_all_charts winmm)
endif()
