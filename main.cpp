// main.cpp
// Demonstration of all chart types together to test for static declaration conflicts
#include "raylib.h"
#include "src/charts/RLBarChart.h"
#include "src/charts/RLBubble.h"
#include "src/charts/RLCandlestickChart.h"
#include "src/charts/RLGauge.h"
#include "src/charts/RLHeatMap.h"
#include "src/charts/RLPieChart.h"
#include "src/charts/RLScatterPlot.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// Helper to generate random float in range
float randFloat(float aMin, float aMax) {
    return aMin + ((float)rand() / (float)RAND_MAX) * (aMax - aMin);
}

// Helper to get palette color
Color paletteColor(int aIndex) {
    const Color PALETTE[] = {
        Color{0, 190, 255, 230},
        Color{80, 220, 120, 230},
        Color{255, 140, 80, 230},
        Color{255, 95, 120, 230},
        Color{170, 120, 255, 230},
        Color{255, 220, 80, 230},
        Color{80, 210, 200, 230},
        Color{210, 120, 200, 230}
    };
    return PALETTE[aIndex % 8];
}

int main() {
    srand((unsigned)time(nullptr));

    const int SCREEN_WIDTH = 1920;
    const int SCREEN_HEIGHT = 1080;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RayLib Charts - All Charts Demo");
    SetTargetFPS(60);

    // Layout: 3x3 grid with small gaps
    const float GAP = 10.0f;
    const float MARGIN = 20.0f;
    const float CHART_WIDTH = (SCREEN_WIDTH - 2 * MARGIN - 2 * GAP) / 3.0f;
    const float CHART_HEIGHT = (SCREEN_HEIGHT - 2 * MARGIN - 2 * GAP) / 3.0f;

    // Helper to get chart bounds
    auto getChartBounds = [&](int aRow, int aCol) -> Rectangle {
        float lX = MARGIN + aCol * (CHART_WIDTH + GAP);
        float lY = MARGIN + aRow * (CHART_HEIGHT + GAP);
        return Rectangle{lX, lY, CHART_WIDTH, CHART_HEIGHT};
    };

    // ===== 1. Bar Chart (Vertical) =====
    RLBarChartStyle lBarStyle;
    lBarStyle.mBackground = Color{24, 26, 32, 255};
    lBarStyle.mShowGrid = true;
    lBarStyle.mGridLines = 4;
    lBarStyle.mSpacing = 8.0f;
    lBarStyle.mCornerRadius = 6.0f;
    lBarStyle.lLabelFontSize = 12;

    RLBarChart lBarChart(getChartBounds(0, 0), RLBarOrientation::VERTICAL, lBarStyle);
    std::vector<RLBarData> lBarData;
    for (int i = 0; i < 6; ++i) {
        RLBarData lBar;
        lBar.value = randFloat(10.0f, 100.0f);
        lBar.color = paletteColor(i);
        lBar.showBorder = true;
        lBar.borderColor = Color{0, 0, 0, 100};
        lBar.label = std::to_string((int)lBar.value);
        lBarData.push_back(lBar);
    }
    lBarChart.setData(lBarData);

    // ===== 2. Bubble Chart =====
    RLBubbleStyle lBubbleStyle;
    lBubbleStyle.mBackground = Color{20, 22, 28, 255};
    lBubbleStyle.mShowAxes = true;
    lBubbleStyle.mGridLines = 4;
    lBubbleStyle.mSizeScale = 20.0f;

    RLBubble lBubble(getChartBounds(0, 1), RLBubbleMode::Scatter, lBubbleStyle);
    std::vector<RLBubblePoint> lBubbleData;
    for (int i = 0; i < 15; ++i) {
        RLBubblePoint lPoint;
        lPoint.mX = randFloat(0.1f, 0.9f);
        lPoint.mY = randFloat(0.1f, 0.9f);
        lPoint.mSize = randFloat(1.0f, 5.0f);
        lPoint.mColor = paletteColor(i);
        lBubbleData.push_back(lPoint);
    }
    lBubble.SetData(lBubbleData);

    // ===== 3. Candlestick Chart =====
    RLCandleStyle lCandleStyle;
    lCandleStyle.mBackground = Color{20, 22, 28, 255};
    lCandleStyle.mGridLines = 4;
    lCandleStyle.mCandleSpacing = 3.0f;
    lCandleStyle.mBodyMinWidth = 4.0f;

    RLCandlestickChart lCandlestick(getChartBounds(0, 2), 1, 20, lCandleStyle);
    float lPrice = 100.0f;
    for (int i = 0; i < 25; ++i) {
        RLCandlestickChart::CandleInput lCandle;
        lCandle.aOpen = lPrice;
        float lChange = randFloat(-5.0f, 5.0f);
        lCandle.aClose = lPrice + lChange;
        lCandle.aHigh = fmaxf(lCandle.aOpen, lCandle.aClose) + randFloat(0.5f, 2.0f);
        lCandle.aLow = fminf(lCandle.aOpen, lCandle.aClose) - randFloat(0.5f, 2.0f);
        lCandle.aVolume = randFloat(1000.0f, 5000.0f);
        lCandle.aDate = "2024-01-" + std::to_string(i + 1);
        lCandlestick.addSample(lCandle);
        lPrice = lCandle.aClose;
    }

    // ===== 4. Gauge =====
    RLGaugeStyle lGaugeStyle;
    lGaugeStyle.mBackgroundColor = Color{30, 30, 36, 255};
    lGaugeStyle.mBaseArcColor = Color{60, 60, 70, 255};
    lGaugeStyle.mValueArcColor = Color{0, 180, 255, 255};
    lGaugeStyle.mNeedleColor = Color{255, 74, 74, 255};
    lGaugeStyle.mThickness = 16.0f;
    lGaugeStyle.mTickCount = 50;
    lGaugeStyle.mShowValueText = true;

    RLGauge lGauge(getChartBounds(1, 0), 0.0f, 100.0f, lGaugeStyle);
    lGauge.setValue(65.0f);

    // ===== 5. Heat Map =====
    RLHeatMapStyle lHeatMapStyle;
    lHeatMapStyle.mBackground = Color{20, 22, 28, 255};
    lHeatMapStyle.mShowBorder = true;
    lHeatMapStyle.mBorderColor = Color{40, 44, 52, 255};

    RLHeatMap lHeatMap(getChartBounds(1, 1), 64, 64);
    lHeatMap.setStyle(lHeatMapStyle);
    lHeatMap.setUpdateMode(RLHeatMapUpdateMode::Accumulate);

    // Add some random points
    std::vector<Vector2> lHeatPoints;
    for (int i = 0; i < 200; ++i) {
        float lAngle = randFloat(0.0f, 6.28318f);
        float lRadius = randFloat(0.0f, 0.8f);
        lHeatPoints.push_back({cosf(lAngle) * lRadius, sinf(lAngle) * lRadius});
    }
    lHeatMap.addPoints(lHeatPoints.data(), lHeatPoints.size());

    // ===== 6. Pie Chart =====
    RLPieChartStyle lPieStyle;
    lPieStyle.mBackground = Color{20, 22, 28, 255};
    lPieStyle.mShowBackground = true;
    lPieStyle.mPadding = 10.0f;

    RLPieChart lPieChart(getChartBounds(1, 2), lPieStyle);
    lPieChart.setHollowFactor(0.4f); // Donut chart

    std::vector<RLPieSliceData> lPieData;
    for (int i = 0; i < 5; ++i) {
        RLPieSliceData lSlice;
        lSlice.mValue = randFloat(10.0f, 50.0f);
        lSlice.mColor = paletteColor(i);
        lSlice.mLabel = "Slice " + std::to_string(i + 1);
        lPieData.push_back(lSlice);
    }
    lPieChart.setData(lPieData);

    // ===== 7. Scatter Plot (Line) =====
    RLScatterPlotStyle lScatterStyle;
    lScatterStyle.mBackground = Color{20, 22, 28, 255};
    lScatterStyle.mShowGrid = true;
    lScatterStyle.mShowAxes = true;
    lScatterStyle.mGridLines = 4;
    lScatterStyle.mAutoScale = true;

    RLScatterPlot lScatterPlot(getChartBounds(2, 0), lScatterStyle);

    RLScatterSeriesStyle lSeriesStyle;
    lSeriesStyle.mLineColor = Color{80, 180, 255, 255};
    lSeriesStyle.mLineThickness = 2.0f;
    lSeriesStyle.mLineMode = RLScatterLineMode::Spline;
    lSeriesStyle.mShowPoints = true;
    lSeriesStyle.mPointScale = 2.0f;

    std::vector<Vector2> lScatterData;
    for (int i = 0; i < 20; ++i) {
        float lX = (float)i / 19.0f;
        float lY = 0.5f + 0.3f * sinf(lX * 6.28318f * 2.0f) + randFloat(-0.05f, 0.05f);
        lScatterData.push_back({lX, lY});
    }
    lScatterPlot.setSingleSeries(lScatterData, lSeriesStyle);

    // ===== 8. Bar Chart (Horizontal) =====
    RLBarChartStyle lBarStyle2 = lBarStyle;
    lBarStyle2.mShowLabels = false;

    RLBarChart lBarChart2(getChartBounds(2, 1), RLBarOrientation::HORIZONTAL, lBarStyle2);
    std::vector<RLBarData> lBarData2;
    for (int i = 0; i < 8; ++i) {
        RLBarData lBar;
        lBar.value = randFloat(20.0f, 80.0f);
        lBar.color = paletteColor(i);
        lBarData2.push_back(lBar);
    }
    lBarChart2.setData(lBarData2);

    // ===== 9. Gauge 2 (Different Value) =====
    RLGaugeStyle lGaugeStyle2 = lGaugeStyle;
    lGaugeStyle2.mValueArcColor = Color{255, 140, 80, 255};
    lGaugeStyle2.mNeedleColor = Color{120, 200, 100, 255};

    RLGauge lGauge2(getChartBounds(2, 2), 0.0f, 100.0f, lGaugeStyle2);
    lGauge2.setValue(35.0f);

    // Animation variables
    float lTime = 0.0f;
    float lGaugeTargetValue = 65.0f;
    float lGaugeTargetValue2 = 35.0f;

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Animate gauges smoothly
        if ((int)lTime % 3 == 0 && (int)(lTime * 10.0f) % 30 == 0) {
            lGaugeTargetValue = randFloat(20.0f, 95.0f);
            lGaugeTargetValue2 = randFloat(15.0f, 85.0f);
        }
        lGauge.setTargetValue(lGaugeTargetValue);
        lGauge2.setTargetValue(lGaugeTargetValue2);

        // Update all charts
        lBarChart.update(lDt);
        lBubble.Update(lDt);
        lCandlestick.update(lDt);
        lGauge.update(lDt);
        lHeatMap.update(lDt);
        lPieChart.update(lDt);
        lScatterPlot.update(lDt);
        lBarChart2.update(lDt);
        lGauge2.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground(Color{15, 17, 20, 255});

        // Draw title
        DrawText("RayLib Charts - All Chart Types (Testing Static Conflicts)",
                 10, 5, 20, Color{200, 200, 210, 255});

        // Draw all charts
        lBarChart.draw();
        lBubble.Draw();
        lCandlestick.draw();
        lGauge.draw();
        lHeatMap.draw();
        lPieChart.draw();
        lScatterPlot.draw();
        lBarChart2.draw();
        lGauge2.draw();

        // Draw labels for each chart
        const char* lLabels[] = {
            "Bar Chart", "Bubble Chart", "Candlestick",
            "Gauge", "Heat Map", "Pie Chart",
            "Scatter Plot", "Bar Chart H", "Gauge 2"
        };

        for (int lRow = 0; lRow < 3; ++lRow) {
            for (int lCol = 0; lCol < 3; ++lCol) {
                Rectangle lBounds = getChartBounds(lRow, lCol);
                int lIndex = lRow * 3 + lCol;
                DrawText(lLabels[lIndex], (int)lBounds.x + 5, (int)lBounds.y - 18,
                         16, Color{180, 180, 190, 255});
            }
        }

        DrawFPS(SCREEN_WIDTH - 100, 5);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

