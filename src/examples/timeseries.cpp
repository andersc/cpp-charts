// timeseries.cpp
// Demo: Multi-trace streaming time series visualizer
#include "raylib.h"
#include "RLTimeSeries.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

// ============================================================================
// Demo Configuration
// ============================================================================

static const int SCREEN_WIDTH = 1200;
static const int SCREEN_HEIGHT = 700;
static const size_t WINDOW_SIZE = 400;

// Trace update rates (samples per second)
static const float TRACE_RATES[] = { 60.0f, 30.0f, 15.0f, 45.0f, 20.0f };
static const int NUM_TRACES = 5;

// Trace colors
static const Color TRACE_COLORS[] = {
    { 80, 200, 255, 255 },   // Cyan
    { 255, 120, 80, 255 },   // Orange
    { 120, 255, 120, 255 },  // Green
    { 255, 200, 80, 255 },   // Yellow
    { 200, 120, 255, 255 }   // Purple
};

// Trace line modes
static const RLTimeSeriesLineMode TRACE_MODES[] = {
    RLTimeSeriesLineMode::Spline,
    RLTimeSeriesLineMode::Linear,
    RLTimeSeriesLineMode::Raw,
    RLTimeSeriesLineMode::Spline,
    RLTimeSeriesLineMode::Linear
};

// ============================================================================
// Signal Generators
// ============================================================================

static float generateSignal(int aTraceIndex, float aTime) {
    switch (aTraceIndex) {
        case 0: {
            // Smooth sine wave with slow frequency modulation
            float lFreq = 0.5f + 0.3f * sinf(aTime * 0.1f);
            return sinf(aTime * lFreq * 2.0f * PI) * 0.8f;
        }
        case 1: {
            // Noisy sawtooth
            float lSaw = fmodf(aTime * 0.3f, 1.0f) * 2.0f - 1.0f;
            float lNoise = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.2f;
            return lSaw * 0.6f + lNoise;
        }
        case 2: {
            // Square wave with harmonics
            float lPhase = fmodf(aTime * 0.4f, 1.0f);
            float lSquare = (lPhase < 0.5f) ? 0.7f : -0.7f;
            lSquare += 0.2f * sinf(aTime * 3.0f * PI);
            return lSquare;
        }
        case 3: {
            // Damped oscillation (simulating sensor data)
            float lEnv = expf(-fmodf(aTime, 4.0f) * 0.5f);
            return sinf(aTime * 4.0f * PI) * lEnv * 0.9f;
        }
        case 4: {
            // Random walk with drift
            static float lWalk = 0.0f;
            lWalk += ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f;
            lWalk *= 0.995f; // Slowly decay toward zero
            lWalk = fmaxf(-1.0f, fminf(1.0f, lWalk)); // Clamp
            return lWalk;
        }
        default:
            return 0.0f;
    }
}

// ============================================================================
// Main Demo
// ============================================================================

int main() {
    // Initialize random seed
    srand((unsigned int)time(nullptr));

    // Initialize raylib
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLTimeSeries Demo - Multi-Trace Streaming Visualizer");
    SetTargetFPS(60);

    // Create time series chart
    Rectangle lChartBounds = { 20.0f, 60.0f, SCREEN_WIDTH - 40.0f, SCREEN_HEIGHT - 120.0f };
    RLTimeSeries lTimeSeries(lChartBounds, WINDOW_SIZE);

    // Configure chart style
    RLTimeSeriesChartStyle lChartStyle{};
    lChartStyle.mShowBackground = true;
    lChartStyle.mBackground = { 15, 18, 22, 255 };
    lChartStyle.mShowGrid = true;
    lChartStyle.mGridColor = { 40, 45, 55, 255 };
    lChartStyle.mGridLinesX = 10;
    lChartStyle.mGridLinesY = 6;
    lChartStyle.mShowAxes = true;
    lChartStyle.mAxesColor = { 80, 85, 95, 255 };
    lChartStyle.mPadding = 15.0f;
    lChartStyle.mAutoScaleY = true;
    lChartStyle.mAutoScaleMargin = 0.15f;
    lChartStyle.mSmoothScale = true;
    lChartStyle.mScaleSpeed = 3.0f;
    lChartStyle.mSplinePixels = 3.0f;
    lTimeSeries.setStyle(lChartStyle);

    // Add traces with different styles
    for (int i = 0; i < NUM_TRACES; ++i) {
        RLTimeSeriesTraceStyle lStyle{};
        lStyle.mColor = TRACE_COLORS[i];
        lStyle.mLineThickness = (i == 0) ? 2.5f : 2.0f;
        lStyle.mLineMode = TRACE_MODES[i];
        lStyle.mShowPoints = (TRACE_MODES[i] == RLTimeSeriesLineMode::Linear);
        lStyle.mPointRadius = 2.5f;
        lStyle.mVisible = true;
        lTimeSeries.addTrace(lStyle);
    }

    // Timing accumulators for each trace
    float lAccumulators[NUM_TRACES] = { 0.0f };
    float lTime = 0.0f;

    // Mode cycling state
    int lModeIndex = 0;
    float lModeTimer = 0.0f;
    const float MODE_CYCLE_TIME = 8.0f;
    const char* MODE_NAMES[] = { "All Modes", "All Spline", "All Linear", "All Raw" };

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;
        lModeTimer += lDt;

        // Cycle through display modes
        if (lModeTimer > MODE_CYCLE_TIME) {
            lModeTimer = 0.0f;
            lModeIndex = (lModeIndex + 1) % 4;

            for (int i = 0; i < NUM_TRACES; ++i) {
                RLTimeSeriesTraceStyle lStyle{};
                lStyle.mColor = TRACE_COLORS[i];
                lStyle.mLineThickness = 2.0f;
                lStyle.mVisible = true;
                lStyle.mPointRadius = 2.5f;

                switch (lModeIndex) {
                    case 0: // Original modes
                        lStyle.mLineMode = TRACE_MODES[i];
                        lStyle.mShowPoints = (TRACE_MODES[i] == RLTimeSeriesLineMode::Linear);
                        break;
                    case 1: // All spline
                        lStyle.mLineMode = RLTimeSeriesLineMode::Spline;
                        lStyle.mShowPoints = false;
                        break;
                    case 2: // All linear with points
                        lStyle.mLineMode = RLTimeSeriesLineMode::Linear;
                        lStyle.mShowPoints = true;
                        break;
                    case 3: // All raw
                        lStyle.mLineMode = RLTimeSeriesLineMode::Raw;
                        lStyle.mShowPoints = false;
                        break;
                }

                lTimeSeries.setTraceStyle((size_t)i, lStyle);
            }
        }

        // Push samples at different rates
        for (int i = 0; i < NUM_TRACES; ++i) {
            lAccumulators[i] += lDt;
            float lInterval = 1.0f / TRACE_RATES[i];

            while (lAccumulators[i] >= lInterval) {
                lAccumulators[i] -= lInterval;
                float lValue = generateSignal(i, lTime);
                lTimeSeries.pushSample((size_t)i, lValue);
            }
        }

        // Toggle trace visibility with number keys
        for (int i = 0; i < NUM_TRACES; ++i) {
            if (IsKeyPressed(KEY_ONE + i)) {
                // Toggle visibility
                static bool lTraceVisible[NUM_TRACES] = { true, true, true, true, true };
                lTraceVisible[i] = !lTraceVisible[i];
                lTimeSeries.setTraceVisible((size_t)i, lTraceVisible[i]);
            }
        }

        // Clear all with C key
        if (IsKeyPressed(KEY_C)) {
            lTimeSeries.clearAllTraces();
        }

        // Update chart
        lTimeSeries.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground({ 10, 12, 16, 255 });

        // Title
        DrawText("RLTimeSeries - Multi-Trace Streaming Demo", 20, 15, 24, { 220, 225, 235, 255 });

        // Mode indicator
        DrawText(TextFormat("Mode: %s (auto-cycles every %.0fs)", MODE_NAMES[lModeIndex], MODE_CYCLE_TIME),
                 20, 42, 14, { 150, 155, 165, 255 });

        // Draw chart
        lTimeSeries.draw();

        // Legend
        float lLegendY = SCREEN_HEIGHT - 50.0f;
        float lLegendX = 30.0f;
        DrawText("Traces:", (int)lLegendX, (int)lLegendY, 14, { 180, 185, 195, 255 });
        lLegendX += 60.0f;

        for (int i = 0; i < NUM_TRACES; ++i) {
            DrawRectangle((int)lLegendX, (int)lLegendY + 2, 20, 12, TRACE_COLORS[i]);
            const char* lModeStr = "";
            RLTimeSeriesLineMode lMode = (lModeIndex == 0) ? TRACE_MODES[i] :
                                         (lModeIndex == 1) ? RLTimeSeriesLineMode::Spline :
                                         (lModeIndex == 2) ? RLTimeSeriesLineMode::Linear :
                                                             RLTimeSeriesLineMode::Raw;
            switch (lMode) {
                case RLTimeSeriesLineMode::Raw: lModeStr = "Raw"; break;
                case RLTimeSeriesLineMode::Linear: lModeStr = "Linear"; break;
                case RLTimeSeriesLineMode::Spline: lModeStr = "Spline"; break;
            }
            DrawText(TextFormat("%d: %s (%.0f Hz)", i + 1, lModeStr, TRACE_RATES[i]),
                     (int)lLegendX + 25, (int)lLegendY, 12, { 160, 165, 175, 255 });
            lLegendX += 160.0f;
        }

        // Instructions
        DrawText("Keys: 1-5 toggle traces | C clear | ESC exit",
                 SCREEN_WIDTH - 320, SCREEN_HEIGHT - 25, 12, { 100, 105, 115, 255 });

        // FPS
        DrawFPS(SCREEN_WIDTH - 90, 15);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

