// volumemeter.cpp
// Demo: VU Meter / Volume Meter - Multi-channel audio-style meters
// Shows stereo, 5.1 surround, and custom multi-source configurations
// Features: green/yellow/red gradient, sticky peak markers, clip indicators, dB scale
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include "RLLinearGauge.h"

// ============================================================================
// Demo Configuration
// ============================================================================

static const int SCREEN_WIDTH = 1200;
static const int SCREEN_HEIGHT = 800;

// ============================================================================
// Helper Functions
// ============================================================================

static float randFloat(float aMin, float aMax) {
    return aMin + ((float)rand() / (float)RAND_MAX) * (aMax - aMin);
}

// Simulate audio-like signal with occasional peaks
static float simulateAudioLevel(float aBaseLevel, float aTime, float aPhaseOffset) {
    // Combine multiple sine waves for more organic movement
    float lLevel = aBaseLevel;
    lLevel += sinf(aTime * 2.0f + aPhaseOffset) * 0.15f;
    lLevel += sinf(aTime * 5.3f + aPhaseOffset * 1.7f) * 0.1f;
    lLevel += sinf(aTime * 11.7f + aPhaseOffset * 2.3f) * 0.05f;

    // Random bursts
    if (randFloat(0.0f, 1.0f) < 0.02f) {
        lLevel += randFloat(0.1f, 0.4f);
    }

    return std::max(0.0f, std::min(1.0f, lLevel));
}

// ============================================================================
// Main Demo
// ============================================================================

int main() {
    srand((unsigned int)time(nullptr));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLLinearGauge VU Meter Demo - Multi-Channel Volume Meters");
    SetTargetFPS(60);

    // Load font
    Font lFont = LoadFontEx("base.ttf", 24, nullptr, 250);

    float lMargin = 30.0f;

    // =========================================================================
    // 1. Stereo VU Meter (Vertical, Classic L/R)
    // =========================================================================

    RLLinearGaugeStyle lStereoStyle;
    lStereoStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lStereoStyle.mTrackColor = Color{40, 44, 52, 255};
    lStereoStyle.mLabelFont = lFont;
    lStereoStyle.mTrackThickness = 80.0f;
    lStereoStyle.mShowTicks = false;
    lStereoStyle.mShowValueText = false;

    // VU meter specific style
    lStereoStyle.mVuStyle.mLowColor = Color{80, 200, 120, 255};
    lStereoStyle.mVuStyle.mMidColor = Color{255, 200, 80, 255};
    lStereoStyle.mVuStyle.mHighColor = Color{255, 80, 80, 255};
    lStereoStyle.mVuStyle.mLowThreshold = 0.6f;
    lStereoStyle.mVuStyle.mMidThreshold = 0.85f;
    lStereoStyle.mVuStyle.mPeakHoldTime = 1.5f;
    lStereoStyle.mVuStyle.mPeakDecaySpeed = 0.4f;
    lStereoStyle.mVuStyle.mChannelSpacing = 6.0f;
    lStereoStyle.mVuStyle.mShowChannelLabels = true;
    lStereoStyle.mVuStyle.mChannelLabelFontSize = 14.0f;
    lStereoStyle.mVuStyle.mClipIndicatorSize = 10.0f;

    Rectangle lStereoBounds = {lMargin, lMargin + 30.0f, 100.0f, 400.0f};
    RLLinearGauge lStereoMeter(lStereoBounds, 0.0f, 1.0f, RLLinearGaugeOrientation::VERTICAL, lStereoStyle);
    lStereoMeter.setMode(RLLinearGaugeMode::VU_METER);
    lStereoMeter.setLabel("Stereo");

    std::vector<RLVuMeterChannel> lStereoChannels = {
        {0.0f, "L"},
        {0.0f, "R"}
    };
    lStereoMeter.setChannels(lStereoChannels);

    // =========================================================================
    // 2. 5.1 Surround VU Meter (Vertical, 6 channels)
    // =========================================================================

    RLLinearGaugeStyle lSurroundStyle = lStereoStyle;
    lSurroundStyle.mTrackThickness = 200.0f;
    lSurroundStyle.mVuStyle.mChannelSpacing = 4.0f;
    lSurroundStyle.mVuStyle.mChannelLabelFontSize = 10.0f;

    Rectangle lSurroundBounds = {lMargin + 130.0f, lMargin + 30.0f, 220.0f, 400.0f};
    RLLinearGauge lSurroundMeter(lSurroundBounds, 0.0f, 1.0f, RLLinearGaugeOrientation::VERTICAL, lSurroundStyle);
    lSurroundMeter.setMode(RLLinearGaugeMode::VU_METER);
    lSurroundMeter.setLabel("5.1 Surround");

    std::vector<RLVuMeterChannel> lSurroundChannels = {
        {0.0f, "L"},
        {0.0f, "R"},
        {0.0f, "C"},
        {0.0f, "LFE"},
        {0.0f, "Ls"},
        {0.0f, "Rs"}
    };
    lSurroundMeter.setChannels(lSurroundChannels);

    // =========================================================================
    // 3. dB Scale VU Meter (Vertical, with dB scale enabled)
    // =========================================================================

    RLLinearGaugeStyle lDbStyle = lStereoStyle;
    lDbStyle.mTrackThickness = 80.0f;
    lDbStyle.mVuStyle.mUseDbScale = true;
    lDbStyle.mVuStyle.mDbMin = -60.0f;
    lDbStyle.mVuStyle.mDbMax = 0.0f;
    lDbStyle.mVuStyle.mLowThreshold = 0.7f;  // Adjust for dB curve
    lDbStyle.mVuStyle.mMidThreshold = 0.9f;

    Rectangle lDbBounds = {lMargin + 380.0f, lMargin + 30.0f, 100.0f, 400.0f};
    RLLinearGauge lDbMeter(lDbBounds, 0.0f, 1.0f, RLLinearGaugeOrientation::VERTICAL, lDbStyle);
    lDbMeter.setMode(RLLinearGaugeMode::VU_METER);
    lDbMeter.setLabel("dB Scale");

    std::vector<RLVuMeterChannel> lDbChannels = {
        {0.0f, "L"},
        {0.0f, "R"}
    };
    lDbMeter.setChannels(lDbChannels);

    // =========================================================================
    // 4. Horizontal Multi-Source Meter (Custom labels)
    // =========================================================================

    RLLinearGaugeStyle lHorizStyle;
    lHorizStyle.mBackgroundColor = Color{28, 32, 40, 255};
    lHorizStyle.mTrackColor = Color{40, 44, 52, 255};
    lHorizStyle.mLabelFont = lFont;
    lHorizStyle.mTrackThickness = 120.0f;
    lHorizStyle.mShowTicks = false;
    lHorizStyle.mShowValueText = false;

    // Custom colors for sensor data visualization
    lHorizStyle.mVuStyle.mLowColor = Color{100, 180, 255, 255};   // Blue
    lHorizStyle.mVuStyle.mMidColor = Color{180, 120, 255, 255};   // Purple
    lHorizStyle.mVuStyle.mHighColor = Color{255, 100, 150, 255};  // Pink
    lHorizStyle.mVuStyle.mLowThreshold = 0.5f;
    lHorizStyle.mVuStyle.mMidThreshold = 0.8f;
    lHorizStyle.mVuStyle.mPeakHoldTime = 2.0f;
    lHorizStyle.mVuStyle.mPeakDecaySpeed = 0.3f;
    lHorizStyle.mVuStyle.mChannelSpacing = 8.0f;
    lHorizStyle.mVuStyle.mShowChannelLabels = true;
    lHorizStyle.mVuStyle.mChannelLabelFontSize = 11.0f;

    Rectangle lHorizBounds = {lMargin + 520.0f, lMargin + 30.0f, 400.0f, 150.0f};
    RLLinearGauge lHorizMeter(lHorizBounds, 0.0f, 100.0f, RLLinearGaugeOrientation::HORIZONTAL, lHorizStyle);
    lHorizMeter.setMode(RLLinearGaugeMode::VU_METER);
    lHorizMeter.setLabel("Multi-Source Data");

    std::vector<RLVuMeterChannel> lHorizChannels = {
        {0.0f, "Sensor 1"},
        {0.0f, "Sensor 2"},
        {0.0f, "Sensor 3"},
        {0.0f, "Sensor 4"}
    };
    lHorizMeter.setChannels(lHorizChannels);

    // =========================================================================
    // 5. Compact 8-Channel Meter
    // =========================================================================

    RLLinearGaugeStyle lCompactStyle = lStereoStyle;
    lCompactStyle.mTrackThickness = 180.0f;
    lCompactStyle.mVuStyle.mChannelSpacing = 2.0f;
    lCompactStyle.mVuStyle.mChannelLabelFontSize = 8.0f;
    lCompactStyle.mVuStyle.mClipIndicatorSize = 6.0f;

    Rectangle lCompactBounds = {lMargin + 520.0f, lMargin + 220.0f, 200.0f, 300.0f};
    RLLinearGauge lCompactMeter(lCompactBounds, 0.0f, 1.0f, RLLinearGaugeOrientation::VERTICAL, lCompactStyle);
    lCompactMeter.setMode(RLLinearGaugeMode::VU_METER);
    lCompactMeter.setLabel("8-Channel");

    std::vector<RLVuMeterChannel> lCompactChannels = {
        {0.0f, "1"}, {0.0f, "2"}, {0.0f, "3"}, {0.0f, "4"},
        {0.0f, "5"}, {0.0f, "6"}, {0.0f, "7"}, {0.0f, "8"}
    };
    lCompactMeter.setChannels(lCompactChannels);

    // =========================================================================
    // Info Panel
    // =========================================================================

    float lInfoX = lMargin + 750.0f;
    float lInfoY = lMargin + 220.0f;

    // State
    float lTime = 0.0f;
    bool lDbScaleEnabled = true;

    // Simulated audio base levels per channel
    std::vector<float> lStereoBase = {0.5f, 0.45f};
    std::vector<float> lSurroundBase = {0.5f, 0.48f, 0.4f, 0.6f, 0.35f, 0.33f};
    std::vector<float> lHorizBase = {50.0f, 60.0f, 45.0f, 55.0f};
    std::vector<float> lCompactBase = {0.4f, 0.45f, 0.5f, 0.55f, 0.42f, 0.48f, 0.52f, 0.46f};

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Keyboard controls
        if (IsKeyPressed(KEY_D)) {
            lDbScaleEnabled = !lDbScaleEnabled;
            auto lStyle = lDbMeter.getMode() == RLLinearGaugeMode::VU_METER ?
                          lDbStyle : lStereoStyle;
            lStyle.mVuStyle.mUseDbScale = lDbScaleEnabled;
            lDbMeter.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_R)) {
            lStereoMeter.resetPeaks();
            lSurroundMeter.resetPeaks();
            lDbMeter.resetPeaks();
            lHorizMeter.resetPeaks();
            lCompactMeter.resetPeaks();
        }

        if (IsKeyPressed(KEY_C)) {
            lStereoMeter.resetClip();
            lSurroundMeter.resetClip();
            lDbMeter.resetClip();
            lHorizMeter.resetClip();
            lCompactMeter.resetClip();
        }

        // Simulate audio levels
        // Stereo
        for (int i = 0; i < 2; ++i) {
            float lLevel = simulateAudioLevel(lStereoBase[(size_t)i], lTime, (float)i * 1.5f);
            lStereoMeter.setChannelValue(i, lLevel);
        }

        // 5.1 Surround
        for (int i = 0; i < 6; ++i) {
            float lLevel = simulateAudioLevel(lSurroundBase[(size_t)i], lTime, (float)i * 0.8f);
            lSurroundMeter.setChannelValue(i, lLevel);
        }

        // dB meter
        for (int i = 0; i < 2; ++i) {
            float lLevel = simulateAudioLevel(lStereoBase[(size_t)i], lTime, (float)i * 2.1f);
            lDbMeter.setChannelValue(i, lLevel);
        }

        // Horizontal multi-source
        for (int i = 0; i < 4; ++i) {
            float lBase = lHorizBase[(size_t)i];
            float lLevel = lBase + sinf(lTime * (1.5f + (float)i * 0.3f)) * 20.0f + randFloat(-5.0f, 5.0f);
            lLevel = std::max(0.0f, std::min(100.0f, lLevel));
            lHorizMeter.setChannelValue(i, lLevel);
        }

        // 8-channel compact
        for (int i = 0; i < 8; ++i) {
            float lLevel = simulateAudioLevel(lCompactBase[(size_t)i], lTime, (float)i * 0.5f);
            lCompactMeter.setChannelValue(i, lLevel);
        }

        // Update all meters
        lStereoMeter.update(lDt);
        lSurroundMeter.update(lDt);
        lDbMeter.update(lDt);
        lHorizMeter.update(lDt);
        lCompactMeter.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground(Color{18, 20, 26, 255});

        // Draw all meters
        lStereoMeter.draw();
        lSurroundMeter.draw();
        lDbMeter.draw();
        lHorizMeter.draw();
        lCompactMeter.draw();

        // Draw info panel
        auto lTextColor = Color{180, 190, 210, 255};
        auto lHeaderColor = Color{220, 225, 235, 255};
        float lY = lInfoY;

        DrawTextEx(lFont, "VU Meter Demo", Vector2{lInfoX, lY}, 22, 1.0f, lHeaderColor);
        lY += 35.0f;

        DrawTextEx(lFont, "Features:", Vector2{lInfoX, lY}, 16, 1.0f, lTextColor);
        lY += 25.0f;

        DrawTextEx(lFont, "- Multi-channel grouping", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "- Green/Yellow/Red gradient", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "- Sticky peak markers", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "- Clip indicator (flashing)", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "- Optional dB scale", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "- Custom colors & labels", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 35.0f;

        DrawTextEx(lFont, "Controls:", Vector2{lInfoX, lY}, 16, 1.0f, lHeaderColor);
        lY += 25.0f;

        DrawTextEx(lFont, "[D] Toggle dB scale", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "[R] Reset peaks", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "[C] Clear clip indicators", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 20.0f;

        DrawTextEx(lFont, "[ESC] Exit", Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lTextColor);
        lY += 35.0f;

        // Status
        DrawTextEx(lFont, "Status:", Vector2{lInfoX, lY}, 16, 1.0f, lHeaderColor);
        lY += 25.0f;

        char lStatusBuf[64];
        Color lDbColor = lDbScaleEnabled ? Color{80, 200, 120, 255} : Color{255, 100, 100, 255};
        snprintf(lStatusBuf, sizeof(lStatusBuf), "dB Scale: %s", lDbScaleEnabled ? "ON" : "OFF");
        DrawTextEx(lFont, lStatusBuf, Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lDbColor);
        lY += 20.0f;

        // Show clip status
        bool lAnyClipping = false;
        for (int i = 0; i < lStereoMeter.getChannelCount(); ++i) {
            if (lStereoMeter.isClipping(i)) {
                lAnyClipping = true;
            }
        }
        Color lClipColor = lAnyClipping ? Color{255, 80, 80, 255} : Color{80, 200, 120, 255};
        snprintf(lStatusBuf, sizeof(lStatusBuf), "Clipping: %s", lAnyClipping ? "YES" : "No");
        DrawTextEx(lFont, lStatusBuf, Vector2{lInfoX + 10.0f, lY}, 13, 1.0f, lClipColor);

        DrawFPS(SCREEN_WIDTH - 90, 10);

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}

