# RLTimeSeries

![RLTimeSeries Animation](gifs/RLTimeSeries.gif)

A high-performance streaming time series visualizer for raylib with multiple trace support.

## Features

- Multiple overlapping traces
- Ring buffer for efficient memory usage
- Build-up and scroll behavior
- Multiple line modes (Raw, Linear, Spline)
- Auto-scaling with smooth transitions

## Constructor

```cpp
explicit RLTimeSeries(Rectangle aBounds, size_t aWindowSize = 500);
```

**Parameters:**
- `aBounds` - The rectangle defining the chart's position and size
- `aWindowSize` - Maximum number of samples to keep per trace

## Data Structures

### Line Mode

```cpp
enum class RLTimeSeriesLineMode {
    Raw,    // Simple connected line segments
    Linear, // Points with linear connecting lines
    Spline  // Points with Catmull-Rom spline interpolation
};
```

### Trace Style

```cpp
struct RLTimeSeriesTraceStyle {
    Color mColor{80, 180, 255, 255};
    float mLineThickness{2.0f};
    RLTimeSeriesLineMode mLineMode{RLTimeSeriesLineMode::Linear};
    bool mShowPoints{false};
    float mPointRadius{3.0f};
    bool mVisible{true};
};
```

## Style Configuration

```cpp
struct RLTimeSeriesChartStyle {
    // Background
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};

    // Grid and axes
    bool mShowGrid{true};
    Color mGridColor{40, 44, 52, 255};
    int mGridLinesX{8};
    int mGridLinesY{4};
    bool mShowAxes{true};
    Color mAxesColor{70, 75, 85, 255};

    // Padding inside bounds
    float mPadding{10.0f};

    // Y-axis scaling
    bool mAutoScaleY{true};
    float mMinY{-1.0f};
    float mMaxY{1.0f};
    float mAutoScaleMargin{0.1f};  // 10% margin above/below data range

    // Spline quality (pixels per segment)
    float mSplinePixels{4.0f};

    // Smooth scale transitions
    bool mSmoothScale{true};
    float mScaleSpeed{4.0f};
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the chart bounds |
| `setStyle(const RLTimeSeriesChartStyle &aStyle)` | Apply a style configuration |
| `setWindowSize(size_t aWindowSize)` | Set maximum samples per trace |
| `getWindowSize() const` | Get window size |

### Trace Management

| Method | Description |
|--------|-------------|
| `addTrace(const RLTimeSeriesTraceStyle &aStyle = {})` | Add a new trace (returns index) |
| `setTraceStyle(size_t aIndex, const RLTimeSeriesTraceStyle &aStyle)` | Set trace style |
| `setTraceVisible(size_t aIndex, bool aVisible)` | Show/hide a trace |
| `clearTrace(size_t aIndex)` | Clear samples from a trace |
| `clearAllTraces()` | Clear all trace data |
| `getTraceCount() const` | Get number of traces |
| `getTraceSampleCount(size_t aIndex) const` | Get sample count for a trace |

### Sample Input

| Method | Description |
|--------|-------------|
| `pushSample(size_t aTraceIndex, float aValue)` | Add one sample to a trace |
| `bool pushSamples(size_t aTraceIndex, const std::vector<float>& rValues)` | Add multiple samples. Returns `false` if `rValues` is empty or `aTraceIndex` is invalid. |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update scale transitions (call each frame) |
| `draw() const` | Draw the chart |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getPlotArea() const` | Get the actual plot area (minus padding) |

## Complete Example

```cpp
#include "raylib.h"
#include "RLTimeSeries.h"
#include <cmath>

int main() {
    InitWindow(1200, 400, "Time Series Example");
    SetTargetFPS(60);

    // Define chart area
    Rectangle lBounds = {20, 20, 1160, 360};

    // Create chart with 500 sample window
    RLTimeSeries lChart(lBounds, 500);

    // Configure style
    RLTimeSeriesChartStyle lStyle;
    lStyle.mBackground = Color{20, 22, 28, 255};
    lStyle.mShowGrid = true;
    lStyle.mGridLinesX = 10;
    lStyle.mGridLinesY = 4;
    lStyle.mAutoScaleY = true;
    lChart.setStyle(lStyle);

    // Add a trace
    RLTimeSeriesTraceStyle lTraceStyle;
    lTraceStyle.mColor = Color{80, 200, 255, 255};
    lTraceStyle.mLineThickness = 2.0f;
    lTraceStyle.mLineMode = RLTimeSeriesLineMode::Linear;
    size_t lTraceIdx = lChart.addTrace(lTraceStyle);

    float lTime = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Generate sample (sine wave with noise)
        float lSample = sinf(lTime * 2.0f) + 0.1f * sinf(lTime * 20.0f);
        
        // Push sample to trace
        lChart.pushSample(lTraceIdx, lSample);

        // Update
        lChart.update(lDt);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw chart
        lChart.draw();
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Multiple Traces

Display multiple overlapping traces:

```cpp
RLTimeSeries lChart(lBounds, 1000);

// Add trace 1 (blue)
RLTimeSeriesTraceStyle lStyle1;
lStyle1.mColor = BLUE;
lStyle1.mLineThickness = 2.0f;
size_t lTrace1 = lChart.addTrace(lStyle1);

// Add trace 2 (red)
RLTimeSeriesTraceStyle lStyle2;
lStyle2.mColor = RED;
lStyle2.mLineThickness = 2.0f;
size_t lTrace2 = lChart.addTrace(lStyle2);

// In update loop, push to both traces
lChart.pushSample(lTrace1, lSineValue);
lChart.pushSample(lTrace2, lCosineValue);
```

## Line Modes

### Raw Mode

Simple connected segments, fastest rendering:

```cpp
lTraceStyle.mLineMode = RLTimeSeriesLineMode::Raw;
```

### Linear Mode

Points with linear interpolation:

```cpp
lTraceStyle.mLineMode = RLTimeSeriesLineMode::Linear;
lTraceStyle.mShowPoints = true;
lTraceStyle.mPointRadius = 3.0f;
```

### Spline Mode

Smooth curves using Catmull-Rom interpolation:

```cpp
lTraceStyle.mLineMode = RLTimeSeriesLineMode::Spline;

// Adjust quality in chart style
lChartStyle.mSplinePixels = 3.0f;  // Lower = smoother
```

## Streaming Data

The chart uses a ring buffer for efficient memory usage:

```cpp
// Window size determines how many samples are kept
lChart.setWindowSize(1000);  // Keep last 1000 samples

// Push samples as they arrive
while (lDataAvailable) {
    float lValue = lGetNextSample();
    lChart.pushSample(lTraceIdx, lValue);
}

// Old samples are automatically discarded when window is full
```

## Show/Hide Traces

Toggle trace visibility:

```cpp
// Hide trace
lChart.setTraceVisible(lTraceIdx, false);

// Show trace
lChart.setTraceVisible(lTraceIdx, true);
```

