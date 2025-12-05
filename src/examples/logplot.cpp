
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "RLLogPlot.h"
#include "RLTimeSeries.h"

// ============================================================================
// Allan Variance-Style Analysis Utilities
// ============================================================================

// Generate realistic noise with drift for demonstration
static float generateNoiseSample(float aTime, float aNoiseLevel, float aDriftRate) {
    // White noise
    float lWhiteNoise = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f * aNoiseLevel;

    // Pink noise (1/f) approximation using multiple sine waves
    float lPinkNoise = 0.0f;
    for (int i = 1; i <= 5; ++i) {
        float lFreq = (float)i * 0.1f;
        lPinkNoise += sinf(aTime * lFreq + (float)i) / (float)i;
    }
    lPinkNoise *= aNoiseLevel * 0.3f;

    // Drift (random walk component)
    static float lDrift = 0.0f;
    lDrift += ((float)rand() / (float)RAND_MAX - 0.5f) * aDriftRate;
    lDrift *= 0.998f; // Slowly decay drift

    return lWhiteNoise + lPinkNoise + lDrift;
}

// Compute Allan variance-like metric for demonstration
// This is a simplified version for visual demonstration
struct AllanAnalysisResult {
    std::vector<float> tau;          // Averaging times
    std::vector<float> deviation;    // Allan deviation values
    std::vector<RLLogPlotConfidence> confidence;
};

static AllanAnalysisResult computeAllanLikeAnalysis(const std::vector<float>& aData,
                                                     int aMinTau = 1,
                                                     int aMaxTau = 0,
                                                     float aConfidenceScale = 1.2f) {
    AllanAnalysisResult lResult;

    if (aData.size() < 4) return lResult;

    if (aMaxTau <= 0 || aMaxTau > (int)aData.size() / 2) {
        aMaxTau = (int)aData.size() / 2;
    }

    // Generate tau values logarithmically spaced
    std::vector<int> lTauValues;
    int lTau = aMinTau;
    while (lTau <= aMaxTau) {
        lTauValues.push_back(lTau);
        lTau = (int)(lTau * 1.5f + 1);
    }

    for (int lTauVal : lTauValues) {
        if (lTauVal >= (int)aData.size() / 2) break;

        // Compute Allan variance approximation
        float lSum = 0.0f;
        int lCount = 0;

        for (size_t i = 0; i + 2 * lTauVal < aData.size(); i += lTauVal) {
            // Average over tau samples
            float lAvg1 = 0.0f, lAvg2 = 0.0f;
            for (int j = 0; j < lTauVal; ++j) {
                if (i + j < aData.size()) lAvg1 += aData[i + j];
                if (i + lTauVal + j < aData.size()) lAvg2 += aData[i + lTauVal + j];
            }
            lAvg1 /= (float)lTauVal;
            lAvg2 /= (float)lTauVal;

            float lDiff = lAvg2 - lAvg1;
            lSum += lDiff * lDiff;
            lCount++;
        }

        if (lCount > 0) {
            float lVariance = lSum / (2.0f * (float)lCount);
            float lDeviation = sqrtf(lVariance);

            lResult.tau.push_back((float)lTauVal);
            lResult.deviation.push_back(lDeviation);

            // Generate confidence intervals (simplified - scale with sqrt(N))
            float lConfScale = aConfidenceScale / sqrtf((float)lCount);
            RLLogPlotConfidence lConf;
            lConf.mEnabled = true;
            lConf.mLowerBound = lDeviation / (1.0f + lConfScale);
            lConf.mUpperBound = lDeviation * (1.0f + lConfScale);
            lResult.confidence.push_back(lConf);
        }
    }

    return lResult;
}

// ============================================================================
// Demo State and UI
// ============================================================================

struct DemoState {
    float time{ 0.0f };
    float sampleRate{ 120.0f };  // Samples per second
    float timeSinceLastSample{ 0.0f };

    // Noise parameters
    float noiseLevel{ 0.5f };
    float driftRate{ 0.02f };

    // Window configuration
    int windowSize{ 500 };

    // Trace configurations
    bool showConfidence{ true };
    bool autoUpdate{ true };
    float updateInterval{ 0.1f };  // Update analysis every 0.1s
    float timeSinceUpdate{ 0.0f };

    // Multiple analysis configurations
    struct TraceConfig {
        Color color;
        float confidenceScale;
        int minTau;
        bool enabled;
    };

    std::vector<TraceConfig> traceConfigs{
        { Color{100, 200, 255, 255}, 1.2f, 1, true },   // Cyan - main analysis
        { Color{255, 150, 100, 255}, 1.5f, 2, true },   // Orange - different tau range
        { Color{150, 255, 150, 255}, 1.0f, 3, true }    // Green - another configuration
    };

    // Visual effects
    float pulsePhase{ 0.0f };
    bool showStats{ true };
};

static void drawUI(const DemoState& rState, const RLLogPlot& rPlot, size_t aSampleCount) {
    int lY = 10;
    int lFontSize = 16;
    Color lTextColor = Color{200, 210, 220, 255};
    Color lHighlight = Color{100, 200, 255, 255};

    // Title
    DrawText("Real-Time Allan Variance Analysis", 10, lY, 24, lHighlight);
    lY += 35;

    // Stats
    if (rState.showStats) {
        char lBuf[256];

        snprintf(lBuf, sizeof(lBuf), "Samples: %zu / %d",
                aSampleCount, rState.windowSize);
        DrawText(lBuf, 10, lY, lFontSize, lTextColor);
        lY += 22;

        snprintf(lBuf, sizeof(lBuf), "Noise Level: %.3f", rState.noiseLevel);
        DrawText(lBuf, 10, lY, lFontSize, lTextColor);
        lY += 22;

        snprintf(lBuf, sizeof(lBuf), "Drift Rate: %.4f", rState.driftRate);
        DrawText(lBuf, 10, lY, lFontSize, lTextColor);
        lY += 22;

        snprintf(lBuf, sizeof(lBuf), "Traces: %zu active", rPlot.getTraceCount());
        DrawText(lBuf, 10, lY, lFontSize, lTextColor);
        lY += 22;
    }

    // Controls
    lY += 10;
    DrawText("Controls:", 10, lY, lFontSize, lHighlight);
    lY += 22;
    DrawText("[SPACE]  Pause/Resume", 10, lY, lFontSize - 2, lTextColor);
    lY += 20;
    DrawText("[C]      Toggle Confidence", 10, lY, lFontSize - 2, lTextColor);
    lY += 20;
    DrawText("[S]      Toggle Stats", 10, lY, lFontSize - 2, lTextColor);
    lY += 20;
    DrawText("[R]      Reset Data", 10, lY, lFontSize - 2, lTextColor);
    lY += 20;
    DrawText("[UP/DN]  Noise Level", 10, lY, lFontSize - 2, lTextColor);
    lY += 20;
    DrawText("[L/R]    Window Size", 10, lY, lFontSize - 2, lTextColor);
    lY += 20;
    DrawText("[1-3]    Toggle Traces", 10, lY, lFontSize - 2, lTextColor);
}

// ============================================================================
// Main Demo
// ============================================================================

int main() {
    srand((unsigned)time(nullptr));

    const int lScreenW = 1600;
    const int lScreenH = 1000;

    InitWindow(lScreenW, lScreenH, "RLLogPlot - Real-Time Allan Variance Analysis");
    SetTargetFPS(120);

    // Calculate layout for time series and log plot
    const float lTimeSeriesHeight = 0.3f;
    const float lGap = 20.0f;
    Rectangle lTSBounds{ 300, 50, (float)lScreenW - 320, ((float)lScreenH - 70) * lTimeSeriesHeight };
    Rectangle lLogPlotBounds{ 300, lTSBounds.y + lTSBounds.height + lGap,
                               (float)lScreenW - 320, ((float)lScreenH - 70) * (1.0f - lTimeSeriesHeight) - lGap };

    // Create time series visualizer using RLTimeSeries class
    RLTimeSeries lTimeSeries(lTSBounds, 500);

    // Configure time series style
    RLTimeSeriesChartStyle lTSStyle;
    lTSStyle.mBackground = Color{18, 20, 24, 255};
    lTSStyle.mShowGrid = true;
    lTSStyle.mAutoScaleY = true;
    lTSStyle.mAutoScaleMargin = 0.1f;
    lTSStyle.mSmoothScale = true;
    lTSStyle.mScaleSpeed = 4.0f;
    lTimeSeries.setStyle(lTSStyle);

    // Add a trace for the time series
    RLTimeSeriesTraceStyle lTraceStyle;
    lTraceStyle.mColor = Color{100, 200, 255, 255};
    lTraceStyle.mLineThickness = 2.0f;
    lTraceStyle.mLineMode = RLTimeSeriesLineMode::Linear;
    lTraceStyle.mShowPoints = false;
    size_t lTraceIdx = lTimeSeries.addTrace(lTraceStyle);

    // Create log-log plot (RLLogPlot without time series portion)
    RLLogPlot lPlot(lLogPlotBounds);

    // Configure log-log plot style
    RLLogPlotStyle lLogStyle;
    lLogStyle.mBackground = Color{20, 22, 28, 255};
    lLogStyle.mShowGrid = true;
    lLogStyle.mShowMinorGrid = true;
    lLogStyle.mSmoothAnimate = true;
    lLogStyle.mAnimSpeed = 8.0f;
    lLogStyle.mTitle = "Allan Deviation Analysis (Log-Log)";
    lLogStyle.mXAxisLabel = "Averaging Time τ (samples)";
    lLogStyle.mYAxisLabel = "A_dev";
    lLogStyle.mAutoScaleX = true;
    lLogStyle.mAutoScaleY = true;
    lPlot.setLogPlotStyle(lLogStyle);

    // Hide the internal time series portion of RLLogPlot
    lPlot.setTimeSeriesHeight(0.0f);

    // Demo state
    DemoState lState;
    lTimeSeries.setWindowSize(lState.windowSize);

    // Data buffer for Allan analysis (separate from RLTimeSeries visualization)
    std::vector<float> lSampleBuffer;
    lSampleBuffer.reserve(2000);

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();

        // Input handling
        if (IsKeyPressed(KEY_SPACE)) {
            lState.autoUpdate = !lState.autoUpdate;
        }
        if (IsKeyPressed(KEY_C)) {
            lState.showConfidence = !lState.showConfidence;
        }
        if (IsKeyPressed(KEY_S)) {
            lState.showStats = !lState.showStats;
        }
        if (IsKeyPressed(KEY_R)) {
            lTimeSeries.clearTrace(lTraceIdx);
            lSampleBuffer.clear();
            lPlot.clearTraces();
            lState.time = 0.0f;
        }

        // Adjust noise level
        if (IsKeyDown(KEY_UP)) {
            lState.noiseLevel += lDt * 0.5f;
            if (lState.noiseLevel > 2.0f) lState.noiseLevel = 2.0f;
        }
        if (IsKeyDown(KEY_DOWN)) {
            lState.noiseLevel -= lDt * 0.5f;
            if (lState.noiseLevel < 0.01f) lState.noiseLevel = 0.01f;
        }

        // Adjust window size
        if (IsKeyPressed(KEY_RIGHT)) {
            lState.windowSize = (int)(lState.windowSize * 1.5f);
            if (lState.windowSize > 2000) lState.windowSize = 2000;
            lTimeSeries.setWindowSize(lState.windowSize);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            lState.windowSize = (int)(lState.windowSize / 1.5f);
            if (lState.windowSize < 50) lState.windowSize = 50;
            lTimeSeries.setWindowSize(lState.windowSize);
        }

        // Toggle traces
        for (int i = 0; i < 3 && i < (int)lState.traceConfigs.size(); ++i) {
            if (IsKeyPressed(KEY_ONE + i)) {
                lState.traceConfigs[i].enabled = !lState.traceConfigs[i].enabled;
            }
        }

        // Update simulation
        if (lState.autoUpdate) {
            lState.time += lDt;
            lState.timeSinceLastSample += lDt;
            lState.pulsePhase += lDt * 3.0f;

            // Add new samples at specified rate
            float lSamplePeriod = 1.0f / lState.sampleRate;
            while (lState.timeSinceLastSample >= lSamplePeriod) {
                float lSample = generateNoiseSample(lState.time, lState.noiseLevel, lState.driftRate);

                // Push to the RLTimeSeries visualizer
                lTimeSeries.pushSample(lTraceIdx, lSample);

                // Also keep in our sample buffer for Allan analysis
                lSampleBuffer.push_back(lSample);
                // Keep buffer size in check with window size
                while (lSampleBuffer.size() > (size_t)lState.windowSize) {
                    lSampleBuffer.erase(lSampleBuffer.begin());
                }

                lState.timeSinceLastSample -= lSamplePeriod;
            }

            // Update analysis periodically
            lState.timeSinceUpdate += lDt;
            if (lState.timeSinceUpdate >= lState.updateInterval) {
                lState.timeSinceUpdate = 0.0f;

                // Only compute if we have enough data
                if (lSampleBuffer.size() >= 10) {
                    // Clear and rebuild traces
                    lPlot.clearTraces();

                    // Add multiple traces with different configurations
                    for (size_t i = 0; i < lState.traceConfigs.size(); ++i) {
                        if (!lState.traceConfigs[i].enabled) continue;

                        auto& lConfig = lState.traceConfigs[i];

                        // Compute analysis
                        AllanAnalysisResult lAnalysis = computeAllanLikeAnalysis(
                            lSampleBuffer,
                            lConfig.minTau,
                            0,
                            lConfig.confidenceScale
                        );

                        if (!lAnalysis.tau.empty()) {
                            RLLogPlotTrace lTrace;
                            lTrace.mXValues = lAnalysis.tau;
                            lTrace.mYValues = lAnalysis.deviation;
                            lTrace.mConfidence = lAnalysis.confidence;

                            // Style
                            lTrace.mStyle.mLineColor = lConfig.color;
                            lTrace.mStyle.mLineThickness = 3.0f;
                            lTrace.mStyle.mShowPoints = true;
                            lTrace.mStyle.mPointRadius = 5.0f;
                            lTrace.mStyle.mShowConfidenceIntervals = lState.showConfidence;
                            lTrace.mStyle.mConfidenceAsBars = false;  // Shaded bands
                            lTrace.mStyle.mConfidenceAlpha = 0.25f;

                            lPlot.addTrace(lTrace);
                        }
                    }
                }
            }
        }

        // Update plot animation
        lPlot.update(lDt);

        // Drawing
        BeginDrawing();
        ClearBackground(Color{15, 16, 20, 255});

        // Draw time series (RLTimeSeries)
        lTimeSeries.update(lDt);
        lTimeSeries.draw();

        // Draw log plot system
        lPlot.draw();

        // Draw UI overlay
        drawUI(lState, lPlot, lSampleBuffer.size());

        // Visual pulse indicator when active
        if (lState.autoUpdate) {
            float lPulse = (sinf(lState.pulsePhase) + 1.0f) * 0.5f;
            Color lPulseColor = Color{
                100,
                (unsigned char)(150 + 105 * lPulse),
                255,
                (unsigned char)(100 + 155 * lPulse)
            };
            DrawCircle(280, 70, 8.0f + 4.0f * lPulse, lPulseColor);
            DrawText("LIVE", 230, 62, 16, lPulseColor);
        } else {
            DrawText("PAUSED", 205, 62, 16, Color{150, 150, 150, 200});
            DrawCircle(280, 70, 8.0f, Color{150, 150, 150, 200});
        }

        // FPS counter
        int lFPS = GetFPS();
        Color lFPSColor = lFPS >= 60 ? Color{100, 255, 100, 200} : Color{255, 200, 100, 200};
        char lFPSBuf[32];
        snprintf(lFPSBuf, sizeof(lFPSBuf), "FPS: %d", lFPS);
        DrawText(lFPSBuf, lScreenW - 100, 10, 16, lFPSColor);

        // Trace legend
        int lLegendY = lScreenH - 120;
        DrawText("Active Traces:", 10, lLegendY, 14, Color{180, 190, 200, 255});
        lLegendY += 20;
        for (size_t i = 0; i < lState.traceConfigs.size(); ++i) {
            const auto& lConfig = lState.traceConfigs[i];
            Color lColor = lConfig.enabled ? lConfig.color : Color{80, 80, 80, 150};

            DrawCircle(20, lLegendY + 8, 6.0f, lColor);

            char lLabel[64];
            snprintf(lLabel, sizeof(lLabel), "[%d] Trace %zu (tau min=%d, conf=%.1f)",
                    (int)(i + 1), i + 1, lConfig.minTau, lConfig.confidenceScale);
            DrawText(lLabel, 35, lLegendY, 12, lColor);
            lLegendY += 18;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

