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

