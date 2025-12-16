// test_charts.cpp
// Unit tests for chart classes (logic only, no draw() calls)

// IMPORTANT: On Windows, we need to prevent Windows SDK conflicts with raylib
#if defined(_WIN32)
    #define NOGDI             // Excludes GDI (Rectangle, etc.)
    #define NOUSER            // Excludes USER (CloseWindow, ShowCursor, etc.)
#endif

#include "RLAreaChart.h"
#include "RLBarChart.h"
#include "RLBubble.h"
#include "RLCandlestickChart.h"
#include "RLGauge.h"
#include "RLHeatMap.h"
#include "RLHeatMap3D.h"
#include "RLLogPlot.h"
#include "RLOrderBookVis.h"
#include "RLPieChart.h"
#include "RLRadarChart.h"
#include "RLSankey.h"
#include "RLScatterPlot.h"
#include "RLTimeSeries.h"
#include "RLTreeMap.h"

#include "doctest/doctest.h"

// Global flag from test_main.cpp indicating raylib availability
extern bool gRaylibAvailable;

// Helper macro to skip tests when raylib isn't available
#define REQUIRE_RAYLIB() do { if (!gRaylibAvailable) { MESSAGE("Skipping: no raylib context"); return; } } while(0)

// Test bounds used across tests
const Rectangle TEST_BOUNDS = {0, 0, 400, 300};

TEST_SUITE("Chart Instantiation") {

    TEST_CASE("All charts can be instantiated without conflicts") {
        REQUIRE_RAYLIB();

        // Verify no static initialization conflicts between chart types
        RLAreaChart lAreaChart(TEST_BOUNDS, RLAreaChartMode::STACKED);
        RLGauge lGauge(TEST_BOUNDS, 0.0f, 100.0f);
        RLBarChart lBarChart(TEST_BOUNDS, RLBarOrientation::VERTICAL);
        RLPieChart lPieChart(TEST_BOUNDS);
        RLRadarChart lRadarChart(TEST_BOUNDS);
        RLSankey lSankey(TEST_BOUNDS);
        RLScatterPlot lScatter(TEST_BOUNDS);
        RLTimeSeries lTimeSeries(TEST_BOUNDS, 100);
        RLHeatMap lHeatMap(TEST_BOUNDS, 32, 32);
        RLHeatMap3D lHeatMap3D(32, 32);
        RLCandlestickChart lCandle(TEST_BOUNDS, 5, 20);
        RLLogPlot lLogPlot(TEST_BOUNDS);
        RLTreeMap lTreeMap(TEST_BOUNDS);
        RLOrderBookVis lOrderBook(TEST_BOUNDS, 100, 10);
        RLBubble lBubble(TEST_BOUNDS);

        // Basic sanity checks
        CHECK(lAreaChart.getBounds().width == doctest::Approx(400.0f));
        CHECK(lGauge.getValue() == doctest::Approx(0.0f));
        CHECK(lBarChart.getBounds().width == doctest::Approx(400.0f));
        CHECK(lPieChart.getBounds().height == doctest::Approx(300.0f));
        CHECK(lRadarChart.getBounds().width == doctest::Approx(400.0f));
        CHECK(lSankey.getBounds().width == doctest::Approx(400.0f));
    }

}

TEST_SUITE("RLGauge") {

    TEST_CASE("Value clamping") {
        REQUIRE_RAYLIB();

        RLGauge lGauge(TEST_BOUNDS, 0.0f, 100.0f);

        SUBCASE("setValue clamps to range") {
            lGauge.setValue(50.0f);
            CHECK(lGauge.getValue() == doctest::Approx(50.0f));

            lGauge.setValue(-10.0f);
            CHECK(lGauge.getValue() == doctest::Approx(0.0f));

            lGauge.setValue(150.0f);
            CHECK(lGauge.getValue() == doctest::Approx(100.0f));
        }

        SUBCASE("setTargetValue clamps to range") {
            lGauge.setTargetValue(200.0f);
            CHECK(lGauge.getTarget() == doctest::Approx(100.0f));

            lGauge.setTargetValue(-50.0f);
            CHECK(lGauge.getTarget() == doctest::Approx(0.0f));
        }
    }

    TEST_CASE("Range changes") {
        REQUIRE_RAYLIB();

        RLGauge lGauge(TEST_BOUNDS, 0.0f, 100.0f);
        lGauge.setValue(50.0f);

        // Change range - value should be clamped if outside new range
        lGauge.setRange(0.0f, 40.0f);
        CHECK(lGauge.getValue() == doctest::Approx(40.0f));

        lGauge.setRange(60.0f, 100.0f);
        CHECK(lGauge.getValue() == doctest::Approx(60.0f));
    }

    TEST_CASE("Animation convergence") {
        REQUIRE_RAYLIB();

        RLGauge lGauge(TEST_BOUNDS, 0.0f, 100.0f);
        lGauge.setValue(0.0f);
        lGauge.setTargetValue(100.0f);

        // Simulate multiple update steps
        for (int i = 0; i < 100; i++) {
            lGauge.update(0.016f); // ~60fps
        }

        // Value should have converged close to target
        CHECK(lGauge.getValue() == doctest::Approx(100.0f).epsilon(0.01));
    }

    TEST_CASE("Bounds update") {
        REQUIRE_RAYLIB();

        RLGauge lGauge(TEST_BOUNDS, 0.0f, 100.0f);

        Rectangle lNewBounds = {100, 100, 200, 200};
        lGauge.setBounds(lNewBounds);

        // Value should be preserved after bounds change
        lGauge.setValue(75.0f);
        CHECK(lGauge.getValue() == doctest::Approx(75.0f));
    }

}

TEST_SUITE("RLAreaChart") {

    TEST_CASE("Mode switching") {
        REQUIRE_RAYLIB();

        RLAreaChart lChart(TEST_BOUNDS, RLAreaChartMode::STACKED);
        CHECK(lChart.getMode() == RLAreaChartMode::STACKED);

        lChart.setMode(RLAreaChartMode::OVERLAPPED);
        CHECK(lChart.getMode() == RLAreaChartMode::OVERLAPPED);

        lChart.setMode(RLAreaChartMode::PERCENT);
        CHECK(lChart.getMode() == RLAreaChartMode::PERCENT);
    }

    TEST_CASE("Data setting") {
        REQUIRE_RAYLIB();

        RLAreaChart lChart(TEST_BOUNDS, RLAreaChartMode::STACKED);

        std::vector<RLAreaSeries> lData;
        RLAreaSeries lSeries1;
        lSeries1.mValues = {10.0f, 20.0f, 30.0f};
        lSeries1.mColor = RED;
        lSeries1.mLabel = "Series A";
        lData.push_back(lSeries1);

        RLAreaSeries lSeries2;
        lSeries2.mValues = {15.0f, 25.0f, 35.0f};
        lSeries2.mColor = BLUE;
        lSeries2.mLabel = "Series B";
        lData.push_back(lSeries2);

        lChart.setData(lData);
        CHECK(lChart.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Animation with target data") {
        REQUIRE_RAYLIB();

        RLAreaChart lChart(TEST_BOUNDS, RLAreaChartMode::STACKED);

        std::vector<RLAreaSeries> lInitial;
        RLAreaSeries lS1;
        lS1.mValues = {10.0f, 20.0f, 30.0f};
        lInitial.push_back(lS1);

        std::vector<RLAreaSeries> lTarget;
        RLAreaSeries lS2;
        lS2.mValues = {50.0f, 60.0f, 70.0f};
        lTarget.push_back(lS2);

        lChart.setData(lInitial);
        lChart.setTargetData(lTarget);

        // Run animation
        for (int i = 0; i < 100; i++) {
            lChart.update(0.016f);
        }

        // Chart should have processed the animation (no crash)
        CHECK(lChart.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Percent mode max value") {
        REQUIRE_RAYLIB();

        RLAreaChart lChart(TEST_BOUNDS, RLAreaChartMode::PERCENT);

        std::vector<RLAreaSeries> lData;
        RLAreaSeries lS1;
        lS1.mValues = {10.0f, 20.0f, 30.0f};
        lData.push_back(lS1);

        lChart.setData(lData);

        // In percent mode, max value should be 100
        // (need to update first to converge)
        for (int i = 0; i < 50; i++) {
            lChart.update(0.016f);
        }
        CHECK(lChart.getMaxValue() == doctest::Approx(100.0f).epsilon(0.1));
    }

}

TEST_SUITE("RLBarChart") {

    TEST_CASE("Data setting") {
        REQUIRE_RAYLIB();

        RLBarChart lChart(TEST_BOUNDS, RLBarOrientation::VERTICAL);

        std::vector<RLBarData> lData = {
            {10.0f, RED, false, BLACK, "A"},
            {20.0f, GREEN, false, BLACK, "B"},
            {30.0f, BLUE, false, BLACK, "C"}
        };

        lChart.setData(lData);
        CHECK(lChart.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Orientation change") {
        REQUIRE_RAYLIB();

        RLBarChart lChart(TEST_BOUNDS, RLBarOrientation::VERTICAL);
        CHECK(lChart.getOrientation() == RLBarOrientation::VERTICAL);

        lChart.setOrientation(RLBarOrientation::HORIZONTAL);
        CHECK(lChart.getOrientation() == RLBarOrientation::HORIZONTAL);
    }

    TEST_CASE("Animation with target data") {
        REQUIRE_RAYLIB();

        RLBarChart lChart(TEST_BOUNDS, RLBarOrientation::VERTICAL);

        std::vector<RLBarData> lInitial = {{10.0f, RED, false, BLACK, "A"}};
        std::vector<RLBarData> lTarget = {{50.0f, RED, false, BLACK, "A"}};

        lChart.setData(lInitial);
        lChart.setTargetData(lTarget);

        // Run animation
        for (int i = 0; i < 100; i++) {
            lChart.update(0.016f);
        }

        // Chart should have processed the animation (no crash)
        CHECK(lChart.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Explicit scale") {
        REQUIRE_RAYLIB();

        RLBarChart lChart(TEST_BOUNDS, RLBarOrientation::VERTICAL);

        RLBarChartStyle lStyle;
        lStyle.mAutoScale = false;
        lStyle.mMinValue = 0.0f;
        lStyle.mMaxValue = 100.0f;
        lChart.setStyle(lStyle);
        lChart.setScale(0.0f, 200.0f);

        // Should accept explicit scale without crash
        CHECK(lChart.getBounds().height == doctest::Approx(300.0f));
    }

}

TEST_SUITE("RLPieChart") {

    TEST_CASE("Hollow factor clamping") {
        REQUIRE_RAYLIB();

        RLPieChart lChart(TEST_BOUNDS);

        lChart.setHollowFactor(0.5f);
        CHECK(lChart.getHollowFactor() == doctest::Approx(0.5f));

        lChart.setHollowFactor(-0.5f);
        CHECK(lChart.getHollowFactor() >= 0.0f);

        lChart.setHollowFactor(1.5f);
        CHECK(lChart.getHollowFactor() <= 1.0f);
    }

    TEST_CASE("Slice data handling") {
        REQUIRE_RAYLIB();

        RLPieChart lChart(TEST_BOUNDS);

        std::vector<RLPieSliceData> lData = {
            {25.0f, RED, "Q1"},
            {25.0f, GREEN, "Q2"},
            {25.0f, BLUE, "Q3"},
            {25.0f, YELLOW, "Q4"}
        };

        lChart.setData(lData);
        CHECK(lChart.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Animation with target data") {
        REQUIRE_RAYLIB();

        RLPieChart lChart(TEST_BOUNDS);

        std::vector<RLPieSliceData> lInitial = {{50.0f, RED, "A"}, {50.0f, BLUE, "B"}};
        std::vector<RLPieSliceData> lTarget = {{75.0f, RED, "A"}, {25.0f, BLUE, "B"}};

        lChart.setData(lInitial);
        lChart.setTargetData(lTarget);

        for (int i = 0; i < 100; i++) {
            lChart.update(0.016f);
        }

        CHECK(lChart.getBounds().width == doctest::Approx(400.0f));
    }

}

TEST_SUITE("RLRadarChart") {

    TEST_CASE("Axis configuration") {
        REQUIRE_RAYLIB();

        RLRadarChart lChart(TEST_BOUNDS);

        // Set axes with labels
        std::vector<std::string> lLabels = {"Axis1", "Axis2", "Axis3", "Axis4", "Axis5"};
        lChart.setAxes(lLabels, 0.0f, 100.0f);

        CHECK(lChart.getAxisCount() == 5);
        CHECK(lChart.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Series management") {
        REQUIRE_RAYLIB();

        RLRadarChart lChart(TEST_BOUNDS);

        std::vector<std::string> lLabels = {"A", "B", "C", "D", "E", "F"};
        lChart.setAxes(lLabels, 0.0f, 100.0f);

        // Add series
        RLRadarSeries lSeries1;
        lSeries1.mLabel = "Series 1";
        lSeries1.mValues = {50.0f, 60.0f, 70.0f, 80.0f, 90.0f, 100.0f};
        lSeries1.mLineColor = RED;
        lChart.addSeries(lSeries1);

        CHECK(lChart.getSeriesCount() == 1);

        // Add another series
        RLRadarSeries lSeries2;
        lSeries2.mLabel = "Series 2";
        lSeries2.mValues = {30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f};
        lSeries2.mLineColor = BLUE;
        lChart.addSeries(lSeries2);

        CHECK(lChart.getSeriesCount() == 2);
    }

    TEST_CASE("Series data update") {
        REQUIRE_RAYLIB();

        RLRadarChart lChart(TEST_BOUNDS);

        std::vector<std::string> lLabels = {"A", "B", "C", "D"};
        lChart.setAxes(lLabels, 0.0f, 100.0f);

        RLRadarSeries lSeries;
        lSeries.mLabel = "Test";
        lSeries.mValues = {25.0f, 50.0f, 75.0f, 100.0f};
        lChart.addSeries(lSeries);

        // Update series data
        std::vector<float> lNewValues = {10.0f, 20.0f, 30.0f, 40.0f};
        lChart.setSeriesData(0, lNewValues);

        // Should accept without crash
        CHECK(lChart.getSeriesCount() == 1);
    }

    TEST_CASE("Animation convergence") {
        REQUIRE_RAYLIB();

        RLRadarChart lChart(TEST_BOUNDS);

        std::vector<std::string> lLabels = {"A", "B", "C", "D", "E"};
        lChart.setAxes(lLabels, 0.0f, 100.0f);

        RLRadarSeries lSeries;
        lSeries.mLabel = "Animated";
        lSeries.mValues = {20.0f, 40.0f, 60.0f, 80.0f, 100.0f};
        lChart.addSeries(lSeries);

        // Run animation
        for (int i = 0; i < 100; i++) {
            lChart.update(0.016f);
        }

        // Should have animated without crash
        CHECK(lChart.getSeriesCount() == 1);
    }

    TEST_CASE("Series removal") {
        REQUIRE_RAYLIB();

        RLRadarChart lChart(TEST_BOUNDS);

        std::vector<std::string> lLabels = {"A", "B", "C"};
        lChart.setAxes(lLabels, 0.0f, 100.0f);

        RLRadarSeries lSeries1;
        lSeries1.mValues = {50.0f, 60.0f, 70.0f};
        lChart.addSeries(lSeries1);

        RLRadarSeries lSeries2;
        lSeries2.mValues = {30.0f, 40.0f, 50.0f};
        lChart.addSeries(lSeries2);

        CHECK(lChart.getSeriesCount() == 2);

        // Remove series (starts fade-out)
        lChart.removeSeries(0);

        // After animation, series count should decrease
        for (int i = 0; i < 200; i++) {
            lChart.update(0.016f);
        }

        CHECK(lChart.getSeriesCount() <= 2);
    }

    TEST_CASE("Bounds update") {
        REQUIRE_RAYLIB();

        RLRadarChart lChart(TEST_BOUNDS);

        Rectangle lNewBounds = {100, 100, 600, 500};
        lChart.setBounds(lNewBounds);

        CHECK(lChart.getBounds().x == doctest::Approx(100.0f));
        CHECK(lChart.getBounds().y == doctest::Approx(100.0f));
        CHECK(lChart.getBounds().width == doctest::Approx(600.0f));
        CHECK(lChart.getBounds().height == doctest::Approx(500.0f));
    }

}

TEST_SUITE("RLScatterPlot") {

    TEST_CASE("Series management") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        RLScatterSeries lSeries;
        lSeries.mData = {{0.0f, 0.0f}, {1.0f, 1.0f}, {2.0f, 4.0f}};

        lPlot.addSeries(lSeries);
        CHECK(lPlot.seriesCount() == 1);

        lPlot.addSeries(lSeries);
        CHECK(lPlot.seriesCount() == 2);

        lPlot.clearSeries();
        CHECK(lPlot.seriesCount() == 0);
    }

    TEST_CASE("Auto scale bounds") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        RLScatterSeries lSeries;
        lSeries.mData = {{-10.0f, -5.0f}, {10.0f, 15.0f}};

        lPlot.addSeries(lSeries);
        lPlot.update(0.016f);

        // Plot should handle the data range
        CHECK(lPlot.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Style configuration") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        RLScatterPlotStyle lStyle;
        lStyle.mAutoScale = false;
        lStyle.mMinX = -100.0f;
        lStyle.mMaxX = 100.0f;
        lStyle.mMinY = -50.0f;
        lStyle.mMaxY = 50.0f;
        lStyle.mShowGrid = true;
        lStyle.mGridLines = 8;
        lStyle.mPadding = 20.0f;

        lPlot.setStyle(lStyle);

        // Style should be applied without crash
        CHECK(lPlot.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Bounds update") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        Rectangle lNewBounds = {50, 50, 600, 400};
        lPlot.setBounds(lNewBounds);

        CHECK(lPlot.getBounds().x == doctest::Approx(50.0f));
        CHECK(lPlot.getBounds().y == doctest::Approx(50.0f));
        CHECK(lPlot.getBounds().width == doctest::Approx(600.0f));
        CHECK(lPlot.getBounds().height == doctest::Approx(400.0f));
    }

    TEST_CASE("Explicit scale") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        RLScatterPlotStyle lStyle;
        lStyle.mAutoScale = false;
        lPlot.setStyle(lStyle);
        lPlot.setScale(-10.0f, 10.0f, -5.0f, 5.0f);

        // Add data outside the scale range - should still work
        RLScatterSeries lSeries;
        lSeries.mData = {{-20.0f, -10.0f}, {20.0f, 10.0f}};
        lPlot.addSeries(lSeries);
        lPlot.update(0.016f);

        CHECK(lPlot.seriesCount() == 1);
    }

    TEST_CASE("Animation convergence") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        RLScatterSeries lSeries;
        lSeries.mData = {{0.0f, 0.0f}, {1.0f, 1.0f}};
        size_t lIdx = lPlot.addSeries(lSeries);

        // Set target data for animation
        std::vector<Vector2> lTarget = {{0.5f, 0.5f}, {2.0f, 3.0f}};
        lPlot.setSeriesTargetData(lIdx, lTarget);

        // Run animation
        for (int i = 0; i < 100; i++) {
            lPlot.update(0.016f);
        }

        // Should have animated without crash
        CHECK(lPlot.seriesCount() == 1);
    }

    TEST_CASE("Single series API") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        std::vector<Vector2> lData = {{0.0f, 0.0f}, {1.0f, 2.0f}, {2.0f, 1.0f}};
        RLScatterSeriesStyle lStyle;
        lStyle.mLineMode = RLScatterLineMode::Spline;
        lStyle.mShowPoints = true;

        lPlot.setSingleSeries(lData, lStyle);
        CHECK(lPlot.seriesCount() == 1);

        // Update single series target data
        std::vector<Vector2> lTarget = {{0.5f, 1.0f}, {1.5f, 2.5f}, {2.5f, 0.5f}};
        lPlot.setSingleSeriesTargetData(lTarget);

        for (int i = 0; i < 50; i++) {
            lPlot.update(0.016f);
        }

        CHECK(lPlot.seriesCount() == 1);
    }

    TEST_CASE("Multiple series with different styles") {
        REQUIRE_RAYLIB();

        RLScatterPlot lPlot(TEST_BOUNDS);

        RLScatterSeries lSeries1;
        lSeries1.mData = {{0.0f, 0.0f}, {1.0f, 1.0f}};
        lSeries1.mStyle.mLineMode = RLScatterLineMode::Linear;
        lSeries1.mStyle.mLineColor = RED;

        RLScatterSeries lSeries2;
        lSeries2.mData = {{0.0f, 1.0f}, {1.0f, 0.0f}};
        lSeries2.mStyle.mLineMode = RLScatterLineMode::None;
        lSeries2.mStyle.mLineColor = BLUE;

        RLScatterSeries lSeries3;
        lSeries3.mData = {{0.5f, 0.0f}, {0.5f, 1.0f}};
        lSeries3.mStyle.mLineMode = RLScatterLineMode::Spline;
        lSeries3.mStyle.mLineColor = GREEN;

        lPlot.addSeries(lSeries1);
        lPlot.addSeries(lSeries2);
        lPlot.addSeries(lSeries3);

        CHECK(lPlot.seriesCount() == 3);

        lPlot.update(0.016f);
        CHECK(lPlot.getBounds().width == doctest::Approx(400.0f));
    }

}

TEST_SUITE("RLTimeSeries") {

    TEST_CASE("Trace management") {
        REQUIRE_RAYLIB();

        RLTimeSeries lTs(TEST_BOUNDS, 100);

        CHECK(lTs.getTraceCount() == 0);

        size_t lIdx = lTs.addTrace();
        CHECK(lTs.getTraceCount() == 1);
        CHECK(lIdx == 0);

        lIdx = lTs.addTrace();
        CHECK(lTs.getTraceCount() == 2);
        CHECK(lIdx == 1);
    }

    TEST_CASE("Sample streaming") {
        REQUIRE_RAYLIB();

        RLTimeSeries lTs(TEST_BOUNDS, 10); // Small window for testing
        size_t lTraceIdx = lTs.addTrace();

        // Add samples
        for (int i = 0; i < 15; i++) {
            lTs.pushSample(lTraceIdx, (float)i);
        }

        // Ring buffer should handle overflow
        CHECK(lTs.getTraceCount() == 1);
    }

    TEST_CASE("Window size") {
        REQUIRE_RAYLIB();

        RLTimeSeries lTs(TEST_BOUNDS, 50);

        CHECK(lTs.getWindowSize() == 50);

        lTs.setWindowSize(100);
        CHECK(lTs.getWindowSize() == 100);

        CHECK(lTs.getBounds().width == doctest::Approx(400.0f));
    }

}

TEST_SUITE("RLHeatMap") {

    TEST_CASE("Grid configuration") {
        REQUIRE_RAYLIB();

        RLHeatMap lHm(TEST_BOUNDS, 32, 32);

        CHECK(lHm.getCellsX() == 32);
        CHECK(lHm.getCellsY() == 32);

        lHm.setGrid(64, 64);
        CHECK(lHm.getCellsX() == 64);
        CHECK(lHm.getCellsY() == 64);
    }

    TEST_CASE("Update modes") {
        REQUIRE_RAYLIB();

        RLHeatMap lHm(TEST_BOUNDS, 16, 16);

        lHm.setUpdateMode(RLHeatMapUpdateMode::Replace);
        CHECK(lHm.getUpdateMode() == RLHeatMapUpdateMode::Replace);

        lHm.setUpdateMode(RLHeatMapUpdateMode::Accumulate);
        CHECK(lHm.getUpdateMode() == RLHeatMapUpdateMode::Accumulate);

        lHm.setUpdateMode(RLHeatMapUpdateMode::Decay);
        CHECK(lHm.getUpdateMode() == RLHeatMapUpdateMode::Decay);
    }

    TEST_CASE("Point addition") {
        REQUIRE_RAYLIB();

        RLHeatMap lHm(TEST_BOUNDS, 16, 16);

        Vector2 lPoints[] = {{0.0f, 0.0f}, {0.5f, 0.5f}, {-0.5f, -0.5f}};
        lHm.addPoints(lPoints, 3);

        // Should handle points without crash
        lHm.update(0.016f);
        CHECK(lHm.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Clear") {
        REQUIRE_RAYLIB();

        RLHeatMap lHm(TEST_BOUNDS, 16, 16);

        Vector2 lPoints[] = {{0.0f, 0.0f}};
        lHm.addPoints(lPoints, 1);
        lHm.clear();

        // Should clear without crash
        CHECK(lHm.getCellsX() == 16);
    }

}

TEST_SUITE("RLHeatMap3D") {

    TEST_CASE("Grid configuration") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(32, 32);

        CHECK(lHm.getWidth() == 32);
        CHECK(lHm.getHeight() == 32);

        lHm.setGridSize(64, 48);
        CHECK(lHm.getWidth() == 64);
        CHECK(lHm.getHeight() == 48);
    }

    TEST_CASE("Value range configuration") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        lHm.setValueRange(0.0f, 100.0f);
        CHECK(lHm.getMinValue() == doctest::Approx(0.0f));
        CHECK(lHm.getMaxValue() == doctest::Approx(100.0f));

        lHm.setValueRange(-50.0f, 50.0f);
        CHECK(lHm.getMinValue() == doctest::Approx(-50.0f));
        CHECK(lHm.getMaxValue() == doctest::Approx(50.0f));
    }

    TEST_CASE("Auto range toggle") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        // Default should be auto range enabled
        CHECK(lHm.isAutoRange() == true);

        lHm.setAutoRange(false);
        CHECK(lHm.isAutoRange() == false);

        lHm.setAutoRange(true);
        CHECK(lHm.isAutoRange() == true);
    }

    TEST_CASE("Mode switching") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        lHm.setMode(RLHeatMap3DMode::Surface);
        CHECK(lHm.getMode() == RLHeatMap3DMode::Surface);

        lHm.setMode(RLHeatMap3DMode::Scatter);
        CHECK(lHm.getMode() == RLHeatMap3DMode::Scatter);
    }

    TEST_CASE("Set values") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(4, 4);

        // Create test data (4x4 = 16 values)
        float lValues[16];
        for (int i = 0; i < 16; i++) {
            lValues[i] = (float)i / 15.0f;
        }

        lHm.setValues(lValues, 16);
        lHm.update(0.016f);

        // Should process values without crash
        CHECK(lHm.getWidth() == 4);
        CHECK(lHm.getHeight() == 4);
    }

    TEST_CASE("Partial value update") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(8, 8);

        // Initialize with zeros
        float lInitial[64] = {0};
        lHm.setValues(lInitial, 64);

        // Update a 2x2 region at position (2, 2)
        float lPartial[4] = {1.0f, 0.8f, 0.6f, 0.4f};
        lHm.updatePartialValues(2, 2, 2, 2, lPartial);

        lHm.update(0.016f);
        CHECK(lHm.getWidth() == 8);
    }

    TEST_CASE("Axis range configuration") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        lHm.setAxisRangeX(-10.0f, 10.0f);
        lHm.setAxisRangeY(0.0f, 100.0f);
        lHm.setAxisRangeZ(-1.0f, 1.0f);

        // Should accept axis ranges without crash
        CHECK(lHm.getWidth() == 16);
    }

    TEST_CASE("Axis labels") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        lHm.setAxisLabels("Time (s)", "Frequency (Hz)", "Amplitude");

        // Should accept labels without crash
        CHECK(lHm.getWidth() == 16);
    }

    TEST_CASE("Palette configuration - 3 colors") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        lHm.setPalette(BLUE, GREEN, RED);

        // Set some values to test palette
        float lValues[256];
        for (int i = 0; i < 256; i++) {
            lValues[i] = (float)i / 255.0f;
        }
        lHm.setValues(lValues, 256);
        lHm.update(0.016f);

        CHECK(lHm.getWidth() == 16);
    }

    TEST_CASE("Palette configuration - 4 colors") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        lHm.setPalette(BLUE, SKYBLUE, YELLOW, RED);

        // Set some values to test palette
        float lValues[256];
        for (int i = 0; i < 256; i++) {
            lValues[i] = (float)i / 255.0f;
        }
        lHm.setValues(lValues, 256);
        lHm.update(0.016f);

        CHECK(lHm.getWidth() == 16);
    }

    TEST_CASE("Style configuration") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        RLHeatMap3DStyle lStyle;
        lStyle.mMode = RLHeatMap3DMode::Surface;
        lStyle.mSmoothingSpeed = 10.0f;
        lStyle.mShowWireframe = true;
        lStyle.mSurfaceOpacity = 0.9f;
        lStyle.mShowAxisBox = true;
        lStyle.mShowFloorGrid = true;
        lStyle.mShowAxisLabels = true;
        lStyle.mShowTicks = true;
        lStyle.mTickCount = 10;
        lStyle.mGridDivisions = 8;

        lHm.setStyle(lStyle);
        CHECK(lHm.getMode() == RLHeatMap3DMode::Surface);
    }

    TEST_CASE("Smoothing configuration") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(8, 8);

        lHm.setSmoothing(15.0f);

        // Set initial values
        float lInitial[64] = {0};
        lHm.setValues(lInitial, 64);
        lHm.update(0.016f);

        // Set new target values
        float lTarget[64];
        for (int i = 0; i < 64; i++) {
            lTarget[i] = 1.0f;
        }
        lHm.setValues(lTarget, 64);

        // Run animation
        for (int i = 0; i < 60; i++) {
            lHm.update(0.016f);
        }

        CHECK(lHm.getWidth() == 8);
    }

    TEST_CASE("Wireframe toggle") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(16, 16);

        lHm.setWireframe(true);
        lHm.update(0.016f);

        lHm.setWireframe(false);
        lHm.update(0.016f);

        CHECK(lHm.getWidth() == 16);
    }

    TEST_CASE("Point size for scatter mode") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm(8, 8);

        lHm.setMode(RLHeatMap3DMode::Scatter);
        lHm.setPointSize(0.25f);

        float lValues[64];
        for (int i = 0; i < 64; i++) {
            lValues[i] = (float)i / 63.0f;
        }
        lHm.setValues(lValues, 64);
        lHm.update(0.016f);

        CHECK(lHm.getMode() == RLHeatMap3DMode::Scatter);
    }

    TEST_CASE("Default constructor") {
        REQUIRE_RAYLIB();

        RLHeatMap3D lHm;

        // Default constructed should have 0x0 grid
        CHECK(lHm.getWidth() == 0);
        CHECK(lHm.getHeight() == 0);

        // Can set grid size after
        lHm.setGridSize(16, 16);
        CHECK(lHm.getWidth() == 16);
        CHECK(lHm.getHeight() == 16);
    }

}

TEST_SUITE("RLCandlestickChart") {

    TEST_CASE("Sample aggregation") {
        REQUIRE_RAYLIB();

        RLCandlestickChart lChart(TEST_BOUNDS, 5, 20);

        RLCandlestickChart::CandleInput lSample;
        lSample.aOpen = 100.0f;
        lSample.aHigh = 105.0f;
        lSample.aLow = 95.0f;
        lSample.aClose = 102.0f;
        lSample.aVolume = 1000.0f;
        lSample.aDate = "2024-01-15 09:30:00";

        // Add multiple samples to form candles
        for (int i = 0; i < 10; i++) {
            lChart.addSample(lSample);
        }

        lChart.update(0.016f);
        // Chart processed samples without crash
        CHECK(true);
    }

    TEST_CASE("Configuration changes") {
        REQUIRE_RAYLIB();

        RLCandlestickChart lChart(TEST_BOUNDS, 5, 20);

        lChart.setValuesPerCandle(10);
        lChart.setVisibleCandles(30);

        RLCandleStyle lStyle;
        lStyle.mAutoScale = false;
        lChart.setExplicitScale(90.0f, 110.0f);

        // Configuration applied without crash
        CHECK(true);
    }

}

TEST_SUITE("RLTreeMap") {

    TEST_CASE("Hierarchy data") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {
            {"Child1", 30.0f, RED, true, {}},
            {"Child2", 70.0f, BLUE, true, {}}
        };

        lTm.setData(lRoot);
        lTm.update(0.016f);

        CHECK(lTm.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Layout modes") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        lTm.setLayout(RLTreeMapLayout::SQUARIFIED);
        lTm.setLayout(RLTreeMapLayout::SLICE);
        lTm.setLayout(RLTreeMapLayout::DICE);
        lTm.setLayout(RLTreeMapLayout::SLICE_DICE);

        CHECK(lTm.getBounds().height == doctest::Approx(300.0f));
    }

    TEST_CASE("Style configuration") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeMapStyle lStyle;
        lStyle.mShowBackground = true;
        lStyle.mPaddingOuter = 8.0f;
        lStyle.mPaddingInner = 4.0f;
        lStyle.mPaddingTop = 24.0f;
        lStyle.mBorderThickness = 2.0f;
        lStyle.mCornerRadius = 5.0f;
        lStyle.mShowInternalNodes = true;
        lStyle.mShowLeafLabels = true;
        lStyle.mLabelFontSize = 16;
        lStyle.mSmoothAnimate = true;
        lStyle.mAnimateSpeed = 8.0f;

        lTm.setStyle(lStyle);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {{"A", 50.0f, RED, true, {}}, {"B", 50.0f, BLUE, true, {}}};
        lTm.setData(lRoot);

        CHECK(lTm.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Target data animation") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lInitial;
        lInitial.mLabel = "Root";
        lInitial.mChildren = {
            {"A", 30.0f, RED, true, {}},
            {"B", 70.0f, BLUE, true, {}}
        };

        RLTreeNode lTarget;
        lTarget.mLabel = "Root";
        lTarget.mChildren = {
            {"A", 60.0f, RED, true, {}},
            {"B", 40.0f, BLUE, true, {}}
        };

        lTm.setData(lInitial);
        lTm.setTargetData(lTarget);

        // Run animation
        for (int i = 0; i < 100; i++) {
            lTm.update(0.016f);
        }

        CHECK(lTm.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Node count") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {
            {"A", 25.0f, RED, true, {}},
            {"B", 25.0f, GREEN, true, {}},
            {"C", 50.0f, BLUE, true, {
                {"C1", 30.0f, YELLOW, true, {}},
                {"C2", 20.0f, ORANGE, true, {}}
            }}
        };

        lTm.setData(lRoot);
        lTm.update(0.016f);

        // Root + A + B + C + C1 + C2 = 6 nodes
        CHECK(lTm.getNodeCount() == 6);
    }

    TEST_CASE("Node highlighting") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {
            {"A", 50.0f, RED, true, {}},
            {"B", 50.0f, BLUE, true, {}}
        };

        lTm.setData(lRoot);
        lTm.update(0.016f);

        CHECK(lTm.getHighlightedNode() == -1);

        lTm.setHighlightedNode(1);
        CHECK(lTm.getHighlightedNode() == 1);

        lTm.setHighlightedNode(-1);
        CHECK(lTm.getHighlightedNode() == -1);
    }

    TEST_CASE("Get node at point") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {
            {"A", 50.0f, RED, true, {}},
            {"B", 50.0f, BLUE, true, {}}
        };

        lTm.setData(lRoot);

        // Force layout computation
        for (int i = 0; i < 50; i++) {
            lTm.update(0.016f);
        }

        // Point outside bounds should return -1
        int lOutside = lTm.getNodeAtPoint({-100.0f, -100.0f});
        CHECK(lOutside == -1);

        // Point inside bounds should return a valid index (could be root or a child)
        int lInside = lTm.getNodeAtPoint({200.0f, 150.0f});
        CHECK(lInside >= -1); // -1 if no node at point, >= 0 if found
    }

    TEST_CASE("Update value by path") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {
            {"Parent", 0.0f, GRAY, false, {
                {"Child1", 30.0f, RED, true, {}},
                {"Child2", 70.0f, BLUE, true, {}}
            }}
        };

        lTm.setData(lRoot);
        lTm.update(0.016f);

        // Update Child1 value via path
        std::vector<std::string> lPath = {"Parent", "Child1"};
        lTm.updateValue(lPath, 50.0f);

        // Run animation
        for (int i = 0; i < 50; i++) {
            lTm.update(0.016f);
        }

        CHECK(lTm.getNodeCount() > 0);
    }

    TEST_CASE("Bounds update") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {{"A", 100.0f, RED, true, {}}};
        lTm.setData(lRoot);

        Rectangle lNewBounds = {100, 100, 800, 600};
        lTm.setBounds(lNewBounds);

        CHECK(lTm.getBounds().x == doctest::Approx(100.0f));
        CHECK(lTm.getBounds().y == doctest::Approx(100.0f));
        CHECK(lTm.getBounds().width == doctest::Approx(800.0f));
        CHECK(lTm.getBounds().height == doctest::Approx(600.0f));
    }

    TEST_CASE("Recompute layout") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {
            {"A", 40.0f, RED, true, {}},
            {"B", 60.0f, BLUE, true, {}}
        };

        lTm.setData(lRoot);
        lTm.update(0.016f);

        size_t lInitialCount = lTm.getNodeCount();

        // Force recompute
        lTm.recomputeLayout();
        lTm.update(0.016f);

        CHECK(lTm.getNodeCount() == lInitialCount);
    }

    TEST_CASE("Computed rects access") {
        REQUIRE_RAYLIB();

        RLTreeMap lTm(TEST_BOUNDS);

        RLTreeNode lRoot;
        lRoot.mLabel = "Root";
        lRoot.mChildren = {
            {"A", 50.0f, RED, true, {}},
            {"B", 50.0f, BLUE, true, {}}
        };

        lTm.setData(lRoot);

        // Run enough updates for layout to stabilize
        for (int i = 0; i < 50; i++) {
            lTm.update(0.016f);
        }

        const auto& lRects = lTm.getComputedRects();
        CHECK(lRects.size() == lTm.getNodeCount());

        // Each rect should have valid dimensions after animation
        for (const auto& lRect : lRects) {
            CHECK(lRect.mAlpha >= 0.0f);
            CHECK(lRect.mAlpha <= 1.0f);
        }
    }

}

TEST_SUITE("RLLogPlot") {

    TEST_CASE("Time series streaming") {
        REQUIRE_RAYLIB();

        RLLogPlot lPlot(TEST_BOUNDS);

        lPlot.setWindowSize(100);
        CHECK(lPlot.getWindowSize() == 100);

        // Push samples
        for (int i = 0; i < 50; i++) {
            lPlot.pushSample((float)i * 0.1f);
        }

        CHECK(lPlot.getTimeSeriesSize() == 50);
        lPlot.update(0.016f);

        CHECK(lPlot.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Trace management") {
        REQUIRE_RAYLIB();

        RLLogPlot lPlot(TEST_BOUNDS);

        RLLogPlotTrace lTrace;
        lTrace.mXValues = {1.0f, 10.0f, 100.0f};
        lTrace.mYValues = {0.1f, 0.01f, 0.001f};

        size_t lIdx = lPlot.addTrace(lTrace);
        CHECK(lIdx == 0);
        CHECK(lPlot.getTraceCount() == 1);

        lPlot.clearTraces();
        CHECK(lPlot.getTraceCount() == 0);
    }

}

TEST_SUITE("RLOrderBookVis") {

    TEST_CASE("Configuration") {
        REQUIRE_RAYLIB();

        RLOrderBookVis lOb(TEST_BOUNDS, 100, 10);

        CHECK(lOb.getPriceLevels() == 10);
        CHECK(lOb.getHistoryLength() == 100);

        lOb.setPriceLevels(20);
        CHECK(lOb.getPriceLevels() == 20);

        lOb.setHistoryLength(50);
        CHECK(lOb.getHistoryLength() == 50);
    }

    TEST_CASE("Snapshot updates") {
        REQUIRE_RAYLIB();

        RLOrderBookVis lOb(TEST_BOUNDS, 10, 5);

        RLOrderBookSnapshot lSnapshot;
        lSnapshot.mBids = {{100.0f, 50.0f}, {99.0f, 30.0f}};
        lSnapshot.mAsks = {{101.0f, 40.0f}, {102.0f, 60.0f}};

        lOb.pushSnapshot(lSnapshot);
        lOb.update(0.016f);

        CHECK(lOb.getBounds().width == doctest::Approx(400.0f));
        CHECK(lOb.getSnapshotCount() == 1);
    }

}

TEST_SUITE("RLBubble") {

    TEST_CASE("Bubble data") {
        REQUIRE_RAYLIB();

        RLBubble lBubble(TEST_BOUNDS);

        std::vector<RLBubblePoint> lData = {
            {0.5f, 0.5f, 10.0f, RED},
            {0.2f, 0.8f, 20.0f, GREEN},
            {0.8f, 0.2f, 15.0f, BLUE}
        };

        lBubble.setData(lData);
        lBubble.update(0.016f);

        CHECK(lBubble.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Animation") {
        REQUIRE_RAYLIB();

        RLBubble lBubble(TEST_BOUNDS);

        std::vector<RLBubblePoint> lInitial = {{0.5f, 0.5f, 10.0f, RED}};
        std::vector<RLBubblePoint> lTarget = {{0.8f, 0.2f, 30.0f, BLUE}};

        lBubble.setData(lInitial);
        lBubble.setTargetData(lTarget);

        for (int i = 0; i < 100; i++) {
            lBubble.update(0.016f);
        }

        CHECK(lBubble.getBounds().height == doctest::Approx(300.0f));
    }

}

TEST_SUITE("RLSankey") {

    TEST_CASE("Node management") {
        REQUIRE_RAYLIB();

        RLSankey lSankey(TEST_BOUNDS);

        // Add nodes
        size_t lNode1 = lSankey.addNode("Source A", RED, 0);
        size_t lNode2 = lSankey.addNode("Target B", BLUE, 1);

        CHECK(lNode1 == 0);
        CHECK(lNode2 == 1);
        CHECK(lSankey.getNodeCount() == 2);

        // Update and check bounds
        lSankey.update(0.016f);
        CHECK(lSankey.getBounds().width == doctest::Approx(400.0f));
    }

    TEST_CASE("Link management") {
        REQUIRE_RAYLIB();

        RLSankey lSankey(TEST_BOUNDS);

        size_t lNode1 = lSankey.addNode("A", RED, 0);
        size_t lNode2 = lSankey.addNode("B", GREEN, 1);
        size_t lNode3 = lSankey.addNode("C", BLUE, 1);

        size_t lLink1 = lSankey.addLink(lNode1, lNode2, 50.0f);
        size_t lLink2 = lSankey.addLink(lNode1, lNode3, 30.0f);

        CHECK(lLink1 == 0);
        CHECK(lLink2 == 1);
        CHECK(lSankey.getLinkCount() == 2);

        lSankey.update(0.016f);
        CHECK(lSankey.getColumnCount() == 2);
    }

    TEST_CASE("Batch data") {
        REQUIRE_RAYLIB();

        RLSankey lSankey(TEST_BOUNDS);

        std::vector<RLSankeyNode> lNodes = {
            {"Source", RED, 0},
            {"Middle", GREEN, 1},
            {"Target", BLUE, 2}
        };

        std::vector<RLSankeyLink> lLinks = {
            {0, 1, 100.0f},
            {1, 2, 80.0f}
        };

        lSankey.setData(lNodes, lLinks);

        CHECK(lSankey.getNodeCount() == 3);
        CHECK(lSankey.getLinkCount() == 2);

        lSankey.update(0.016f);
        CHECK(lSankey.getColumnCount() == 3);
    }

    TEST_CASE("Value animation") {
        REQUIRE_RAYLIB();

        RLSankey lSankey(TEST_BOUNDS);

        lSankey.addNode("A", RED, 0);
        lSankey.addNode("B", BLUE, 1);
        size_t lLinkId = lSankey.addLink(0, 1, 50.0f);

        // Update several times
        for (int i = 0; i < 60; i++) {
            lSankey.update(0.016f);
        }

        // Change link value
        lSankey.setLinkValue(lLinkId, 100.0f);

        // Update more
        for (int i = 0; i < 60; i++) {
            lSankey.update(0.016f);
        }

        CHECK(lSankey.getBounds().height == doctest::Approx(300.0f));
    }

    TEST_CASE("Node removal") {
        REQUIRE_RAYLIB();

        RLSankey lSankey(TEST_BOUNDS);

        lSankey.addNode("A", RED, 0);
        lSankey.addNode("B", GREEN, 1);
        lSankey.addNode("C", BLUE, 2);
        lSankey.addLink(0, 1, 50.0f);
        lSankey.addLink(1, 2, 40.0f);

        CHECK(lSankey.getNodeCount() == 3);

        // Remove middle node
        lSankey.removeNode(1);

        // Animate removal
        for (int i = 0; i < 120; i++) {
            lSankey.update(0.016f);
        }

        // Node count should decrease after fade-out
        CHECK(lSankey.getNodeCount() < 3);
    }

    TEST_CASE("Clear") {
        REQUIRE_RAYLIB();

        RLSankey lSankey(TEST_BOUNDS);

        lSankey.addNode("A", RED, 0);
        lSankey.addNode("B", BLUE, 1);
        lSankey.addLink(0, 1, 50.0f);

        CHECK(lSankey.getNodeCount() == 2);
        CHECK(lSankey.getLinkCount() == 1);

        lSankey.clear();

        CHECK(lSankey.getNodeCount() == 0);
        CHECK(lSankey.getLinkCount() == 0);
    }

    TEST_CASE("Auto column assignment") {
        REQUIRE_RAYLIB();

        RLSankey lSankey(TEST_BOUNDS);

        // Add nodes without explicit columns
        RLSankeyNode lNodeA;
        lNodeA.mLabel = "A";
        lNodeA.mColumn = -1; // Auto

        RLSankeyNode lNodeB;
        lNodeB.mLabel = "B";
        lNodeB.mColumn = -1;

        RLSankeyNode lNodeC;
        lNodeC.mLabel = "C";
        lNodeC.mColumn = -1;

        lSankey.addNode(lNodeA);
        lSankey.addNode(lNodeB);
        lSankey.addNode(lNodeC);

        // Links: A -> B -> C
        lSankey.addLink(0, 1, 50.0f);
        lSankey.addLink(1, 2, 40.0f);

        lSankey.update(0.016f);

        // Should auto-assign 3 columns
        CHECK(lSankey.getColumnCount() == 3);
    }

    TEST_CASE("Flow conservation validation - valid flow") {
        REQUIRE_RAYLIB();

        RLSankeyStyle lStyle;
        lStyle.mStrictFlowConservation = true;
        lStyle.mFlowTolerance = 0.001f;
        RLSankey lSankey(TEST_BOUNDS, lStyle);

        // Create a balanced flow: A(50) + B(30) -> Middle(80) -> X(45) + Y(35)
        std::vector<RLSankeyNode> lNodes = {
            {"Source A", RED, 0},
            {"Source B", GREEN, 0},
            {"Middle", BLUE, 1},
            {"Target X", ORANGE, 2},
            {"Target Y", PURPLE, 2}
        };

        std::vector<RLSankeyLink> lLinks = {
            {0, 2, 50.0f},  // A -> Middle
            {1, 2, 30.0f},  // B -> Middle (total in = 80)
            {2, 3, 45.0f},  // Middle -> X
            {2, 4, 35.0f}   // Middle -> Y (total out = 80)
        };

        bool lValid = lSankey.setData(lNodes, lLinks);
        CHECK(lValid == true);
        CHECK(lSankey.validateFlowConservation() == true);
    }

    TEST_CASE("Flow conservation validation - invalid flow") {
        REQUIRE_RAYLIB();

        RLSankeyStyle lStyle;
        lStyle.mStrictFlowConservation = true;
        lStyle.mFlowTolerance = 0.001f;
        RLSankey lSankey(TEST_BOUNDS, lStyle);

        // Create an unbalanced flow: A(50) + B(30) -> Middle(80) -> X(45) + Y(20)
        // Inflow = 80, Outflow = 65 (difference of 15)
        std::vector<RLSankeyNode> lNodes = {
            {"Source A", RED, 0},
            {"Source B", GREEN, 0},
            {"Middle", BLUE, 1},
            {"Target X", ORANGE, 2},
            {"Target Y", PURPLE, 2}
        };

        std::vector<RLSankeyLink> lLinks = {
            {0, 2, 50.0f},  // A -> Middle
            {1, 2, 30.0f},  // B -> Middle (total in = 80)
            {2, 3, 45.0f},  // Middle -> X
            {2, 4, 20.0f}   // Middle -> Y (total out = 65, unbalanced!)
        };

        bool lValid = lSankey.setData(lNodes, lLinks);
        CHECK(lValid == false);
        CHECK(lSankey.validateFlowConservation() == false);
    }

    TEST_CASE("Flow conservation - edge nodes excluded") {
        REQUIRE_RAYLIB();

        RLSankeyStyle lStyle;
        lStyle.mStrictFlowConservation = true;
        lStyle.mFlowTolerance = 0.001f;
        RLSankey lSankey(TEST_BOUNDS, lStyle);

        // Edge nodes (source-only or sink-only) should not be validated
        std::vector<RLSankeyNode> lNodes = {
            {"Source", RED, 0},
            {"Target", BLUE, 1}
        };

        std::vector<RLSankeyLink> lLinks = {
            {0, 1, 100.0f}
        };

        // This should be valid - no intermediate nodes to check
        bool lValid = lSankey.setData(lNodes, lLinks);
        CHECK(lValid == true);
        CHECK(lSankey.validateFlowConservation() == true);
    }

    TEST_CASE("Flow mode - normalized vs raw") {
        REQUIRE_RAYLIB();

        // Test that both flow modes can be set without issues
        RLSankeyStyle lStyleNorm;
        lStyleNorm.mFlowMode = RLSankeyFlowMode::NORMALIZED;
        RLSankey lSankeyNorm(TEST_BOUNDS, lStyleNorm);

        RLSankeyStyle lStyleRaw;
        lStyleRaw.mFlowMode = RLSankeyFlowMode::RAW_VALUE;
        RLSankey lSankeyRaw(TEST_BOUNDS, lStyleRaw);

        // Add same data to both
        std::vector<RLSankeyNode> lNodes = {
            {"A", RED, 0},
            {"B", GREEN, 1},
            {"C", BLUE, 2}
        };

        std::vector<RLSankeyLink> lLinks = {
            {0, 1, 100.0f},
            {1, 2, 80.0f}  // Intentionally unbalanced
        };

        lSankeyNorm.setData(lNodes, lLinks);
        lSankeyRaw.setData(lNodes, lLinks);

        // Both should update without issues
        lSankeyNorm.update(0.016f);
        lSankeyRaw.update(0.016f);

        CHECK(lSankeyNorm.getNodeCount() == 3);
        CHECK(lSankeyRaw.getNodeCount() == 3);
    }

}
