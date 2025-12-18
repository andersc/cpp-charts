// main.cpp
// Demonstration of all chart types together to test for static declaration conflicts
#include "raylib.h"
#include "src/charts/RLAreaChart.h"
#include "src/charts/RLBarChart.h"
#include "src/charts/RLBubble.h"
#include "src/charts/RLCandlestickChart.h"
#include "src/charts/RLGauge.h"
#include "src/charts/RLHeatMap.h"
#include "src/charts/RLHeatMap3D.h"
#include "src/charts/RLLinearGauge.h"
#include "src/charts/RLLogPlot.h"
#include "src/charts/RLOrderBookVis.h"
#include "src/charts/RLPieChart.h"
#include "src/charts/RLRadarChart.h"
#include "src/charts/RLSankey.h"
#include "src/charts/RLScatterPlot.h"
#include "src/charts/RLTimeSeries.h"
#include "src/charts/RLTreeMap.h"
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

    // Load custom font
    Font lBaseFont = LoadFontEx("base.ttf", 24, nullptr, 250);

    // Layout: 5x4 grid with small gaps (allows for 20 charts, using 17)
    const float GAP = 8.0f;
    const float MARGIN = 15.0f;
    const float CHART_WIDTH = (SCREEN_WIDTH - 2 * MARGIN - 4 * GAP) / 5.0f;
    const float CHART_HEIGHT = (SCREEN_HEIGHT - 2 * MARGIN - 3 * GAP) / 4.0f;

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
    lBarStyle.mLabelFontSize = 12;
    lBarStyle.mLabelFont = lBaseFont;

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
    lBubble.setData(lBubbleData);

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
    lGaugeStyle.mLabelFont = lBaseFont;

    RLGauge lGauge(getChartBounds(0, 3), 0.0f, 100.0f, lGaugeStyle);
    lGauge.setValue(65.0f);

    // ===== 5. Heat Map =====
    RLHeatMapStyle lHeatMapStyle;
    lHeatMapStyle.mBackground = Color{20, 22, 28, 255};
    lHeatMapStyle.mShowBorder = true;
    lHeatMapStyle.mBorderColor = Color{40, 44, 52, 255};

    RLHeatMap lHeatMap(getChartBounds(1, 0), 64, 64);
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

    RLPieChart lPieChart(getChartBounds(1, 1), lPieStyle);
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

    RLScatterPlot lScatterPlot(getChartBounds(1, 2), lScatterStyle);

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

    RLBarChart lBarChart2(getChartBounds(1, 3), RLBarOrientation::HORIZONTAL, lBarStyle2);
    std::vector<RLBarData> lBarData2;
    for (int i = 0; i < 8; ++i) {
        RLBarData lBar;
        lBar.value = randFloat(20.0f, 80.0f);
        lBar.color = paletteColor(i);
        lBarData2.push_back(lBar);
    }
    lBarChart2.setData(lBarData2);

    // ===== 9. Order Book Visualization =====
    RLOrderBookVisStyle lOrderBookStyle;
    lOrderBookStyle.mBackground = Color{20, 22, 28, 255};
    lOrderBookStyle.mShowBorder = true;
    lOrderBookStyle.mBorderColor = Color{40, 44, 52, 255};
    lOrderBookStyle.mShowGrid = true;
    lOrderBookStyle.mGridLinesX = 6;
    lOrderBookStyle.mGridLinesY = 4;
    lOrderBookStyle.mShowMidLine = true;
    lOrderBookStyle.mIntensityScale = 1.2f;

    RLOrderBookVis lOrderBook(getChartBounds(2, 0), 60, 40);
    lOrderBook.setStyle(lOrderBookStyle);
    lOrderBook.setPriceMode(RLOrderBookPriceMode::SpreadTicks);
    lOrderBook.setSpreadTicks(20);

    // Initialize order book with some data
    float lMidPrice = 100.0f;
    for (int i = 0; i < 40; ++i) {
        RLOrderBookSnapshot lSnap;
        float lBestBid = lMidPrice - 0.05f + randFloat(-0.02f, 0.02f);
        float lBestAsk = lMidPrice + 0.05f + randFloat(-0.02f, 0.02f);

        for (int j = 0; j < 25; ++j) {
            float lDecay = expf(-(float)j * 0.15f);
            lSnap.mBids.push_back(std::make_pair(lBestBid - (float)j * 0.01f, randFloat(100.0f, 3000.0f) * lDecay));
            lSnap.mAsks.push_back(std::make_pair(lBestAsk + (float)j * 0.01f, randFloat(100.0f, 3000.0f) * lDecay));
        }
        lOrderBook.pushSnapshot(lSnap);
        lMidPrice += randFloat(-0.01f, 0.01f);
    }

    // ===== 10. TreeMap =====
    RLTreeMapStyle lTreeMapStyle;
    lTreeMapStyle.mBackground = Color{20, 22, 28, 255};
    lTreeMapStyle.mShowBackground = true;
    lTreeMapStyle.mPaddingOuter = 4.0f;
    lTreeMapStyle.mPaddingInner = 2.0f;
    lTreeMapStyle.mPaddingTop = 16.0f;
    lTreeMapStyle.mBorderThickness = 1.0f;
    lTreeMapStyle.mBorderColor = Color{40, 44, 52, 255};
    lTreeMapStyle.mCornerRadius = 3.0f;
    lTreeMapStyle.mShowInternalNodes = true;
    lTreeMapStyle.mInternalNodeColor = Color{30, 34, 42, 220};
    lTreeMapStyle.mShowInternalLabels = true;
    lTreeMapStyle.mShowLeafLabels = true;
    lTreeMapStyle.mMinNodeSize = 10.0f;
    lTreeMapStyle.mLabelFontSize = 10;
    lTreeMapStyle.mAutoLabelColor = true;
    lTreeMapStyle.mSmoothAnimate = true;
    lTreeMapStyle.mUseDepthColors = false;
    lTreeMapStyle.mLabelFont = lBaseFont;

    RLTreeMap lTreeMap(getChartBounds(2, 1), lTreeMapStyle);
    lTreeMap.setLayout(RLTreeMapLayout::SQUARIFIED);

    // Create sample tree hierarchy
    RLTreeNode lTreeRoot;
    lTreeRoot.mLabel = "Root";

    RLTreeNode lCategory1;
    lCategory1.mLabel = "Category A";
    lCategory1.mChildren.push_back({"Item 1", randFloat(30.0f, 80.0f), paletteColor(0), true, {}});
    lCategory1.mChildren.push_back({"Item 2", randFloat(20.0f, 60.0f), paletteColor(0), true, {}});
    lCategory1.mChildren.push_back({"Item 3", randFloat(15.0f, 40.0f), paletteColor(0), true, {}});
    lTreeRoot.mChildren.push_back(lCategory1);

    RLTreeNode lCategory2;
    lCategory2.mLabel = "Category B";
    lCategory2.mChildren.push_back({"Item 4", randFloat(50.0f, 100.0f), paletteColor(1), true, {}});
    lCategory2.mChildren.push_back({"Item 5", randFloat(25.0f, 55.0f), paletteColor(1), true, {}});
    lTreeRoot.mChildren.push_back(lCategory2);

    RLTreeNode lCategory3;
    lCategory3.mLabel = "Category C";
    lCategory3.mChildren.push_back({"Item 6", randFloat(40.0f, 90.0f), paletteColor(2), true, {}});
    lCategory3.mChildren.push_back({"Item 7", randFloat(20.0f, 45.0f), paletteColor(2), true, {}});
    lCategory3.mChildren.push_back({"Item 8", randFloat(10.0f, 30.0f), paletteColor(2), true, {}});
    lCategory3.mChildren.push_back({"Item 9", randFloat(5.0f, 20.0f), paletteColor(2), true, {}});
    lTreeRoot.mChildren.push_back(lCategory3);

    lTreeMap.setData(lTreeRoot);

    // ===== 11. Time Series =====
    RLTimeSeriesChartStyle lTSStyle;
    lTSStyle.mBackground = Color{20, 22, 28, 255};
    lTSStyle.mShowGrid = true;
    lTSStyle.mAutoScaleY = true;
    lTSStyle.mSmoothScale = true;

    RLTimeSeries lTimeSeries(getChartBounds(2, 2), 200);
    lTimeSeries.setStyle(lTSStyle);

    RLTimeSeriesTraceStyle lTSTraceStyle;
    lTSTraceStyle.mColor = Color{80, 200, 255, 255};
    lTSTraceStyle.mLineThickness = 2.0f;
    lTSTraceStyle.mLineMode = RLTimeSeriesLineMode::Spline;
    size_t lTSTrace1 = lTimeSeries.addTrace(lTSTraceStyle);

    lTSTraceStyle.mColor = Color{255, 150, 80, 255};
    size_t lTSTrace2 = lTimeSeries.addTrace(lTSTraceStyle);

    // Pre-populate with some data
    for (int i = 0; i < 100; ++i) {
        float lT = (float)i * 0.05f;
        lTimeSeries.pushSample(lTSTrace1, 0.5f * sinf(lT * 2.0f) + randFloat(-0.05f, 0.05f));
        lTimeSeries.pushSample(lTSTrace2, 0.4f * cosf(lT * 1.5f) + randFloat(-0.05f, 0.05f));
    }

    // ===== 12. Log Plot =====
    RLLogPlotStyle lLogStyle;
    lLogStyle.mBackground = Color{20, 22, 28, 255};
    lLogStyle.mShowGrid = true;
    lLogStyle.mAutoScaleX = true;
    lLogStyle.mAutoScaleY = true;
    lLogStyle.mSmoothAnimate = true;

    RLLogPlot lLogPlot(getChartBounds(2, 3));
    lLogPlot.setLogPlotStyle(lLogStyle);
    lLogPlot.setTimeSeriesHeight(0.0f);  // No time series portion

    // Add a log-log trace (e.g., power law)
    RLLogPlotTrace lLogTrace;
    for (int i = 1; i <= 20; ++i) {
        float lX = (float)i;
        float lY = 10.0f / sqrtf(lX) + randFloat(-0.5f, 0.5f);  // 1/sqrt(x) decay
        lLogTrace.mXValues.push_back(lX);
        lLogTrace.mYValues.push_back(lY);
    }
    lLogTrace.mStyle.mLineColor = Color{150, 100, 255, 255};
    lLogTrace.mStyle.mLineThickness = 2.5f;
    lLogTrace.mStyle.mShowPoints = true;
    lLogPlot.addTrace(lLogTrace);

    // ===== 13. Area Chart =====
    RLAreaChartStyle lAreaStyle;
    lAreaStyle.mBackground = Color{20, 22, 28, 255};
    lAreaStyle.mShowGrid = true;
    lAreaStyle.mGridLines = 4;
    lAreaStyle.mPadding = 35.0f;
    lAreaStyle.mLineThickness = 1.5f;
    lAreaStyle.mShowLegend = false;
    lAreaStyle.mSmoothAnimate = true;
    lAreaStyle.mAnimateSpeed = 5.0f;

    RLAreaChart lAreaChart(getChartBounds(3, 0), RLAreaChartMode::STACKED, lAreaStyle);

    std::vector<RLAreaSeries> lAreaData;
    for (int s = 0; s < 3; ++s) {
        RLAreaSeries lSeries;
        lSeries.mColor = paletteColor(s);
        lSeries.mAlpha = 0.7f;
        for (int i = 0; i < 10; ++i) {
            lSeries.mValues.push_back(randFloat(15.0f, 40.0f));
        }
        lAreaData.push_back(lSeries);
    }
    lAreaChart.setData(lAreaData);

    // ===== 14. Radar Chart =====
    RLRadarChartStyle lRadarStyle;
    lRadarStyle.mShowBackground = true;
    lRadarStyle.mBackground = Color{20, 22, 28, 255};
    lRadarStyle.mShowGrid = true;
    lRadarStyle.mGridRings = 4;
    lRadarStyle.mGridColor = Color{45, 50, 60, 255};
    lRadarStyle.mShowAxes = true;
    lRadarStyle.mAxisColor = Color{55, 60, 70, 255};
    lRadarStyle.mShowLabels = true;
    lRadarStyle.mLabelColor = Color{160, 170, 190, 255};
    lRadarStyle.mLabelFont = lBaseFont;
    lRadarStyle.mLabelFontSize = 10;
    lRadarStyle.mLabelOffset = 8.0f;
    lRadarStyle.mShowLegend = true;
    lRadarStyle.mPadding = 45.0f;
    lRadarStyle.mSmoothAnimate = true;
    lRadarStyle.mAnimateSpeed = 5.0f;

    RLRadarChart lRadarChart(getChartBounds(3, 2), lRadarStyle);

    // Set up axes (6 dimensions)
    std::vector<std::string> lRadarLabels = {"Speed", "Power", "Defense", "Magic", "Stamina", "Luck"};
    lRadarChart.setAxes(lRadarLabels, 0.0f, 100.0f);

    // Add two series
    RLRadarSeries lRadarSeries1;
    lRadarSeries1.mLabel = "Player 1";
    lRadarSeries1.mValues = {75.0f, 85.0f, 60.0f, 40.0f, 70.0f, 55.0f};
    lRadarSeries1.mLineColor = paletteColor(0);
    lRadarSeries1.mFillColor = Color{paletteColor(0).r, paletteColor(0).g, paletteColor(0).b, 50};
    lRadarSeries1.mLineThickness = 2.0f;
    lRadarSeries1.mShowFill = true;
    lRadarSeries1.mShowMarkers = true;
    lRadarChart.addSeries(lRadarSeries1);

    RLRadarSeries lRadarSeries2;
    lRadarSeries2.mLabel = "Player 2";
    lRadarSeries2.mValues = {55.0f, 65.0f, 90.0f, 80.0f, 50.0f, 70.0f};
    lRadarSeries2.mLineColor = paletteColor(1);
    lRadarSeries2.mFillColor = Color{paletteColor(1).r, paletteColor(1).g, paletteColor(1).b, 50};
    lRadarSeries2.mLineThickness = 2.0f;
    lRadarSeries2.mShowFill = true;
    lRadarSeries2.mShowMarkers = true;
    lRadarChart.addSeries(lRadarSeries2);

    // ===== 15. Sankey Diagram =====
    RLSankeyStyle lSankeyStyle;
    lSankeyStyle.mShowBackground = true;
    lSankeyStyle.mBackground = Color{20, 22, 28, 255};
    lSankeyStyle.mNodeWidth = 12.0f;
    lSankeyStyle.mNodePadding = 6.0f;
    lSankeyStyle.mNodeCornerRadius = 2.0f;
    lSankeyStyle.mShowNodeBorder = true;
    lSankeyStyle.mNodeBorderColor = Color{255, 255, 255, 30};
    lSankeyStyle.mMinLinkThickness = 1.5f;
    lSankeyStyle.mLinkAlpha = 0.5f;
    lSankeyStyle.mLinkSegments = 20;
    lSankeyStyle.mLinkColorMode = RLSankeyLinkColorMode::GRADIENT;
    lSankeyStyle.mShowLabels = false;  // Small chart, skip labels
    lSankeyStyle.mPadding = 20.0f;
    lSankeyStyle.mSmoothAnimate = true;
    lSankeyStyle.mAnimateSpeed = 5.0f;

    RLSankey lSankey(getChartBounds(3, 1), lSankeyStyle);

    // Create a simple flow: 3 sources -> 2 middle -> 2 targets
    lSankey.addNode("A", paletteColor(0), 0);
    lSankey.addNode("B", paletteColor(1), 0);
    lSankey.addNode("C", paletteColor(2), 0);
    lSankey.addNode("X", paletteColor(3), 1);
    lSankey.addNode("Y", paletteColor(4), 1);
    lSankey.addNode("P", paletteColor(5), 2);
    lSankey.addNode("Q", paletteColor(6), 2);

    // Links from sources to middle
    lSankey.addLink(0, 3, 30.0f);  // A -> X
    lSankey.addLink(0, 4, 20.0f);  // A -> Y
    lSankey.addLink(1, 3, 25.0f);  // B -> X
    lSankey.addLink(1, 4, 15.0f);  // B -> Y
    lSankey.addLink(2, 4, 35.0f);  // C -> Y

    // Links from middle to targets
    lSankey.addLink(3, 5, 40.0f);  // X -> P
    lSankey.addLink(3, 6, 15.0f);  // X -> Q
    lSankey.addLink(4, 5, 30.0f);  // Y -> P
    lSankey.addLink(4, 6, 40.0f);  // Y -> Q

    // ===== 16. 3D Heat Map =====
    // Create a render texture for the 3D heat map (to display in 2D grid)
    Rectangle lHeatMap3DBounds = getChartBounds(3, 3);
    RenderTexture2D lHeatMap3DRT = LoadRenderTexture((int)lHeatMap3DBounds.width, (int)lHeatMap3DBounds.height);

    // Create 3D camera for the heat map
    Camera3D lHeatMap3DCamera = {};
    lHeatMap3DCamera.position = Vector3{1.5f, 1.2f, 1.5f};
    lHeatMap3DCamera.target = Vector3{0.0f, 0.3f, 0.0f};
    lHeatMap3DCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    lHeatMap3DCamera.fovy = 45.0f;
    lHeatMap3DCamera.projection = CAMERA_PERSPECTIVE;

    // Create 3D heat map with scientific plot style
    RLHeatMap3D lHeatMap3D(24, 24);

    RLHeatMap3DStyle lHeatMap3DStyle;
    lHeatMap3DStyle.mMode = RLHeatMap3DMode::Surface;
    lHeatMap3DStyle.mSmoothingSpeed = 4.0f;
    lHeatMap3DStyle.mShowWireframe = true;
    lHeatMap3DStyle.mWireframeColor = Color{60, 60, 70, 150};
    lHeatMap3DStyle.mSurfaceOpacity = 0.9f;
    lHeatMap3DStyle.mShowAxisBox = true;
    lHeatMap3DStyle.mShowFloorGrid = true;
    lHeatMap3DStyle.mGridDivisions = 8;
    lHeatMap3D.setStyle(lHeatMap3DStyle);

    lHeatMap3D.setPalette(
        Color{30, 60, 180, 255},
        Color{0, 180, 200, 255},
        Color{100, 220, 100, 255},
        Color{255, 180, 50, 255}
    );

    // Initialize with sine wave pattern
    std::vector<float> lHeatMap3DValues(24 * 24);
    for (int lY = 0; lY < 24; ++lY) {
        for (int lX = 0; lX < 24; ++lX) {
            float lNx = (float)lX / 24.0f;
            float lNy = (float)lY / 24.0f;
            lHeatMap3DValues[(size_t)(lY * 24 + lX)] = 0.5f + 0.3f * sinf(lNx * 6.28318f * 2.0f) * cosf(lNy * 6.28318f * 2.0f);
        }
    }
    lHeatMap3D.setValues(lHeatMap3DValues.data(), (int)lHeatMap3DValues.size());

    float lHeatMap3DRotation = 0.0f;

    // ===== 17. Linear Gauge =====
    RLLinearGaugeStyle lLinearGaugeStyle;
    lLinearGaugeStyle.mBackgroundColor = Color{24, 26, 32, 255};
    lLinearGaugeStyle.mTrackColor = Color{50, 55, 65, 255};
    lLinearGaugeStyle.mFillColor = Color{80, 200, 120, 255};
    lLinearGaugeStyle.mLabelFont = lBaseFont;
    lLinearGaugeStyle.mMajorTickCount = 5;
    lLinearGaugeStyle.mMinorTicksPerMajor = 1;
    lLinearGaugeStyle.mShowValueText = true;
    lLinearGaugeStyle.mValueDecimals = 0;
    lLinearGaugeStyle.mSmoothAnimate = true;
    lLinearGaugeStyle.mAnimateSpeed = 8.0f;
    lLinearGaugeStyle.mTrackThickness = 18.0f;

    RLLinearGauge lLinearGauge(getChartBounds(0, 4), 0.0f, 100.0f,
                                RLLinearGaugeOrientation::VERTICAL, lLinearGaugeStyle);
    lLinearGauge.setLabel("Level");
    lLinearGauge.setUnit("%");
    lLinearGauge.setValue(45.0f);

    // Add colored range bands
    std::vector<RLLinearGaugeRangeBand> lLinearGaugeRanges = {
        {0.0f, 30.0f, Color{255, 80, 80, 255}},    // Red: Low
        {30.0f, 70.0f, Color{255, 200, 80, 255}},  // Yellow: Medium
        {70.0f, 100.0f, Color{80, 200, 120, 255}}  // Green: High
    };
    lLinearGauge.setRanges(lLinearGaugeRanges);
    lLinearGauge.setTargetMarker(75.0f);

    float lLinearGaugeTarget = 45.0f;

    // Animation variables
    float lTime = 0.0f;
    float lGaugeTargetValue = 65.0f;
    float lOrderBookTimer = 0.0f;

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Animate gauges smoothly
        if ((int)lTime % 3 == 0 && (int)(lTime * 10.0f) % 30 == 0) {
            lGaugeTargetValue = randFloat(20.0f, 95.0f);
            lLinearGaugeTarget = randFloat(10.0f, 95.0f);
        }
        lGauge.setTargetValue(lGaugeTargetValue);
        lLinearGauge.setTargetValue(lLinearGaugeTarget);

        // Animate order book - push new snapshots periodically
        lOrderBookTimer += lDt;
        if (lOrderBookTimer > 0.1f) {
            lOrderBookTimer = 0.0f;
            RLOrderBookSnapshot lSnap;
            float lBestBid = lMidPrice - 0.05f + randFloat(-0.02f, 0.02f);
            float lBestAsk = lMidPrice + 0.05f + randFloat(-0.02f, 0.02f);

            for (int j = 0; j < 25; ++j) {
                float lDecay = expf(-(float)j * 0.15f);
                lSnap.mBids.push_back(std::make_pair(lBestBid - (float)j * 0.01f, randFloat(100.0f, 3000.0f) * lDecay));
                lSnap.mAsks.push_back(std::make_pair(lBestAsk + (float)j * 0.01f, randFloat(100.0f, 3000.0f) * lDecay));
            }
            lOrderBook.pushSnapshot(lSnap);
            lMidPrice += randFloat(-0.02f, 0.02f);
        }

        // Update all charts
        lBarChart.update(lDt);
        lBubble.update(lDt);
        lCandlestick.update(lDt);
        lGauge.update(lDt);
        lHeatMap.update(lDt);
        lPieChart.update(lDt);
        lScatterPlot.update(lDt);
        lBarChart2.update(lDt);
        lOrderBook.update(lDt);
        lTreeMap.update(lDt);
        lTimeSeries.update(lDt);
        lLogPlot.update(lDt);
        lAreaChart.update(lDt);
        lRadarChart.update(lDt);
        lSankey.update(lDt);
        lLinearGauge.update(lDt);

        // Update 3D heat map with animated data
        lHeatMap3DRotation += lDt * 0.5f;
        for (int lY = 0; lY < 24; ++lY) {
            for (int lX = 0; lX < 24; ++lX) {
                float lNx = (float)lX / 24.0f;
                float lNy = (float)lY / 24.0f;
                float lWave1 = sinf(lNx * 6.28318f * 2.0f + lTime * 2.0f) * 0.25f;
                float lWave2 = cosf(lNy * 6.28318f * 2.0f + lTime * 1.5f) * 0.25f;
                lHeatMap3DValues[(size_t)(lY * 24 + lX)] = 0.5f + lWave1 + lWave2;
            }
        }
        lHeatMap3D.setValues(lHeatMap3DValues.data(), (int)lHeatMap3DValues.size());
        lHeatMap3D.update(lDt);

        // Update 3D camera rotation for heat map (orbit around the plot)
        float lCamDist = 2.5f;
        lHeatMap3DCamera.position.x = sinf(lHeatMap3DRotation) * cosf(0.5f) * lCamDist;
        lHeatMap3DCamera.position.y = sinf(0.5f) * lCamDist;
        lHeatMap3DCamera.position.z = cosf(lHeatMap3DRotation) * cosf(0.5f) * lCamDist;

        // Animate time series with new samples
        float lTSTime = lTime * 2.0f;
        lTimeSeries.pushSample(lTSTrace1, 0.5f * sinf(lTSTime * 2.0f) + randFloat(-0.05f, 0.05f));
        lTimeSeries.pushSample(lTSTrace2, 0.4f * cosf(lTSTime * 1.5f) + randFloat(-0.05f, 0.05f));

        // Draw
        BeginDrawing();
        ClearBackground(Color{15, 17, 20, 255});

        // Render 3D heat map to texture
        BeginTextureMode(lHeatMap3DRT);
        ClearBackground(Color{25, 28, 35, 255});
        BeginMode3D(lHeatMap3DCamera);
        lHeatMap3D.draw(Vector3{0.0f, 0.0f, 0.0f}, 1.0f, lHeatMap3DCamera);
        EndMode3D();
        EndTextureMode();

        // Draw title
        DrawText("RayLib Charts - All Chart Types (Testing Static Conflicts)",
                 10, 5, 20, Color{200, 200, 210, 255});

        // Draw all charts
        lBarChart.draw();
        lBubble.draw();
        lCandlestick.draw();
        lGauge.draw();
        lHeatMap.draw();
        lPieChart.draw();
        lScatterPlot.draw();
        lBarChart2.draw();
        lOrderBook.draw2D();
        lTreeMap.draw();
        lTimeSeries.draw();
        lLogPlot.draw();
        lAreaChart.draw();
        lRadarChart.draw();
        lSankey.draw();
        lLinearGauge.draw();

        // Draw 3D heat map render texture (flipped vertically because render textures are inverted)
        DrawTextureRec(lHeatMap3DRT.texture,
                       Rectangle{0, 0, (float)lHeatMap3DRT.texture.width, -(float)lHeatMap3DRT.texture.height},
                       Vector2{lHeatMap3DBounds.x, lHeatMap3DBounds.y}, WHITE);

        // Draw labels for each chart (5x4 grid, 17 charts)
        const char* lLabels[] = {
            "Bar Chart", "Bubble Chart", "Candlestick", "Gauge", "Linear Gauge",
            "Heat Map", "Pie Chart", "Scatter Plot", "Bar Chart H", "",
            "Order Book", "TreeMap", "Time Series", "Log Plot", "",
            "Area Chart", "Sankey", "Radar Chart", "3D Heat Map", ""
        };

        for (int lRow = 0; lRow < 4; ++lRow) {
            for (int lCol = 0; lCol < 5; ++lCol) {
                int lIndex = lRow * 5 + lCol;
                if (lIndex < 20 && lLabels[lIndex][0] != '\0') {
                    Rectangle lBounds = getChartBounds(lRow, lCol);
                    DrawText(lLabels[lIndex], (int)lBounds.x + 5, (int)lBounds.y - 16,
                             14, Color{180, 180, 190, 255});
                }
            }
        }

        DrawFPS(SCREEN_WIDTH - 100, 5);

        EndDrawing();
    }

    UnloadFont(lBaseFont);
    UnloadRenderTexture(lHeatMap3DRT);
    CloseWindow();
    return 0;
}

