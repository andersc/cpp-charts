# RLScatterPlot

![RLScatterPlot Animation](gifs/RLScatterPlot.gif)

A high-performance scatter plot for raylib with multiple series support and spline interpolation.

## Features

- Single or multiple data series
- Linear or Catmull-Rom spline line modes
- Point markers with customizable styling
- Animated data transitions
- Auto-scaling or manual axis ranges

## Constructor

```cpp
explicit RLScatterPlot(Rectangle aBounds, const RLScatterPlotStyle &aStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the chart's position and size
- `aStyle` - Optional style configuration

## Data Structures

### Line Mode

```cpp
enum class RLScatterLineMode {
    None,   // Scatter-only (no connecting line)
    Linear, // Straight line segments
    Spline  // Catmull-Rom spline interpolation
};
```

### Series Style

```cpp
struct RLScatterSeriesStyle {
    Color mLineColor{80, 180, 255, 255};
    float mLineThickness{2.0f};
    RLScatterLineMode mLineMode{RLScatterLineMode::Linear};

    // Points
    bool mShowPoints{true};
    Color mPointColor{0, 0, 0, 0};  // If a==0, derived from line color
    float mPointSizePx{0.0f};       // If <=0, derived from thickness * pointScale
    float mPointScale{1.5f};        // Radius = max(1, thickness * pointScale)
};
```

### Series

```cpp
struct RLScatterSeries {
    std::vector<Vector2> mData;       // Data points
    std::vector<Vector2> mTargetData; // For animation
    RLScatterSeriesStyle mStyle{};
};
```

## Style Configuration

```cpp
struct RLScatterPlotStyle {
    // Background and grid/axes
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};
    bool mShowAxes{true};
    Color mAxesColor{70, 75, 85, 255};
    bool mShowGrid{false};
    Color mGridColor{40, 44, 52, 255};
    int mGridLines{4};

    // Padding inside bounds (for labels if any, or just breathing room)
    float mPadding{10.0f};

    // Scaling
    bool mAutoScale{true};
    float mMinX{0.0f};
    float mMaxX{1.0f};
    float mMinY{0.0f};
    float mMaxY{1.0f};

    // Spline quality (higher = smoother, more points). Base target pixels per segment.
    float mSplinePixels{6.0f};  // Approximate pixels between samples

    // Animation
    bool mSmoothAnimate{true};
    float mMoveSpeed{8.0f};   // Position approach speed (1/s)
    float mFadeSpeed{6.0f};   // Visibility fade speed (1/s)
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the chart bounds |
| `setStyle(const RLScatterPlotStyle &aStyle)` | Apply a style configuration |
| `setScale(float aMinX, float aMaxX, float aMinY, float aMaxY)` | Set explicit scale |

### Series Management

| Method | Description |
|--------|-------------|
| `clearSeries()` | Remove all series |
| `addSeries(const RLScatterSeries &aSeries)` | Add a series (returns index) |
| `setSeries(size_t aIndex, const RLScatterSeries &aSeries)` | Replace a series |
| `seriesCount() const` | Get number of series |

### Single-Series Convenience

| Method | Description |
|--------|-------------|
| `setSingleSeries(const std::vector<Vector2> &aData, const RLScatterSeriesStyle &aStyle = {})` | Set single series data |
| `setSingleSeriesTargetData(const std::vector<Vector2> &aData)` | Animate single series to new data |

### Animated Updates

| Method | Description |
|--------|-------------|
| `setSeriesTargetData(size_t aIndex, const std::vector<Vector2> &aData)` | Animate series to new data |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame) |
| `draw() const` | Draw the chart |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |

## Complete Example

```cpp
#include "raylib.h"
#include "RLScatterPlot.h"
#include <vector>
#include <cmath>

int main() {
    InitWindow(800, 600, "Scatter Plot Example");
    SetTargetFPS(60);

    // Define chart area
    Rectangle lBounds = {50, 50, 700, 500};

    // Create style
    RLScatterPlotStyle lStyle;
    lStyle.mBackground = Color{24, 26, 32, 255};
    lStyle.mShowGrid = true;
    lStyle.mGridLines = 5;

    // Create chart
    RLScatterPlot lChart(lBounds, lStyle);

    // Generate sine wave data
    std::vector<Vector2> lData;
    for (int i = 0; i <= 100; ++i) {
        float lX = (float)i / 100.0f;
        float lY = 0.5f + 0.4f * sinf(lX * 6.28f);
        lData.push_back({lX, lY});
    }

    // Set series style
    RLScatterSeriesStyle lSeriesStyle;
    lSeriesStyle.mLineColor = Color{80, 200, 255, 255};
    lSeriesStyle.mLineThickness = 2.5f;
    lSeriesStyle.mLineMode = RLScatterLineMode::Spline;
    lSeriesStyle.mShowPoints = true;

    // Set data
    lChart.setSingleSeries(lData, lSeriesStyle);

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();

        // Update animation
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

## Multiple Series

Add multiple series with different styles:

```cpp
RLScatterPlot lChart(lBounds, lStyle);

// Series 1: Spline line
RLScatterSeries lSeries1;
lSeries1.mData = lSineData;
lSeries1.mStyle.mLineColor = BLUE;
lSeries1.mStyle.mLineMode = RLScatterLineMode::Spline;
lChart.addSeries(lSeries1);

// Series 2: Linear line
RLScatterSeries lSeries2;
lSeries2.mData = lCosineData;
lSeries2.mStyle.mLineColor = RED;
lSeries2.mStyle.mLineMode = RLScatterLineMode::Linear;
lChart.addSeries(lSeries2);

// Series 3: Points only
RLScatterSeries lSeries3;
lSeries3.mData = lScatterData;
lSeries3.mStyle.mLineColor = GREEN;
lSeries3.mStyle.mLineMode = RLScatterLineMode::None;
lSeries3.mStyle.mShowPoints = true;
lChart.addSeries(lSeries3);
```

## Animation Example

Animate between data states:

```cpp
// Initial data
lChart.setSingleSeries(lSineData, lStyle);

// Later, animate to new data
std::vector<Vector2> lNewData;
for (int i = 0; i <= 100; ++i) {
    float lX = (float)i / 100.0f;
    float lY = 0.5f + 0.4f * cosf(lX * 6.28f);  // Changed to cosine
    lNewData.push_back({lX, lY});
}

lChart.setSingleSeriesTargetData(lNewData);

// Points will smoothly animate to new positions
```

## Line Modes

### None (Scatter Only)

```cpp
lSeriesStyle.mLineMode = RLScatterLineMode::None;
lSeriesStyle.mShowPoints = true;
```

### Linear

```cpp
lSeriesStyle.mLineMode = RLScatterLineMode::Linear;
```

### Spline (Smooth Curves)

```cpp
lSeriesStyle.mLineMode = RLScatterLineMode::Spline;

// Adjust spline quality in chart style
lChartStyle.mSplinePixels = 4.0f;  // Lower = smoother, more samples
```

