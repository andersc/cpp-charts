# RLLogPlot

A dual-view log-log plot with time series display, designed for Allan variance-style analysis.

## Features

- Combined time series and log-log plot views
- Multiple analysis traces with confidence intervals
- Streaming time series data support
- Auto-scaling or manual axis ranges
- Smooth animations

## Constructor

```cpp
explicit RLLogPlot(Rectangle aBounds);
```

**Parameters:**
- `aBounds` - The rectangle defining the plot's total position and size

## Data Structures

### Confidence Interval

```cpp
struct RLLogPlotConfidence {
    float mLowerBound{0.0f};
    float mUpperBound{0.0f};
    bool mEnabled{false};
};
```

### Trace Style

```cpp
struct RLLogPlotTraceStyle {
    Color mLineColor{80, 180, 255, 255};
    float mLineThickness{2.5f};
    bool mShowPoints{true};
    float mPointRadius{4.0f};
    Color mPointColor{0, 0, 0, 0};  // If a==0, derived from line color

    // Confidence interval styling
    bool mShowConfidenceIntervals{true};
    Color mConfidenceColor{0, 0, 0, 0};  // If a==0, derived from line color with transparency
    float mConfidenceAlpha{0.3f};        // Alpha multiplier for confidence regions
    bool mConfidenceAsBars{false};       // true: error bars, false: shaded band
    float mConfidenceBarWidth{8.0f};     // Width of error bar caps (if bars)
};
```

### Trace

```cpp
struct RLLogPlotTrace {
    std::vector<float> mXValues;  // X-axis values (e.g., tau)
    std::vector<float> mYValues;  // Y-axis values (e.g., Allan deviation)
    std::vector<RLLogPlotConfidence> mConfidence;
    RLLogPlotTraceStyle mStyle{};
};
```

## Style Configurations

### Log Plot Style

```cpp
struct RLLogPlotStyle {
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};
    Color mAxesColor{120, 125, 135, 255};
    Color mGridColor{45, 48, 55, 255};
    Color mTextColor{180, 185, 195, 255};
    float mPadding{50.0f};

    bool mShowGrid{true};
    bool mShowMinorGrid{false};
    Color mMinorGridColor{35, 38, 42, 255};

    // Axis ranges (log10 space)
    bool mAutoScaleX{true};
    bool mAutoScaleY{true};
    float mMinLogX{-2.0f};  // 10^-2
    float mMaxLogX{3.0f};   // 10^3
    float mMinLogY{-6.0f};  // 10^-6
    float mMaxLogY{0.0f};   // 10^0

    // Animation
    bool mSmoothAnimate{true};
    float mAnimSpeed{6.0f};

    // Labels
    std::string mTitle{};
    std::string mXAxisLabel{};
    std::string mYAxisLabel{};
    float mFontSize{14.0f};
    float mTitleFontSize{18.0f};
    Font mFont{};                 // Optional custom font; if .baseSize==0 use default
};
```

### Time Series Style

```cpp
struct RLTimeSeriesStyle {
    bool mShowBackground{true};
    Color mBackground{18, 20, 24, 255};
    Color mLineColor{100, 200, 255, 255};
    float mLineThickness{1.5f};
    Color mAxesColor{100, 105, 115, 255};
    Color mGridColor{35, 38, 42, 255};
    Color mTextColor{160, 165, 175, 255};
    float mPadding{40.0f};

    bool mShowGrid{true};
    bool mAutoScaleY{true};
    float mMinY{-1.0f};
    float mMaxY{1.0f};

    // Fill under curve
    bool mFillUnderCurve{false};
    Color mFillColor{100, 200, 255, 60};

    std::string mTitle{};
    std::string mYAxisLabel{};
    float mFontSize{12.0f};
    Font mFont{};                 // Optional custom font; if .baseSize==0 use default
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the total plot bounds |
| `setTimeSeriesHeight(float aHeightFraction)` | Set time series height (0-1, default 0.35) |
| `setLogPlotStyle(const RLLogPlotStyle &aStyle)` | Set log plot style |
| `setTimeSeriesStyle(const RLTimeSeriesStyle &aStyle)` | Set time series style |
| `setWindowSize(size_t aMaxSamples)` | Set max samples to keep |

### Time Series Data

| Method | Description |
|--------|-------------|
| `pushSample(float aValue)` | Add one sample (FIFO) |
| `pushSamples(const std::vector<float> &aValues)` | Add multiple samples |
| `clearTimeSeries()` | Clear all time series data |
| `getTimeSeriesSize() const` | Get current sample count |
| `getWindowSize() const` | Get max window size |

### Log-Log Trace Management

| Method | Description |
|--------|-------------|
| `clearTraces()` | Remove all traces |
| `addTrace(const RLLogPlotTrace &aTrace)` | Add a new trace |
| `setTrace(size_t aIndex, const RLLogPlotTrace &aTrace)` | Replace a trace |
| `updateTraceData(...)` | Update trace data with optional confidence |
| `getTraceCount() const` | Get number of traces |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame) |
| `draw() const` | Draw both plots |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get total bounds |
| `getTimeSeriesBounds() const` | Get time series area bounds |
| `getLogPlotBounds() const` | Get log plot area bounds |
| `getTimeSeries() const` | Get raw time series data |
| `getTraces()` | Get mutable trace list |

## Complete Example

```cpp
#include "raylib.h"
#include "RLLogPlot.h"
#include <cmath>

int main() {
    InitWindow(1200, 800, "Log Plot Example");
    SetTargetFPS(60);

    // Define plot area
    Rectangle lBounds = {20, 20, 1160, 760};

    // Create log plot
    RLLogPlot lPlot(lBounds);
    lPlot.setWindowSize(1000);
    lPlot.setTimeSeriesHeight(0.3f);

    // Add an analysis trace
    RLLogPlotTrace lTrace;
    lTrace.mStyle.mLineColor = Color{80, 200, 255, 255};
    lTrace.mStyle.mShowConfidenceIntervals = true;

    // Example Allan deviation data (tau vs sigma)
    std::vector<float> lTau = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000};
    std::vector<float> lSigma;
    for (float t : lTau) {
        lSigma.push_back(0.001f / sqrtf(t));  // 1/sqrt(tau) slope
    }

    lTrace.mXValues = lTau;
    lTrace.mYValues = lSigma;
    lPlot.addTrace(lTrace);

    float lTime = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Stream time series data
        float lSample = sinf(lTime * 2.0f) + 0.1f * sinf(lTime * 50.0f);
        lPlot.pushSample(lSample);

        // Update
        lPlot.update(lDt);

        BeginDrawing();
        ClearBackground(Color{15, 15, 20, 255});
        
        // Draw plot
        lPlot.draw();
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Confidence Intervals

Add confidence intervals to traces:

```cpp
RLLogPlotTrace lTrace;
lTrace.mXValues = {1, 10, 100, 1000};
lTrace.mYValues = {0.01f, 0.003f, 0.001f, 0.0003f};

// Add confidence intervals
lTrace.mConfidence.resize(4);
lTrace.mConfidence[0] = {0.008f, 0.012f, true};
lTrace.mConfidence[1] = {0.0025f, 0.0035f, true};
lTrace.mConfidence[2] = {0.0008f, 0.0012f, true};
lTrace.mConfidence[3] = {0.00025f, 0.00035f, true};

// Style: shaded band or error bars
lTrace.mStyle.mShowConfidenceIntervals = true;
lTrace.mStyle.mConfidenceAsBars = false;  // Shaded band
```

## Dynamic Analysis Updates

Update analysis traces as new data comes in:

```cpp
// Periodically recompute analysis
if (lShouldUpdate) {
    std::vector<float> lNewTau = computeTau();
    std::vector<float> lNewSigma = computeAllanDeviation();
    std::vector<RLLogPlotConfidence> lNewConf = computeConfidence();

    lPlot.updateTraceData(0, lNewTau, lNewSigma, &lNewConf);
}
```

