// areachart.cpp
// Demo: Area chart with three modes - OVERLAPPED, STACKED, and PERCENT (100% stacked)
// Shows smooth data transitions using setTargetData()
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include "RLAreaChart.h"

// ============================================================================
// Demo Configuration
// ============================================================================

static const int SCREEN_WIDTH = 1400;
static const int SCREEN_HEIGHT = 700;
static const int NUM_POINTS = 12;
static const int NUM_SERIES = 4;
static const float UPDATE_INTERVAL = 2.0f;

// Series colors (with good contrast for overlapped mode)
static const Color SERIES_COLORS[] = {
    {80, 180, 255, 255},   // Blue
    {255, 120, 80, 255},   // Orange
    {120, 220, 120, 255},  // Green
    {220, 100, 220, 255}   // Purple
};

// Series labels
static const char* SERIES_LABELS[] = {
    "Series A",
    "Series B",
    "Series C",
    "Series D"
};

// X-axis labels (months)
static const char* X_LABELS[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// ============================================================================
// Data Generation
// ============================================================================

static float randFloat(float aMin, float aMax) {
    return aMin + ((float)rand() / (float)RAND_MAX) * (aMax - aMin);
}

static std::vector<RLAreaSeries> generateData(float aTime) {
    std::vector<RLAreaSeries> lData;

    for (int s = 0; s < NUM_SERIES; ++s) {
        RLAreaSeries lSeries;
        lSeries.mColor = SERIES_COLORS[s];
        lSeries.mLabel = SERIES_LABELS[s];
        lSeries.mAlpha = 0.7f;

        for (int i = 0; i < NUM_POINTS; ++i) {
            // Generate wave-like data with some randomness
            float lPhase = (float)i / (float)NUM_POINTS * 2.0f * PI;
            float lWave = sinf(lPhase + aTime * 0.5f + (float)s * 0.8f);
            float lBase = 20.0f + (float)s * 15.0f;
            float lValue = lBase + lWave * 15.0f + randFloat(-5.0f, 5.0f);
            lSeries.mValues.push_back(fmaxf(5.0f, lValue));
        }

        lData.push_back(lSeries);
    }

    return lData;
}

// ============================================================================
// Main Demo
// ============================================================================

int main() {
    srand((unsigned int)time(nullptr));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLAreaChart Demo - Three Modes with Smooth Transitions");
    SetTargetFPS(60);

    // Load font
    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    // Calculate chart bounds (three side-by-side)
    float lMargin = 20.0f;
    float lGap = 15.0f;
    float lChartWidth = (SCREEN_WIDTH - 2.0f * lMargin - 2.0f * lGap) / 3.0f;
    float lChartHeight = SCREEN_HEIGHT - 2.0f * lMargin - 60.0f;
    float lChartY = lMargin + 50.0f;

    Rectangle lBounds1 = {lMargin, lChartY, lChartWidth, lChartHeight};
    Rectangle lBounds2 = {lMargin + lChartWidth + lGap, lChartY, lChartWidth, lChartHeight};
    Rectangle lBounds3 = {lMargin + 2.0f * (lChartWidth + lGap), lChartY, lChartWidth, lChartHeight};

    // Common style
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

    // Create three charts with different modes
    RLAreaChart lChartOverlapped(lBounds1, RLAreaChartMode::OVERLAPPED, lStyle);
    RLAreaChart lChartStacked(lBounds2, RLAreaChartMode::STACKED, lStyle);
    RLAreaChart lChartPercent(lBounds3, RLAreaChartMode::PERCENT, lStyle);

    // Set X labels
    std::vector<std::string> lXLabels;
    for (int i = 0; i < NUM_POINTS; ++i) {
        lXLabels.push_back(X_LABELS[i]);
    }
    lChartOverlapped.setXLabels(lXLabels);
    lChartStacked.setXLabels(lXLabels);
    lChartPercent.setXLabels(lXLabels);

    // Initial data
    std::vector<RLAreaSeries> lInitialData = generateData(0.0f);

    // For overlapped mode, use more transparency
    std::vector<RLAreaSeries> lOverlappedData = lInitialData;
    for (auto& rSeries : lOverlappedData) {
        rSeries.mAlpha = 0.5f;
    }

    lChartOverlapped.setData(lOverlappedData);
    lChartStacked.setData(lInitialData);
    lChartPercent.setData(lInitialData);

    float lTime = 0.0f;
    float lUpdateTimer = 0.0f;

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;
        lUpdateTimer += lDt;

        // Update data periodically
        if (lUpdateTimer >= UPDATE_INTERVAL) {
            lUpdateTimer = 0.0f;

            std::vector<RLAreaSeries> lNewData = generateData(lTime);

            // Overlapped uses more transparency
            std::vector<RLAreaSeries> lNewOverlappedData = lNewData;
            for (auto& rSeries : lNewOverlappedData) {
                rSeries.mAlpha = 0.5f;
            }

            lChartOverlapped.setTargetData(lNewOverlappedData);
            lChartStacked.setTargetData(lNewData);
            lChartPercent.setTargetData(lNewData);
        }

        // Update charts
        lChartOverlapped.update(lDt);
        lChartStacked.update(lDt);
        lChartPercent.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground(Color{12, 14, 18, 255});

        // Title
        const char* lTitle = "RLAreaChart - Three Visualization Modes";
        int lTitleWidth = MeasureText(lTitle, 24);
        DrawTextEx(lFont, lTitle, Vector2{(float)((SCREEN_WIDTH - lTitleWidth) / 2), 12}, 24, 1.0f, Color{220, 225, 235, 255});

        // Mode labels
        Color lLabelColor = Color{150, 160, 180, 255};

        const char* lLabel1 = "OVERLAPPED (Transparent)";
        int lLabel1Width = MeasureText(lLabel1, 16);
        DrawTextEx(lFont, lLabel1, Vector2{lBounds1.x + (lBounds1.width - (float)lLabel1Width) / 2.0f, lBounds1.y - 22.0f},
                 16, 1.0f, lLabelColor);

        const char* lLabel2 = "STACKED (Normal)";
        int lLabel2Width = MeasureText(lLabel2, 16);
        DrawTextEx(lFont, lLabel2, Vector2{lBounds2.x + (lBounds2.width - (float)lLabel2Width) / 2.0f, lBounds2.y - 22.0f},
                 16, 1.0f, lLabelColor);

        const char* lLabel3 = "PERCENT (100% Stacked)";
        int lLabel3Width = MeasureText(lLabel3, 16);
        DrawTextEx(lFont, lLabel3, Vector2{lBounds3.x + (lBounds3.width - (float)lLabel3Width) / 2.0f, lBounds3.y - 22.0f},
                 16, 1.0f, lLabelColor);

        // Draw charts
        lChartOverlapped.draw();
        lChartStacked.draw();
        lChartPercent.draw();

        // Instructions
        DrawTextEx(lFont, "Data transitions smoothly every 2 seconds | ESC to exit",
                 Vector2{10, (float)(SCREEN_HEIGHT - 25)}, 14, 1.0f, Color{100, 110, 130, 255});

        DrawFPS(SCREEN_WIDTH - 90, 10);

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}

