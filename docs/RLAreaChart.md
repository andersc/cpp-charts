# RLAreaChart

![RLAreaChart Animation](gifs/RLAreaChart.gif)

A versatile area chart for raylib supporting overlapped, stacked, and 100% stacked visualization modes with smooth animations.

## Features

- Three visualization modes (Overlapped, Stacked, Percent)
- Smooth data transitions with configurable animation speed
- Multiple series support with individual colors and opacity
- Customizable grid, axes, and labels
- Legend display
- X-axis label support

## Constructor

```cpp
RLAreaChart(Rectangle aBounds, RLAreaChartMode aMode = RLAreaChartMode::STACKED,
            const RLAreaChartStyle& rStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the chart's position and size
- `aMode` - Visualization mode (OVERLAPPED, STACKED, or PERCENT)
- `rStyle` - Optional style configuration

## Chart Modes

```cpp
enum class RLAreaChartMode {
    OVERLAPPED,  // Areas overlap with transparency showing layering
    STACKED,     // Areas stack on top of each other
    PERCENT      // 100% stacked (normalized to percentage)
};
```

### Mode Descriptions

| Mode | Description | Use Case |
|------|-------------|----------|
| `OVERLAPPED` | Series overlap with transparency | Comparing trends across series |
| `STACKED` | Series stack vertically | Showing cumulative totals |
| `PERCENT` | Normalized to 100% | Showing proportions over time |

## Data Structures

### Series Data

```cpp
struct RLAreaSeries {
    std::vector<float> mValues;        // Data points
    Color mColor{80, 180, 255, 255};   // Series color
    std::string mLabel;                 // Legend label
    float mAlpha{0.6f};                 // Fill opacity (0.0 - 1.0)
};
```

## Style Configuration

```cpp
struct RLAreaChartStyle {
    // Background and grid
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};
    bool mShowGrid{true};
    Color mGridColor{40, 44, 52, 255};
    int mGridLines{5};

    // Axes
    Color mAxisColor{180, 180, 180, 255};
    Color mLabelColor{200, 200, 200, 255};

    // Chart area
    float mPadding{40.0f};
    float mLineThickness{2.0f};
    bool mShowPoints{false};
    float mPointRadius{4.0f};

    // Labels
    bool mShowLabels{true};
    Font mLabelFont{};
    int mLabelFontSize{12};

    // Legend
    bool mShowLegend{true};

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{6.0f};
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the chart bounds |
| `setMode(RLAreaChartMode aMode)` | Change visualization mode |
| `setStyle(const RLAreaChartStyle& rStyle)` | Apply a style configuration |
| `setXLabels(const std::vector<std::string>& rLabels)` | Set X-axis labels |

### Data Management

| Method | Description |
|--------|-------------|
| `setData(const std::vector<RLAreaSeries>& rSeries)` | Set data (immediate with entry animation) |
| `setTargetData(const std::vector<RLAreaSeries>& rSeries)` | Set target data (smooth transition) |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame) |
| `draw() const` | Render the chart |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getMode() const` | Get current mode |
| `getMaxValue() const` | Get current maximum Y value |

## Usage Examples

### Basic Stacked Area Chart

```cpp
#include "RLAreaChart.h"

// Create chart
Rectangle lBounds = {50, 50, 600, 400};
RLAreaChart lChart(lBounds, RLAreaChartMode::STACKED);

// Prepare data
std::vector<RLAreaSeries> lData;

RLAreaSeries lSeries1;
lSeries1.mLabel = "Product A";
lSeries1.mColor = Color{80, 180, 255, 255};
lSeries1.mValues = {10, 15, 20, 25, 30, 28, 32};
lData.push_back(lSeries1);

RLAreaSeries lSeries2;
lSeries2.mLabel = "Product B";
lSeries2.mColor = Color{255, 120, 80, 255};
lSeries2.mValues = {8, 12, 18, 22, 18, 20, 24};
lData.push_back(lSeries2);

lChart.setData(lData);

// Main loop
while (!WindowShouldClose()) {
    float lDt = GetFrameTime();
    lChart.update(lDt);
    
    BeginDrawing();
    ClearBackground(RAYWHITE);
    lChart.draw();
    EndDrawing();
}
```

### Overlapped Mode with Transparency

```cpp
// Use overlapped mode for comparing trends
RLAreaChart lChart(lBounds, RLAreaChartMode::OVERLAPPED);

// Set lower alpha for better overlap visibility
for (auto& rSeries : lData) {
    rSeries.mAlpha = 0.5f;
}

lChart.setData(lData);
```

### 100% Stacked (Percent Mode)

```cpp
// Show proportions over time
RLAreaChart lChart(lBounds, RLAreaChartMode::PERCENT);
lChart.setData(lData);  // Values automatically normalized to 100%
```

### Smooth Data Transitions

```cpp
// Initial data
lChart.setData(lInitialData);

// Later, smoothly transition to new data
lChart.setTargetData(lNewData);

// In main loop - animation happens automatically
lChart.update(lDt);
```

### With X-Axis Labels

```cpp
std::vector<std::string> lMonths = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun"
};
lChart.setXLabels(lMonths);
```

### Custom Styling

```cpp
RLAreaChartStyle lStyle;
lStyle.mBackground = Color{15, 18, 24, 255};
lStyle.mShowGrid = true;
lStyle.mGridColor = Color{40, 45, 55, 255};
lStyle.mGridLines = 6;
lStyle.mPadding = 50.0f;
lStyle.mLineThickness = 2.5f;
lStyle.mShowPoints = true;
lStyle.mPointRadius = 4.0f;
lStyle.mShowLegend = true;
lStyle.mAnimateSpeed = 8.0f;

RLAreaChart lChart(lBounds, RLAreaChartMode::STACKED, lStyle);
```

## Animation System

The chart supports smooth animations for:
- **Entry animation**: Data grows from zero when first set
- **Data transitions**: Smooth interpolation between old and new values
- **Scale transitions**: Y-axis scale animates smoothly when data range changes

Animation is controlled by:
- `mSmoothAnimate` - Enable/disable animations
- `mAnimateSpeed` - Higher values = faster transitions (default: 6.0)

```cpp
// Disable animations for immediate updates
lStyle.mSmoothAnimate = false;

// Or make animations faster
lStyle.mAnimateSpeed = 10.0f;
```

## Performance Notes

- Uses `DrawTriangleStrip` for efficient filled area rendering
- Pre-allocates vectors to minimize per-frame allocations
- Suitable for real-time data visualization

## See Also

- [RLTimeSeries](RLTimeSeries.md) - For streaming time-based data
- [RLBarChart](RLBarChart.md) - For categorical comparisons
- [RLScatterPlot](RLScatterPlot.md) - For point-based visualization

