// lineargauge.cpp
// Demo: Linear Gauge - Dashboard-style horizontal and vertical progress gauges
// Shows smooth animations, colored range bands, tick marks, and pointer styles
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include "RLLinearGauge.h"

// ============================================================================
// Demo Configuration
// ============================================================================

static const int SCREEN_WIDTH = 1400;
static const int SCREEN_HEIGHT = 800;
static const float UPDATE_INTERVAL = 2.5f;

// ============================================================================
// Helper Functions
// ============================================================================

static float randFloat(float aMin, float aMax) {
    return aMin + ((float)rand() / (float)RAND_MAX) * (aMax - aMin);
}

// ============================================================================
// Main Demo
// ============================================================================

int main() {
    srand((unsigned int)time(nullptr));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLLinearGauge Demo - Dashboard Linear Gauges");
    SetTargetFPS(60);

    // Load font
    Font lFont = LoadFontEx("base.ttf", 24, nullptr, 250);

    // =========================================================================
    // Create Horizontal Gauges (Top Row)
    // =========================================================================

    float lMargin = 30.0f;
    float lGaugeHeight = 100.0f;
    float lGaugeWidth = (SCREEN_WIDTH - 4.0f * lMargin) / 3.0f;

    // Gauge 1: Temperature gauge with colored zones (Fill Bar style)
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
    lTempGauge.setUnit("\xC2\xB0" "C"); // °C in UTF-8
    lTempGauge.setValue(45.0f);

    // Add colored range bands (green/yellow/red zones)
    std::vector<RLLinearGaugeRangeBand> lTempRanges = {
        {0.0f, 60.0f, Color{80, 200, 120, 255}},   // Green: Safe
        {60.0f, 80.0f, Color{255, 200, 80, 255}},  // Yellow: Warning
        {80.0f, 100.0f, Color{255, 80, 80, 255}}   // Red: Danger
    };
    lTempGauge.setRanges(lTempRanges);
    lTempGauge.setTargetMarker(75.0f); // Goal/threshold marker

    // Gauge 2: CPU Load gauge (Triangle pointer style)
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
        {0.0f, 50.0f, Color{80, 180, 255, 255}},   // Blue: Low
        {50.0f, 80.0f, Color{255, 180, 80, 255}},  // Orange: Medium
        {80.0f, 100.0f, Color{255, 80, 100, 255}}  // Red: High
    };
    lCpuGauge.setRanges(lCpuRanges);

    // Gauge 3: Progress gauge (Line marker style)
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
    lProgressGauge.setTargetMarker(800.0f); // Target download

    // =========================================================================
    // Create Vertical Gauges (Bottom Section)
    // =========================================================================

    float lVerticalTop = lMargin + lGaugeHeight + 40.0f;
    float lVerticalHeight = SCREEN_HEIGHT - lVerticalTop - lMargin - 60.0f;
    float lVerticalWidth = 120.0f;
    float lVerticalSpacing = 40.0f;

    // Vertical Gauge 1: Pressure (Fill Bar)
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

    // Vertical Gauge 2: Volume (Triangle pointer)
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

    // Vertical Gauge 3: Fuel Level (Fill Bar with zones)
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
        {0.0f, 20.0f, Color{255, 80, 80, 255}},    // Red: Low
        {20.0f, 50.0f, Color{255, 200, 80, 255}},  // Yellow: Medium
        {50.0f, 100.0f, Color{80, 200, 120, 255}}  // Green: Full
    };
    lFuelGauge.setRanges(lFuelRanges);
    lFuelGauge.setTargetMarker(25.0f); // Low fuel warning threshold

    // Vertical Gauge 4: Speed (Line marker)
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
    lSpeedGauge.setTargetMarker(120.0f); // Speed limit

    // =========================================================================
    // Info Panel (right side) - Position after vertical gauges with proper spacing
    // =========================================================================

    // Calculate end of vertical gauges area (including space for labels/ticks)
    float lVerticalGaugesEnd = lMargin + 4.0f * (lVerticalWidth + lVerticalSpacing);
    float lInfoPanelWidth = 300.0f;
    float lInfoX = lVerticalGaugesEnd + 60.0f; // Add extra spacing for gauge labels

    // Ensure info panel fits within screen, otherwise position at right edge
    if (lInfoX + lInfoPanelWidth > SCREEN_WIDTH) {
        lInfoX = SCREEN_WIDTH - lInfoPanelWidth - lMargin;
    }

    // State variables
    bool lAnimationEnabled = true;
    int lPointerStyleIndex = 0;
    const char* lPointerStyleNames[] = {"Fill Bar", "Triangle", "Line Marker"};

    float lUpdateTimer = 0.0f;
    float lSinTime = 0.0f;

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lSinTime += lDt;
        lUpdateTimer += lDt;

        // Keyboard controls
        if (IsKeyPressed(KEY_SPACE)) {
            lAnimationEnabled = !lAnimationEnabled;

            lTempGauge.setAnimationEnabled(lAnimationEnabled);
            lCpuGauge.setAnimationEnabled(lAnimationEnabled);
            lProgressGauge.setAnimationEnabled(lAnimationEnabled);
            lPressureGauge.setAnimationEnabled(lAnimationEnabled);
            lVolumeGauge.setAnimationEnabled(lAnimationEnabled);
            lFuelGauge.setAnimationEnabled(lAnimationEnabled);
            lSpeedGauge.setAnimationEnabled(lAnimationEnabled);
        }

        if (IsKeyPressed(KEY_P)) {
            lPointerStyleIndex = (lPointerStyleIndex + 1) % 3;
            auto lNewStyle = (RLLinearGaugePointerStyle)lPointerStyleIndex;
            lCpuGauge.setPointerStyle(lNewStyle);
            lVolumeGauge.setPointerStyle(lNewStyle);
        }

        // Update values periodically (with sin wave + random)
        if (lUpdateTimer >= UPDATE_INTERVAL) {
            lUpdateTimer = 0.0f;

            // Temperature: slow wave
            float lTempValue = 50.0f + sinf(lSinTime * 0.3f) * 40.0f + randFloat(-5.0f, 5.0f);
            lTempGauge.setTargetValue(lTempValue);

            // CPU: faster random
            lCpuGauge.setTargetValue(randFloat(10.0f, 95.0f));

            // Progress: always increasing
            float lCurrentProgress = lProgressGauge.getValue();
            float lNewProgress = lCurrentProgress + randFloat(50.0f, 150.0f);
            if (lNewProgress > 1000.0f) {
                lNewProgress = randFloat(0.0f, 200.0f);
            }
            lProgressGauge.setTargetValue(lNewProgress);

            // Pressure: medium wave
            float lPressureValue = 100.0f + sinf(lSinTime * 0.5f) * 80.0f + randFloat(-10.0f, 10.0f);
            lPressureGauge.setTargetValue(lPressureValue);

            // Volume: random jumps
            lVolumeGauge.setTargetValue(randFloat(20.0f, 90.0f));

            // Fuel: slowly decreasing with refills
            float lCurrentFuel = lFuelGauge.getValue();
            float lNewFuel = lCurrentFuel - randFloat(5.0f, 15.0f);
            if (lNewFuel < 10.0f) {
                lNewFuel = randFloat(70.0f, 100.0f); // Refuel
            }
            lFuelGauge.setTargetValue(lNewFuel);

            // Speed: variable
            float lSpeedValue = 80.0f + sinf(lSinTime * 0.8f) * 60.0f + randFloat(-20.0f, 20.0f);
            lSpeedGauge.setTargetValue(std::max(0.0f, std::min(240.0f, lSpeedValue)));
        }

        // Update all gauges
        lTempGauge.update(lDt);
        lCpuGauge.update(lDt);
        lProgressGauge.update(lDt);
        lPressureGauge.update(lDt);
        lVolumeGauge.update(lDt);
        lFuelGauge.update(lDt);
        lSpeedGauge.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground(Color{18, 20, 26, 255});

        // Draw all gauges
        lTempGauge.draw();
        lCpuGauge.draw();
        lProgressGauge.draw();
        lPressureGauge.draw();
        lVolumeGauge.draw();
        lFuelGauge.draw();
        lSpeedGauge.draw();

        // Draw info panel
        float lInfoY = lVerticalTop;
        auto lTextColor = Color{180, 190, 210, 255};
        auto lHeaderColor = Color{220, 225, 235, 255};

        DrawTextEx(lFont, "RLLinearGauge Demo", Vector2{lInfoX, lInfoY}, 22, 1.0f, lHeaderColor);
        lInfoY += 35.0f;

        DrawTextEx(lFont, "Dashboard-style linear gauges with:", Vector2{lInfoX, lInfoY}, 16, 1.0f, lTextColor);
        lInfoY += 28.0f;

        DrawTextEx(lFont, "- Horizontal & Vertical orientations", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 22.0f;

        DrawTextEx(lFont, "- Colored range bands (zones)", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 22.0f;

        DrawTextEx(lFont, "- Multiple pointer styles", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 22.0f;

        DrawTextEx(lFont, "- Smooth value animations", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 22.0f;

        DrawTextEx(lFont, "- Target/goal markers", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 22.0f;

        DrawTextEx(lFont, "- Major & minor tick marks", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 40.0f;

        DrawTextEx(lFont, "Controls:", Vector2{lInfoX, lInfoY}, 18, 1.0f, lHeaderColor);
        lInfoY += 28.0f;

        DrawTextEx(lFont, "[SPACE] Toggle animation", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 22.0f;

        DrawTextEx(lFont, "[P] Cycle pointer style", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 22.0f;

        DrawTextEx(lFont, "[ESC] Exit", Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);
        lInfoY += 40.0f;

        // Status
        DrawTextEx(lFont, "Status:", Vector2{lInfoX, lInfoY}, 18, 1.0f, lHeaderColor);
        lInfoY += 28.0f;

        const char* lAnimStatus = lAnimationEnabled ? "Enabled" : "Disabled";
        Color lAnimColor = lAnimationEnabled ? Color{80, 200, 120, 255} : Color{255, 100, 100, 255};
        char lStatusBuf[64];
        snprintf(lStatusBuf, sizeof(lStatusBuf), "Animation: %s", lAnimStatus);
        DrawTextEx(lFont, lStatusBuf, Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lAnimColor);
        lInfoY += 22.0f;

        snprintf(lStatusBuf, sizeof(lStatusBuf), "Pointer: %s", lPointerStyleNames[lPointerStyleIndex]);
        DrawTextEx(lFont, lStatusBuf, Vector2{lInfoX + 10.0f, lInfoY}, 14, 1.0f, lTextColor);

        DrawFPS(SCREEN_WIDTH - 90, 10);

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}

