cmake_minimum_required(VERSION 3.20)
project(cpp_charts_wasm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure we're building with Emscripten
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This CMakeLists.txt is designed for Emscripten builds only. Use emcmake cmake ..")
endif()

# Fetch raylib for WebAssembly
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

# Configure raylib for web platform
set(PLATFORM Web CACHE STRING "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

# Source directories (relative to wasm folder, go up one level)
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(EXAMPLES_DIR "${SRC_DIR}/examples")
set(CHARTS_DIR "${SRC_DIR}/charts")
set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Include directories
include_directories(${CHARTS_DIR})
include_directories(${SRC_DIR})

# Common Emscripten flags
set(COMMON_LINK_FLAGS
    "-sUSE_GLFW=3"
    "-sASYNCIFY"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sTOTAL_MEMORY=67108864"
    "-sGL_ENABLE_GET_PROC_ADDRESS"
    "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
)

# Convert list to string
list(JOIN COMMON_LINK_FLAGS " " COMMON_LINK_FLAGS_STR)

# Copy assets to build directory
configure_file(${EXAMPLES_DIR}/base.ttf ${CMAKE_CURRENT_BINARY_DIR}/base.ttf COPYONLY)
# Use smaller sample CSV for WASM demos (web-friendly size)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/JPM_sample.csv ${CMAKE_CURRENT_BINARY_DIR}/JPM_1_minute_bars.csv COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/index.html ${CMAKE_CURRENT_BINARY_DIR}/index.html COPYONLY)

# Preload flags for assets
set(PRELOAD_FONT "--preload-file ${CMAKE_CURRENT_BINARY_DIR}/base.ttf@base.ttf")
set(PRELOAD_CSV "--preload-file ${CMAKE_CURRENT_BINARY_DIR}/JPM_1_minute_bars.csv@JPM_1_minute_bars.csv")

# Macro to create a WASM demo target
macro(add_wasm_demo TARGET_NAME SOURCE_FILE CHART_FILES EXTRA_PRELOAD)
    add_executable(${TARGET_NAME}
        ${EXAMPLES_DIR}/${SOURCE_FILE}
        ${CHART_FILES}
    )
    target_link_libraries(${TARGET_NAME} raylib)

    # Set link flags with preloaded assets
    set(TARGET_LINK_FLAGS "${COMMON_LINK_FLAGS_STR} ${PRELOAD_FONT}")
    if(NOT "${EXTRA_PRELOAD}" STREQUAL "")
        set(TARGET_LINK_FLAGS "${TARGET_LINK_FLAGS} ${EXTRA_PRELOAD}")
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "${TARGET_LINK_FLAGS}"
    )
endmacro()

# Build all demo targets

# Gauges demo
add_wasm_demo(gauges
    "gauges.cpp"
    "${CHARTS_DIR}/RLGauge.cpp"
    ""
)

# Bubble chart demo
add_wasm_demo(bubble
    "bubble.cpp"
    "${CHARTS_DIR}/RLBubble.cpp"
    ""
)

# Bar chart demo
add_wasm_demo(barchart
    "barchart.cpp"
    "${CHARTS_DIR}/RLBarChart.cpp"
    ""
)

# Candlestick chart demo (needs CSV file)
add_wasm_demo(candlestick
    "candlestick.cpp"
    "${CHARTS_DIR}/RLCandlestickChart.cpp"
    "${PRELOAD_CSV}"
)

# Candlestick chart demo 2 (also needs CSV file)
add_wasm_demo(candlestick2
    "candlestick2.cpp"
    "${CHARTS_DIR}/RLCandlestickChart.cpp"
    "${PRELOAD_CSV}"
)

# Heat map demo
add_wasm_demo(heatmap
    "heatmap.cpp"
    "${CHARTS_DIR}/RLHeatMap.cpp"
    ""
)

# Scatter plot demo
add_wasm_demo(scatter
    "scatterplot.cpp"
    "${CHARTS_DIR}/RLScatterPlot.cpp"
    ""
)

# Pie chart demo
add_wasm_demo(piechart
    "piechart.cpp"
    "${CHARTS_DIR}/RLPieChart.cpp"
    ""
)

# Log-log plot demo
add_wasm_demo(logplot
    "logplot.cpp"
    "${CHARTS_DIR}/RLLogPlot.cpp;${CHARTS_DIR}/RLTimeSeries.cpp"
    ""
)

# Time series demo
add_wasm_demo(timeseries
    "timeseries.cpp"
    "${CHARTS_DIR}/RLTimeSeries.cpp"
    ""
)

# Area chart demo
add_wasm_demo(areachart
    "areachart.cpp"
    "${CHARTS_DIR}/RLAreaChart.cpp"
    ""
)

# Order book demo
add_wasm_demo(orderbook
    "orderbook.cpp"
    "${CHARTS_DIR}/RLOrderBookVis.cpp"
    ""
)

# Tree map demo
add_wasm_demo(treemap
    "treemap.cpp"
    "${CHARTS_DIR}/RLTreeMap.cpp"
    ""
)

# 3D Heat map demo
add_wasm_demo(heatmap3d
    "heatmap3d.cpp"
    "${CHARTS_DIR}/RLHeatMap3D.cpp"
    ""
)

# Radar chart demo
add_wasm_demo(radar
    "radar.cpp"
    "${CHARTS_DIR}/RLRadarChart.cpp"
    ""
)

# Sankey diagram demo
add_wasm_demo(sankey
    "sankey.cpp"
    "${CHARTS_DIR}/RLSankey.cpp"
    ""
)

message(STATUS "")
message(STATUS "=== cpp-charts WebAssembly Build ===")
message(STATUS "Demos to build: gauges, bubble, barchart, candlestick, candlestick2,")
message(STATUS "                heatmap, scatter, piechart, logplot, timeseries,")
message(STATUS "                areachart, orderbook, treemap, heatmap3d, radar, sankey")
message(STATUS "")
message(STATUS "After build, serve the build directory with a web server:")
message(STATUS "  cd build && python3 -m http.server 8080")
message(STATUS "Then open http://localhost:8080 in your browser")
message(STATUS "")

