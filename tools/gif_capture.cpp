// gif_capture.cpp
// Automated frame capture for generating animated GIFs of chart demos.
// Usage: ./gif_capture <chart_name> <output_dir>
// Renders the chart for 240 frames (4s at 60fps), saving every 4th frame as PNG.

#include "raylib.h"
#include "RLAreaChart.h"
#include "RLLinearGauge.h"
#include "RLRadarChart.h"
#include "RLSankey.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <sys/stat.h>

static const int SCREEN_WIDTH = 1400;
static const int SCREEN_HEIGHT = 800;
static const int TOTAL_FRAMES = 240;
static const int CAPTURE_EVERY = 4;
static const float FIXED_DT = 1.0f / 60.0f;

static float randFloat(float aMin, float aMax) {
    return aMin + ((float)rand() / (float)RAND_MAX) * (aMax - aMin);
}

// ============================================================================
// Area Chart capture
// ============================================================================

static void captureAreaChart(const std::string& rOutputDir) {
    static const int NUM_POINTS = 12;
    static const int NUM_SERIES = 4;

    static const Color SERIES_COLORS[] = {
        {80, 180, 255, 255},
        {255, 120, 80, 255},
        {120, 220, 120, 255},
        {220, 100, 220, 255}
    };
    static const char* SERIES_LABELS[] = {"Series A", "Series B", "Series C", "Series D"};
    static const char* X_LABELS[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    float lMargin = 20.0f;
    float lGap = 15.0f;
    float lChartWidth = (SCREEN_WIDTH - 2.0f * lMargin - 2.0f * lGap) / 3.0f;
    float lChartHeight = SCREEN_HEIGHT - 2.0f * lMargin - 60.0f;
    float lChartY = lMargin + 50.0f;

    Rectangle lBounds1 = {lMargin, lChartY, lChartWidth, lChartHeight};
    Rectangle lBounds2 = {lMargin + lChartWidth + lGap, lChartY, lChartWidth, lChartHeight};
    Rectangle lBounds3 = {lMargin + 2.0f * (lChartWidth + lGap), lChartY, lChartWidth, lChartHeight};

    RLAreaChartStyle lStyle;
    lStyle.mShowBackground = true;
    lStyle.mBackground = Color{20, 24, 32, 255};
    lStyle.mShowGrid = true;
    lStyle.mGridColor = Color{40, 48, 60, 255};
    lStyle.mGridLines = 5;
    lStyle.mAxisColor = Color{100, 110, 130, 255};
    lStyle.mLabelColor = Color{180, 190, 210, 255};
    lStyle.mPadding = 50.0f;
    lStyle.mLineThickness = 2.0f;
    lStyle.mShowPoints = false;
    lStyle.mShowLabels = true;
    lStyle.mLabelFont = lFont;
    lStyle.mLabelFontSize = 11;
    lStyle.mShowLegend = true;
    lStyle.mSmoothAnimate = true;
    lStyle.mAnimateSpeed = 5.0f;

    RLAreaChart lChartOverlapped(lBounds1, RLAreaChartMode::OVERLAPPED, lStyle);
    RLAreaChart lChartStacked(lBounds2, RLAreaChartMode::STACKED, lStyle);
    RLAreaChart lChartPercent(lBounds3, RLAreaChartMode::PERCENT, lStyle);

    std::vector<std::string> lXLabels;
    for (int i = 0; i < NUM_POINTS; ++i) {
        lXLabels.push_back(X_LABELS[i]);
    }
    lChartOverlapped.setXLabels(lXLabels);
    lChartStacked.setXLabels(lXLabels);
    lChartPercent.setXLabels(lXLabels);

    // Generate initial data
    auto generateData = [&](float aTime) {
        std::vector<RLAreaSeries> lData;
        for (int s = 0; s < NUM_SERIES; ++s) {
            RLAreaSeries lSeries;
            lSeries.mColor = SERIES_COLORS[s];
            lSeries.mLabel = SERIES_LABELS[s];
            lSeries.mAlpha = 0.7f;
            for (int i = 0; i < NUM_POINTS; ++i) {
                float lPhase = (float)i / (float)NUM_POINTS * 2.0f * PI;
                float lWave = sinf(lPhase + aTime * 0.5f + (float)s * 0.8f);
                float lBase = 20.0f + (float)s * 15.0f;
                float lValue = lBase + lWave * 15.0f + randFloat(-5.0f, 5.0f);
                lSeries.mValues.push_back(fmaxf(5.0f, lValue));
            }
            lData.push_back(lSeries);
        }
        return lData;
    };

    auto lInitialData = generateData(0.0f);
    auto lOverlappedData = lInitialData;
    for (auto& rSeries : lOverlappedData) {
        rSeries.mAlpha = 0.5f;
    }
    lChartOverlapped.setData(lOverlappedData);
    lChartStacked.setData(lInitialData);
    lChartPercent.setData(lInitialData);

    float lTime = 0.0f;
    float lUpdateTimer = 0.0f;
    int lFramesSaved = 0;

    for (int lFrame = 0; lFrame < TOTAL_FRAMES; ++lFrame) {
        lTime += FIXED_DT;
        lUpdateTimer += FIXED_DT;

        if (lUpdateTimer >= 1.5f) {
            lUpdateTimer = 0.0f;
            auto lNewData = generateData(lTime);
            auto lNewOverlapped = lNewData;
            for (auto& rSeries : lNewOverlapped) {
                rSeries.mAlpha = 0.5f;
            }
            lChartOverlapped.setTargetData(lNewOverlapped);
            lChartStacked.setTargetData(lNewData);
            lChartPercent.setTargetData(lNewData);
        }

        lChartOverlapped.update(FIXED_DT);
        lChartStacked.update(FIXED_DT);
        lChartPercent.update(FIXED_DT);

        BeginDrawing();
        ClearBackground(Color{12, 14, 18, 255});

        const char* lTitle = "RLAreaChart - Three Visualization Modes";
        float lTitleWidth = MeasureTextEx(lFont, lTitle, 24, 1.0f).x;
        DrawTextEx(lFont, lTitle, Vector2{(float)(SCREEN_WIDTH - lTitleWidth) / 2.0f, 12}, 24, 1.0f, Color{220, 225, 235, 255});

        Color lLabelColor = Color{150, 160, 180, 255};
        const char* lLabel1 = "OVERLAPPED";
        float lL1W = MeasureTextEx(lFont, lLabel1, 16, 1.0f).x;
        DrawTextEx(lFont, lLabel1, Vector2{lBounds1.x + (lBounds1.width - lL1W) / 2.0f, lBounds1.y - 22.0f}, 16, 1.0f, lLabelColor);

        const char* lLabel2 = "STACKED";
        float lL2W = MeasureTextEx(lFont, lLabel2, 16, 1.0f).x;
        DrawTextEx(lFont, lLabel2, Vector2{lBounds2.x + (lBounds2.width - lL2W) / 2.0f, lBounds2.y - 22.0f}, 16, 1.0f, lLabelColor);

        const char* lLabel3 = "PERCENT (100%)";
        float lL3W = MeasureTextEx(lFont, lLabel3, 16, 1.0f).x;
        DrawTextEx(lFont, lLabel3, Vector2{lBounds3.x + (lBounds3.width - lL3W) / 2.0f, lBounds3.y - 22.0f}, 16, 1.0f, lLabelColor);

        lChartOverlapped.draw();
        lChartStacked.draw();
        lChartPercent.draw();
        EndDrawing();

        if (lFrame % CAPTURE_EVERY == 0) {
            Image lImg = LoadImageFromScreen();
            char lPath[512];
            snprintf(lPath, sizeof(lPath), "%s/frame_%04d.png", rOutputDir.c_str(), lFramesSaved);
            ExportImage(lImg, lPath);
            UnloadImage(lImg);
            lFramesSaved++;
        }
    }

    UnloadFont(lFont);
    printf("AreaChart: saved %d frames\n", lFramesSaved);
}

// ============================================================================
// Linear Gauge capture
// ============================================================================

static void captureLinearGauge(const std::string& rOutputDir) {
    Font lFont = LoadFontEx("base.ttf", 24, nullptr, 250);

    float lMargin = 30.0f;
    float lGaugeHeight = 100.0f;
    float lGaugeWidth = (SCREEN_WIDTH - 4.0f * lMargin) / 3.0f;

    // Horizontal gauge 1: Temperature
    RLLinearGaugeStyle lTempStyle;
    lTempStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lTempStyle.mTrackColor = Color{50, 55, 65, 255};
    lTempStyle.mFillColor = Color{80, 200, 120, 255};
    lTempStyle.mLabelFont = lFont;
    lTempStyle.mMajorTickCount = 10;
    lTempStyle.mMinorTicksPerMajor = 1;
    lTempStyle.mShowValueText = true;
    lTempStyle.mValueDecimals = 1;
    lTempStyle.mSmoothAnimate = true;
    lTempStyle.mAnimateSpeed = 8.0f;

    Rectangle lTempBounds = {lMargin, lMargin, lGaugeWidth, lGaugeHeight};
    RLLinearGauge lTempGauge(lTempBounds, 0.0f, 100.0f, RLLinearGaugeOrientation::HORIZONTAL, lTempStyle);
    lTempGauge.setLabel("Temperature");
    lTempGauge.setUnit("\xC2\xB0" "C");
    lTempGauge.setValue(45.0f);
    std::vector<RLLinearGaugeRangeBand> lTempRanges = {
        {0.0f, 60.0f, Color{80, 200, 120, 255}},
        {60.0f, 80.0f, Color{255, 200, 80, 255}},
        {80.0f, 100.0f, Color{255, 80, 80, 255}}
    };
    lTempGauge.setRanges(lTempRanges);
    lTempGauge.setTargetMarker(75.0f);

    // Horizontal gauge 2: CPU
    RLLinearGaugeStyle lCpuStyle;
    lCpuStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lCpuStyle.mTrackColor = Color{50, 55, 65, 255};
    lCpuStyle.mPointerColor = Color{255, 100, 100, 255};
    lCpuStyle.mLabelFont = lFont;
    lCpuStyle.mMajorTickCount = 10;
    lCpuStyle.mMinorTicksPerMajor = 4;
    lCpuStyle.mShowValueText = true;
    lCpuStyle.mValueDecimals = 0;
    lCpuStyle.mSmoothAnimate = true;
    lCpuStyle.mAnimateSpeed = 12.0f;

    Rectangle lCpuBounds = {lMargin + lGaugeWidth + lMargin, lMargin, lGaugeWidth, lGaugeHeight};
    RLLinearGauge lCpuGauge(lCpuBounds, 0.0f, 100.0f, RLLinearGaugeOrientation::HORIZONTAL, lCpuStyle);
    lCpuGauge.setPointerStyle(RLLinearGaugePointerStyle::TRIANGLE);
    lCpuGauge.setLabel("CPU Load");
    lCpuGauge.setUnit("%");
    lCpuGauge.setValue(35.0f);
    std::vector<RLLinearGaugeRangeBand> lCpuRanges = {
        {0.0f, 50.0f, Color{80, 180, 255, 255}},
        {50.0f, 80.0f, Color{255, 180, 80, 255}},
        {80.0f, 100.0f, Color{255, 80, 100, 255}}
    };
    lCpuGauge.setRanges(lCpuRanges);

    // Horizontal gauge 3: Progress
    RLLinearGaugeStyle lProgressStyle;
    lProgressStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lProgressStyle.mTrackColor = Color{50, 55, 65, 255};
    lProgressStyle.mPointerColor = Color{255, 220, 80, 255};
    lProgressStyle.mFillColor = Color{100, 180, 255, 255};
    lProgressStyle.mLabelFont = lFont;
    lProgressStyle.mMajorTickCount = 5;
    lProgressStyle.mMinorTicksPerMajor = 3;
    lProgressStyle.mShowValueText = true;
    lProgressStyle.mValueDecimals = 0;
    lProgressStyle.mSmoothAnimate = true;
    lProgressStyle.mAnimateSpeed = 6.0f;

    Rectangle lProgressBounds = {lMargin + 2.0f * (lGaugeWidth + lMargin), lMargin, lGaugeWidth, lGaugeHeight};
    RLLinearGauge lProgressGauge(lProgressBounds, 0.0f, 1000.0f, RLLinearGaugeOrientation::HORIZONTAL, lProgressStyle);
    lProgressGauge.setPointerStyle(RLLinearGaugePointerStyle::LINE_MARKER);
    lProgressGauge.setLabel("Download Progress");
    lProgressGauge.setUnit("MB");
    lProgressGauge.setValue(250.0f);
    lProgressGauge.setTargetMarker(800.0f);

    // Vertical gauges
    float lVerticalTop = lMargin + lGaugeHeight + 40.0f;
    float lVerticalHeight = SCREEN_HEIGHT - lVerticalTop - lMargin - 60.0f;
    float lVerticalWidth = 120.0f;
    float lVerticalSpacing = 40.0f;

    RLLinearGaugeStyle lPressureStyle;
    lPressureStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lPressureStyle.mTrackColor = Color{50, 55, 65, 255};
    lPressureStyle.mFillColor = Color{120, 200, 255, 255};
    lPressureStyle.mLabelFont = lFont;
    lPressureStyle.mMajorTickCount = 8;
    lPressureStyle.mMinorTicksPerMajor = 1;
    lPressureStyle.mShowValueText = true;
    lPressureStyle.mValueDecimals = 0;
    lPressureStyle.mTrackThickness = 32.0f;
    lPressureStyle.mSmoothAnimate = true;
    lPressureStyle.mAnimateSpeed = 5.0f;

    Rectangle lPressureBounds = {lMargin, lVerticalTop, lVerticalWidth, lVerticalHeight};
    RLLinearGauge lPressureGauge(lPressureBounds, 0.0f, 200.0f, RLLinearGaugeOrientation::VERTICAL, lPressureStyle);
    lPressureGauge.setLabel("Pressure");
    lPressureGauge.setUnit("PSI");
    lPressureGauge.setValue(80.0f);
    std::vector<RLLinearGaugeRangeBand> lPressureRanges = {
        {0.0f, 100.0f, Color{80, 200, 160, 255}},
        {100.0f, 150.0f, Color{255, 200, 80, 255}},
        {150.0f, 200.0f, Color{255, 80, 80, 255}}
    };
    lPressureGauge.setRanges(lPressureRanges);

    RLLinearGaugeStyle lVolumeStyle;
    lVolumeStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lVolumeStyle.mTrackColor = Color{50, 55, 65, 255};
    lVolumeStyle.mPointerColor = Color{255, 120, 180, 255};
    lVolumeStyle.mLabelFont = lFont;
    lVolumeStyle.mMajorTickCount = 10;
    lVolumeStyle.mMinorTicksPerMajor = 0;
    lVolumeStyle.mShowValueText = true;
    lVolumeStyle.mValueDecimals = 0;
    lVolumeStyle.mTrackThickness = 28.0f;
    lVolumeStyle.mSmoothAnimate = true;
    lVolumeStyle.mAnimateSpeed = 15.0f;

    Rectangle lVolumeBounds = {lMargin + lVerticalWidth + lVerticalSpacing, lVerticalTop, lVerticalWidth, lVerticalHeight};
    RLLinearGauge lVolumeGauge(lVolumeBounds, 0.0f, 100.0f, RLLinearGaugeOrientation::VERTICAL, lVolumeStyle);
    lVolumeGauge.setPointerStyle(RLLinearGaugePointerStyle::TRIANGLE);
    lVolumeGauge.setLabel("Volume");
    lVolumeGauge.setUnit("%");
    lVolumeGauge.setValue(70.0f);

    RLLinearGaugeStyle lFuelStyle;
    lFuelStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lFuelStyle.mTrackColor = Color{50, 55, 65, 255};
    lFuelStyle.mFillColor = Color{255, 180, 80, 255};
    lFuelStyle.mLabelFont = lFont;
    lFuelStyle.mMajorTickCount = 4;
    lFuelStyle.mMinorTicksPerMajor = 3;
    lFuelStyle.mShowValueText = true;
    lFuelStyle.mValueDecimals = 0;
    lFuelStyle.mTrackThickness = 36.0f;
    lFuelStyle.mSmoothAnimate = true;
    lFuelStyle.mAnimateSpeed = 4.0f;

    Rectangle lFuelBounds = {lMargin + 2.0f * (lVerticalWidth + lVerticalSpacing), lVerticalTop, lVerticalWidth, lVerticalHeight};
    RLLinearGauge lFuelGauge(lFuelBounds, 0.0f, 100.0f, RLLinearGaugeOrientation::VERTICAL, lFuelStyle);
    lFuelGauge.setLabel("Fuel Level");
    lFuelGauge.setUnit("L");
    lFuelGauge.setValue(65.0f);
    std::vector<RLLinearGaugeRangeBand> lFuelRanges = {
        {0.0f, 20.0f, Color{255, 80, 80, 255}},
        {20.0f, 50.0f, Color{255, 200, 80, 255}},
        {50.0f, 100.0f, Color{80, 200, 120, 255}}
    };
    lFuelGauge.setRanges(lFuelRanges);
    lFuelGauge.setTargetMarker(25.0f);

    RLLinearGaugeStyle lSpeedStyle;
    lSpeedStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lSpeedStyle.mTrackColor = Color{50, 55, 65, 255};
    lSpeedStyle.mPointerColor = Color{80, 255, 180, 255};
    lSpeedStyle.mLabelFont = lFont;
    lSpeedStyle.mMajorTickCount = 6;
    lSpeedStyle.mMinorTicksPerMajor = 4;
    lSpeedStyle.mShowValueText = true;
    lSpeedStyle.mValueDecimals = 0;
    lSpeedStyle.mTrackThickness = 28.0f;
    lSpeedStyle.mSmoothAnimate = true;
    lSpeedStyle.mAnimateSpeed = 10.0f;

    Rectangle lSpeedBounds = {lMargin + 3.0f * (lVerticalWidth + lVerticalSpacing), lVerticalTop, lVerticalWidth, lVerticalHeight};
    RLLinearGauge lSpeedGauge(lSpeedBounds, 0.0f, 240.0f, RLLinearGaugeOrientation::VERTICAL, lSpeedStyle);
    lSpeedGauge.setPointerStyle(RLLinearGaugePointerStyle::LINE_MARKER);
    lSpeedGauge.setLabel("Speed");
    lSpeedGauge.setUnit("km/h");
    lSpeedGauge.setValue(60.0f);
    lSpeedGauge.setTargetMarker(120.0f);

    float lUpdateTimer = 0.0f;
    float lSinTime = 0.0f;
    int lFramesSaved = 0;

    for (int lFrame = 0; lFrame < TOTAL_FRAMES; ++lFrame) {
        lSinTime += FIXED_DT;
        lUpdateTimer += FIXED_DT;

        if (lUpdateTimer >= 1.5f) {
            lUpdateTimer = 0.0f;
            lTempGauge.setTargetValue(50.0f + sinf(lSinTime * 0.3f) * 40.0f + randFloat(-5.0f, 5.0f));
            lCpuGauge.setTargetValue(randFloat(10.0f, 95.0f));
            float lNewProgress = lProgressGauge.getValue() + randFloat(50.0f, 150.0f);
            if (lNewProgress > 1000.0f) lNewProgress = randFloat(0.0f, 200.0f);
            lProgressGauge.setTargetValue(lNewProgress);
            lPressureGauge.setTargetValue(100.0f + sinf(lSinTime * 0.5f) * 80.0f + randFloat(-10.0f, 10.0f));
            lVolumeGauge.setTargetValue(randFloat(20.0f, 90.0f));
            float lNewFuel = lFuelGauge.getValue() - randFloat(5.0f, 15.0f);
            if (lNewFuel < 10.0f) lNewFuel = randFloat(70.0f, 100.0f);
            lFuelGauge.setTargetValue(lNewFuel);
            float lSpeedValue = 80.0f + sinf(lSinTime * 0.8f) * 60.0f + randFloat(-20.0f, 20.0f);
            lSpeedGauge.setTargetValue(std::max(0.0f, std::min(240.0f, lSpeedValue)));
        }

        lTempGauge.update(FIXED_DT);
        lCpuGauge.update(FIXED_DT);
        lProgressGauge.update(FIXED_DT);
        lPressureGauge.update(FIXED_DT);
        lVolumeGauge.update(FIXED_DT);
        lFuelGauge.update(FIXED_DT);
        lSpeedGauge.update(FIXED_DT);

        BeginDrawing();
        ClearBackground(Color{18, 20, 26, 255});

        lTempGauge.draw();
        lCpuGauge.draw();
        lProgressGauge.draw();
        lPressureGauge.draw();
        lVolumeGauge.draw();
        lFuelGauge.draw();
        lSpeedGauge.draw();

        // Info panel
        float lInfoX = lMargin + 4.0f * (lVerticalWidth + lVerticalSpacing) + 60.0f;
        if (lInfoX + 300.0f > SCREEN_WIDTH) lInfoX = SCREEN_WIDTH - 300.0f - lMargin;
        auto lHeaderColor = Color{220, 225, 235, 255};
        auto lTextColor = Color{180, 190, 210, 255};
        DrawTextEx(lFont, "RLLinearGauge Demo", Vector2{lInfoX, lVerticalTop}, 22, 1.0f, lHeaderColor);
        DrawTextEx(lFont, "Dashboard-style linear gauges with:", Vector2{lInfoX, lVerticalTop + 35.0f}, 16, 1.0f, lTextColor);
        DrawTextEx(lFont, "- Horizontal & Vertical orientations", Vector2{lInfoX + 10.0f, lVerticalTop + 63.0f}, 14, 1.0f, lTextColor);
        DrawTextEx(lFont, "- Colored range bands (zones)", Vector2{lInfoX + 10.0f, lVerticalTop + 85.0f}, 14, 1.0f, lTextColor);
        DrawTextEx(lFont, "- Multiple pointer styles", Vector2{lInfoX + 10.0f, lVerticalTop + 107.0f}, 14, 1.0f, lTextColor);
        DrawTextEx(lFont, "- Smooth value animations", Vector2{lInfoX + 10.0f, lVerticalTop + 129.0f}, 14, 1.0f, lTextColor);

        EndDrawing();

        if (lFrame % CAPTURE_EVERY == 0) {
            Image lImg = LoadImageFromScreen();
            char lPath[512];
            snprintf(lPath, sizeof(lPath), "%s/frame_%04d.png", rOutputDir.c_str(), lFramesSaved);
            ExportImage(lImg, lPath);
            UnloadImage(lImg);
            lFramesSaved++;
        }
    }

    UnloadFont(lFont);
    printf("LinearGauge: saved %d frames\n", lFramesSaved);
}

// ============================================================================
// Radar Chart capture
// ============================================================================

static Color withAlpha(Color aColor, unsigned char aAlpha) {
    return Color{aColor.r, aColor.g, aColor.b, aAlpha};
}

static void captureRadarChart(const std::string& rOutputDir) {
    static const Color SERIES_COLORS[] = {
        {80, 180, 255, 255},
        {255, 120, 80, 255},
        {120, 220, 120, 255},
        {220, 100, 220, 255},
        {255, 200, 80, 255}
    };
    static const char* SKILL_AXES[] = {"Strength", "Speed", "Intelligence", "Stamina", "Magic", "Defense"};
    static const char* PRODUCT_AXES[] = {"Performance", "Reliability", "Cost", "Features", "Support", "Ease of Use", "Security"};

    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    float lMargin = 25.0f;
    float lGap = 20.0f;
    float lChartWidth = (SCREEN_WIDTH - 2.0f * lMargin - lGap) / 2.0f;
    float lChartHeight = SCREEN_HEIGHT - 2.0f * lMargin - 80.0f;

    Rectangle lBounds1 = {lMargin, lMargin + 60.0f, lChartWidth, lChartHeight};
    Rectangle lBounds2 = {lMargin + lChartWidth + lGap, lMargin + 60.0f, lChartWidth, lChartHeight};

    RLRadarChartStyle lStyle1;
    lStyle1.mShowBackground = true;
    lStyle1.mBackground = Color{20, 24, 32, 255};
    lStyle1.mShowGrid = true;
    lStyle1.mGridRings = 5;
    lStyle1.mGridColor = Color{45, 50, 60, 255};
    lStyle1.mGridThickness = 1.0f;
    lStyle1.mShowAxes = true;
    lStyle1.mAxisColor = Color{60, 65, 75, 255};
    lStyle1.mAxisThickness = 1.5f;
    lStyle1.mShowLabels = true;
    lStyle1.mLabelColor = Color{180, 190, 210, 255};
    lStyle1.mLabelFont = lFont;
    lStyle1.mLabelFontSize = 14;
    lStyle1.mLabelOffset = 15.0f;
    lStyle1.mShowLegend = true;
    lStyle1.mPadding = 70.0f;
    lStyle1.mNormMode = RLRadarNormMode::GLOBAL;
    lStyle1.mSmoothAnimate = true;
    lStyle1.mAnimateSpeed = 5.0f;
    lStyle1.mFadeSpeed = 4.0f;

    RLRadarChart lChart1(lBounds1, lStyle1);
    std::vector<std::string> lSkillLabels;
    for (const char* pLabel : SKILL_AXES) lSkillLabels.push_back(pLabel);
    lChart1.setAxes(lSkillLabels, 0.0f, 100.0f);

    // Initial warrior profile
    float PROFILE_BASES[][6] = {
        {90.0f, 60.0f, 40.0f, 80.0f, 20.0f, 85.0f},
        {30.0f, 40.0f, 95.0f, 50.0f, 100.0f, 35.0f},
        {50.0f, 95.0f, 70.0f, 60.0f, 40.0f, 40.0f},
        {70.0f, 30.0f, 50.0f, 95.0f, 30.0f, 95.0f}
    };
    const char* PROFILE_NAMES[] = {"Warrior", "Mage", "Rogue", "Tank"};

    RLRadarSeries lSeries1;
    lSeries1.mLabel = PROFILE_NAMES[0];
    for (int i = 0; i < 6; ++i) lSeries1.mValues.push_back(PROFILE_BASES[0][i] + randFloat(-10.0f, 10.0f));
    lSeries1.mLineColor = SERIES_COLORS[0];
    lSeries1.mFillColor = withAlpha(SERIES_COLORS[0], 60);
    lSeries1.mLineThickness = 2.5f;
    lSeries1.mShowFill = true;
    lSeries1.mShowMarkers = true;
    lSeries1.mMarkerScale = 1.8f;
    lChart1.addSeries(lSeries1);

    // Chart 2: Multi-series
    RLRadarChartStyle lStyle2 = lStyle1;
    lStyle2.mBackground = Color{18, 22, 30, 255};
    lStyle2.mGridColor = Color{40, 45, 55, 255};
    RLRadarChart lChart2(lBounds2, lStyle2);

    std::vector<std::string> lProductLabels;
    for (const char* pLabel : PRODUCT_AXES) lProductLabels.push_back(pLabel);
    lChart2.setAxes(lProductLabels, 0.0f, 100.0f);

    const char* PRODUCT_NAMES[] = {"Product A", "Product B", "Product C"};
    float BASE_VALUES[] = {75.0f, 60.0f, 85.0f};
    for (int i = 0; i < 3; ++i) {
        RLRadarSeries lS;
        lS.mLabel = PRODUCT_NAMES[i];
        for (int j = 0; j < 7; ++j) {
            lS.mValues.push_back(fmaxf(10.0f, fminf(100.0f, BASE_VALUES[i] + randFloat(-20.0f, 20.0f))));
        }
        lS.mLineColor = SERIES_COLORS[i];
        lS.mFillColor = withAlpha(SERIES_COLORS[i], 40);
        lS.mLineThickness = 2.0f;
        lS.mShowFill = true;
        lS.mShowMarkers = true;
        lS.mMarkerScale = 1.5f;
        lChart2.addSeries(lS);
    }

    float lTimer = 0.0f;
    int lProfileIdx = 0;
    float lProfileSwitchTimer = 0.0f;
    int lFramesSaved = 0;

    for (int lFrame = 0; lFrame < TOTAL_FRAMES; ++lFrame) {
        lTimer += FIXED_DT;
        lProfileSwitchTimer += FIXED_DT;

        // Switch profile every 1.5 seconds for visible animation
        if (lProfileSwitchTimer >= 1.5f) {
            lProfileSwitchTimer = 0.0f;
            lProfileIdx = (lProfileIdx + 1) % 4;

            std::vector<float> lNewValues;
            for (int i = 0; i < 6; ++i) {
                lNewValues.push_back(PROFILE_BASES[lProfileIdx][i] + randFloat(-10.0f, 10.0f));
            }
            RLRadarSeries lNewSeries;
            lNewSeries.mLabel = PROFILE_NAMES[lProfileIdx];
            lNewSeries.mValues = lNewValues;
            lNewSeries.mLineColor = SERIES_COLORS[lProfileIdx];
            lNewSeries.mFillColor = withAlpha(SERIES_COLORS[lProfileIdx], 60);
            lNewSeries.mLineThickness = 2.5f;
            lNewSeries.mShowFill = true;
            lNewSeries.mShowMarkers = true;
            lNewSeries.mMarkerScale = 1.8f;
            lChart1.setSeriesData(0, lNewSeries);

            // Update chart 2 values
            for (size_t i = 0; i < lChart2.getSeriesCount(); ++i) {
                std::vector<float> lNewVals;
                for (int j = 0; j < 7; ++j) {
                    lNewVals.push_back(fmaxf(10.0f, fminf(100.0f, BASE_VALUES[i % 3] + randFloat(-20.0f, 20.0f))));
                }
                lChart2.setSeriesData(i, lNewVals);
            }
        }

        lChart1.update(FIXED_DT);
        lChart2.update(FIXED_DT);

        BeginDrawing();
        ClearBackground(Color{12, 14, 18, 255});

        const char* lTitle = "RLRadarChart Demo - Radar/Spider Charts";
        float lTitleWidth = MeasureTextEx(lFont, lTitle, 24, 1.0f).x;
        DrawTextEx(lFont, lTitle, Vector2{(float)(SCREEN_WIDTH - lTitleWidth) / 2.0f, 18}, 24, 1.0f, Color{200, 210, 230, 255});

        DrawTextEx(lFont, "Single Series - Character Profile",
                   Vector2{lBounds1.x + 10.0f, lBounds1.y - 28.0f}, 16.0f, 1.0f, Color{160, 170, 190, 255});
        DrawTextEx(lFont, "Multi-Series - Product Comparison",
                   Vector2{lBounds2.x + 10.0f, lBounds2.y - 28.0f}, 16.0f, 1.0f, Color{160, 170, 190, 255});

        lChart1.draw();
        lChart2.draw();
        EndDrawing();

        if (lFrame % CAPTURE_EVERY == 0) {
            Image lImg = LoadImageFromScreen();
            char lPath[512];
            snprintf(lPath, sizeof(lPath), "%s/frame_%04d.png", rOutputDir.c_str(), lFramesSaved);
            ExportImage(lImg, lPath);
            UnloadImage(lImg);
            lFramesSaved++;
        }
    }

    UnloadFont(lFont);
    printf("RadarChart: saved %d frames\n", lFramesSaved);
}

// ============================================================================
// Sankey Diagram capture
// ============================================================================

static Color getSankeyColor(int aIndex) {
    static const Color COLORS[] = {
        {66, 133, 244, 255}, {52, 168, 83, 255}, {251, 188, 4, 255},
        {234, 67, 53, 255}, {154, 99, 191, 255}, {0, 188, 212, 255},
        {255, 112, 67, 255}, {156, 204, 101, 255}, {121, 134, 203, 255},
        {255, 167, 38, 255}
    };
    return COLORS[aIndex % 10];
}

static void captureSankey(const std::string& rOutputDir) {
    Font lFont = LoadFontEx("base.ttf", 18, nullptr, 250);

    float lMargin = 25.0f;
    float lGap = 30.0f;
    float lChartWidth = (SCREEN_WIDTH - 2.0f * lMargin - lGap) / 2.0f;
    float lChartHeight = SCREEN_HEIGHT - 2.0f * lMargin - 80.0f;

    Rectangle lBounds1 = {lMargin, lMargin + 60.0f, lChartWidth, lChartHeight};
    Rectangle lBounds2 = {lMargin + lChartWidth + lGap, lMargin + 60.0f, lChartWidth, lChartHeight};

    RLSankeyStyle lStyle;
    lStyle.mShowBackground = true;
    lStyle.mBackground = Color{18, 22, 30, 255};
    lStyle.mNodeWidth = 18.0f;
    lStyle.mNodePadding = 8.0f;
    lStyle.mNodeCornerRadius = 3.0f;
    lStyle.mShowNodeBorder = true;
    lStyle.mNodeBorderColor = Color{255, 255, 255, 30};
    lStyle.mColumnSpacing = 120.0f;
    lStyle.mMinLinkThickness = 2.0f;
    lStyle.mLinkAlpha = 0.55f;
    lStyle.mLinkSegments = 32;
    lStyle.mLinkColorMode = RLSankeyLinkColorMode::GRADIENT;
    lStyle.mShowLabels = true;
    lStyle.mLabelColor = Color{200, 210, 225, 255};
    lStyle.mLabelFont = lFont;
    lStyle.mLabelFontSize = 13;
    lStyle.mLabelPadding = 6.0f;
    lStyle.mPadding = 50.0f;
    lStyle.mSmoothAnimate = true;
    lStyle.mAnimateSpeed = 4.0f;
    lStyle.mFadeSpeed = 3.0f;

    // Energy flow data
    std::vector<RLSankeyNode> lNodes1;
    lNodes1.push_back({"Coal", getSankeyColor(0), 0});
    lNodes1.push_back({"Natural Gas", getSankeyColor(1), 0});
    lNodes1.push_back({"Nuclear", getSankeyColor(2), 0});
    lNodes1.push_back({"Renewable", getSankeyColor(3), 0});
    lNodes1.push_back({"Power Plants", getSankeyColor(4), 1});
    lNodes1.push_back({"Direct Use", getSankeyColor(5), 1});
    lNodes1.push_back({"Grid", getSankeyColor(6), 2});
    lNodes1.push_back({"Local Gen", getSankeyColor(7), 2});
    lNodes1.push_back({"Residential", getSankeyColor(8), 3});
    lNodes1.push_back({"Commercial", getSankeyColor(9), 3});
    lNodes1.push_back({"Industrial", getSankeyColor(0), 3});
    lNodes1.push_back({"Transport", getSankeyColor(1), 3});

    std::vector<RLSankeyLink> lLinks1 = {
        {0, 4, 35.0f}, {1, 4, 25.0f}, {1, 5, 15.0f}, {2, 4, 20.0f},
        {3, 4, 15.0f}, {3, 5, 10.0f}, {4, 6, 80.0f}, {4, 7, 15.0f},
        {5, 7, 25.0f}, {6, 8, 25.0f}, {6, 9, 20.0f}, {6, 10, 30.0f},
        {6, 11, 5.0f}, {7, 8, 15.0f}, {7, 10, 20.0f}, {7, 11, 5.0f}
    };

    std::vector<float> lOrigValues1;
    for (const auto& rLink : lLinks1) lOrigValues1.push_back(rLink.mValue);

    RLSankey lChart1(lBounds1, lStyle);
    lChart1.setData(lNodes1, lLinks1);

    // Website flow data
    std::vector<RLSankeyNode> lNodes2;
    lNodes2.push_back({"Search", {66, 133, 244, 255}, 0});
    lNodes2.push_back({"Social", {234, 67, 53, 255}, 0});
    lNodes2.push_back({"Direct", {52, 168, 83, 255}, 0});
    lNodes2.push_back({"Referral", {251, 188, 4, 255}, 0});
    lNodes2.push_back({"Homepage", {154, 99, 191, 255}, 1});
    lNodes2.push_back({"Blog", {0, 188, 212, 255}, 1});
    lNodes2.push_back({"Products", {255, 112, 67, 255}, 1});
    lNodes2.push_back({"Browse", {156, 204, 101, 255}, 2});
    lNodes2.push_back({"Read", {121, 134, 203, 255}, 2});
    lNodes2.push_back({"Add to Cart", {255, 167, 38, 255}, 2});
    lNodes2.push_back({"Purchase", {76, 175, 80, 255}, 3});
    lNodes2.push_back({"Subscribe", {33, 150, 243, 255}, 3});
    lNodes2.push_back({"Bounce", {158, 158, 158, 255}, 3});

    std::vector<RLSankeyLink> lLinks2 = {
        {0, 4, 40.0f}, {0, 5, 25.0f}, {0, 6, 35.0f}, {1, 4, 15.0f},
        {1, 5, 30.0f}, {2, 4, 50.0f}, {2, 6, 20.0f}, {3, 5, 15.0f},
        {3, 6, 10.0f}, {4, 7, 60.0f}, {4, 9, 25.0f}, {4, 12, 20.0f},
        {5, 8, 50.0f}, {5, 12, 20.0f}, {6, 7, 30.0f}, {6, 9, 25.0f},
        {6, 12, 10.0f}, {7, 9, 40.0f}, {7, 12, 50.0f}, {8, 11, 30.0f},
        {8, 12, 20.0f}, {9, 10, 50.0f}
    };

    std::vector<float> lOrigValues2;
    for (const auto& rLink : lLinks2) lOrigValues2.push_back(rLink.mValue);

    RLSankeyStyle lStyle2 = lStyle;
    lStyle2.mBackground = Color{22, 18, 30, 255};
    RLSankey lChart2(lBounds2, lStyle2);
    lChart2.setData(lNodes2, lLinks2);

    float lFluctuateTimer = 0.0f;
    int lFramesSaved = 0;

    for (int lFrame = 0; lFrame < TOTAL_FRAMES; ++lFrame) {
        lFluctuateTimer += FIXED_DT;

        if (lFluctuateTimer > 1.5f) {
            lFluctuateTimer = 0.0f;
            for (size_t i = 0; i < lOrigValues1.size(); ++i) {
                lChart1.setLinkValue(i, lOrigValues1[i] * randFloat(0.7f, 1.3f));
            }
            for (size_t i = 0; i < lOrigValues2.size(); ++i) {
                lChart2.setLinkValue(i, lOrigValues2[i] * randFloat(0.75f, 1.25f));
            }
        }

        lChart1.update(FIXED_DT);
        lChart2.update(FIXED_DT);

        BeginDrawing();
        ClearBackground(Color{12, 14, 18, 255});

        lChart1.draw();
        lChart2.draw();

        Color lTitleColor = {230, 235, 245, 255};
        const char* lT1 = "Energy Flow";
        const char* lT2 = "Website Analytics";
        Vector2 lT1S = MeasureTextEx(lFont, lT1, 22.0f, 1.0f);
        Vector2 lT2S = MeasureTextEx(lFont, lT2, 22.0f, 1.0f);
        DrawTextEx(lFont, lT1, {lBounds1.x + (lBounds1.width - lT1S.x) * 0.5f, lMargin + 15.0f}, 22.0f, 1.0f, lTitleColor);
        DrawTextEx(lFont, lT2, {lBounds2.x + (lBounds2.width - lT2S.x) * 0.5f, lMargin + 15.0f}, 22.0f, 1.0f, lTitleColor);

        EndDrawing();

        if (lFrame % CAPTURE_EVERY == 0) {
            Image lImg = LoadImageFromScreen();
            char lPath[512];
            snprintf(lPath, sizeof(lPath), "%s/frame_%04d.png", rOutputDir.c_str(), lFramesSaved);
            ExportImage(lImg, lPath);
            UnloadImage(lImg);
            lFramesSaved++;
        }
    }

    UnloadFont(lFont);
    printf("Sankey: saved %d frames\n", lFramesSaved);
}

// ============================================================================
// Main
// ============================================================================

int main(int aArgc, char* apArgv[]) {
    if (aArgc < 3) {
        printf("Usage: %s <chart_name> <output_dir>\n", apArgv[0]);
        printf("  chart_name: areachart, lineargauge, radar, sankey\n");
        return 1;
    }

    std::string lChartName = apArgv[1];
    std::string lOutputDir = apArgv[2];

    srand(42);

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GIF Frame Capture");
    SetTargetFPS(60);

    if (lChartName == "areachart") {
        captureAreaChart(lOutputDir);
    } else if (lChartName == "lineargauge") {
        captureLinearGauge(lOutputDir);
    } else if (lChartName == "radar") {
        captureRadarChart(lOutputDir);
    } else if (lChartName == "sankey") {
        captureSankey(lOutputDir);
    } else {
        printf("Unknown chart: %s\n", lChartName.c_str());
        CloseWindow();
        return 1;
    }

    CloseWindow();
    return 0;
}
