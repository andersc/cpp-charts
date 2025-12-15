# RLRadarChart

A versatile radar/spider chart for raylib supporting single and multiple series with smooth animations, configurable axes, and customizable styling.

## Features

- Single and multi-series radar/spider charts
- Smooth animation on data changes
- Series add/remove with fade in/out effects
- Per-axis or global value normalization
- Configurable spider web grid with rings and spokes
- Axis labels with smart positioning
- Optional filled polygons with transparency
- Point markers at data vertices
- Legend support
- Full styling customization

## Constructor

```cpp
RLRadarChart(Rectangle aBounds, const RLRadarChartStyle& rStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the chart's position and size
- `rStyle` - Optional style configuration

## Data Structures

### Axis Definition

```cpp
struct RLRadarAxis {
    std::string mLabel;         // Axis label text
    float mMin{0.0f};           // Minimum value for this axis
    float mMax{100.0f};         // Maximum value for this axis
};
```

### Series Definition

```cpp
struct RLRadarSeries {
    std::string mLabel;                         // Series name (for legend)
    std::vector<float> mValues;                 // One value per axis
    Color mLineColor{80, 180, 255, 255};        // Outline color
    Color mFillColor{80, 180, 255, 80};         // Fill color with alpha
    float mLineThickness{2.0f};                 // Line width
    bool mShowFill{true};                       // Enable filled polygon
    bool mShowMarkers{true};                    // Enable point markers
    float mMarkerScale{1.5f};                   // Marker radius = lineThickness * markerScale
};
```

### Normalization Mode

```cpp
enum class RLRadarNormMode {
    GLOBAL,     // Use global min/max across all axes
    PER_AXIS    // Use per-axis min/max ranges
};
```

| Mode | Description | Use Case |
|------|-------------|----------|
| `GLOBAL` | Single scale for all axes | When all axes measure the same metric |
| `PER_AXIS` | Independent scale per axis | When axes have different value ranges |

## Style Configuration

```cpp
struct RLRadarChartStyle {
    // Background
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};

    // Grid (spider web)
    bool mShowGrid{true};
    int mGridRings{5};                      // Number of concentric rings
    Color mGridColor{50, 55, 65, 255};
    float mGridThickness{1.0f};

    // Axis lines (radial spokes)
    bool mShowAxes{true};
    Color mAxisColor{70, 75, 85, 255};
    float mAxisThickness{1.0f};

    // Labels
    bool mShowLabels{true};
    Color mLabelColor{180, 190, 210, 255};
    Font mLabelFont{};
    int mLabelFontSize{12};
    float mLabelOffset{12.0f};              // Distance from chart edge to labels

    // Legend
    bool mShowLegend{true};
    float mLegendPadding{8.0f};

    // Chart area
    float mPadding{60.0f};                  // Padding from bounds to chart area

    // Normalization
    RLRadarNormMode mNormMode{RLRadarNormMode::GLOBAL};

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{6.0f};              // Value interpolation speed
    float mFadeSpeed{4.0f};                 // Fade in/out speed for series
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the chart bounds |
| `setStyle(const RLRadarChartStyle& rStyle)` | Apply a style configuration |
| `setAxes(const std::vector<RLRadarAxis>& rAxes)` | Set axes with full configuration |
| `setAxes(const std::vector<std::string>& rLabels, float aMin, float aMax)` | Set axes with labels and uniform range |

### Series Management

| Method | Description |
|--------|-------------|
| `addSeries(const RLRadarSeries& rSeries)` | Add a new series (animates in) |
| `setSeriesData(size_t aIndex, const std::vector<float>& rValues)` | Update series values (animates) |
| `setSeriesData(size_t aIndex, const RLRadarSeries& rSeries)` | Update full series data |
| `removeSeries(size_t aIndex)` | Remove a series (animates out) |
| `clearSeries()` | Remove all series immediately |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame) |
| `draw() const` | Render the chart |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getAxisCount() const` | Get number of axes |
| `getSeriesCount() const` | Get number of active series |

## Usage Examples

### Basic Single-Series Radar Chart

```cpp
#include "RLRadarChart.h"

// Create chart
Rectangle lBounds = {50, 50, 500, 500};
RLRadarChart lChart(lBounds);

// Set up axes
std::vector<std::string> lLabels = {"Strength", "Speed", "Defense", "Magic", "Stamina"};
lChart.setAxes(lLabels, 0.0f, 100.0f);

// Add a series
RLRadarSeries lSeries;
lSeries.mLabel = "Player Stats";
lSeries.mValues = {80.0f, 65.0f, 70.0f, 45.0f, 85.0f};
lSeries.mLineColor = Color{80, 180, 255, 255};
lSeries.mFillColor = Color{80, 180, 255, 60};
lSeries.mShowFill = true;
lSeries.mShowMarkers = true;
lChart.addSeries(lSeries);

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

### Multi-Series Comparison

```cpp
RLRadarChart lChart(lBounds);

std::vector<std::string> lLabels = {
    "Performance", "Reliability", "Cost", "Features", "Support"
};
lChart.setAxes(lLabels, 0.0f, 100.0f);

// Add multiple series for comparison
RLRadarSeries lProduct1;
lProduct1.mLabel = "Product A";
lProduct1.mValues = {85.0f, 70.0f, 60.0f, 90.0f, 75.0f};
lProduct1.mLineColor = Color{80, 180, 255, 255};
lProduct1.mFillColor = Color{80, 180, 255, 40};
lChart.addSeries(lProduct1);

RLRadarSeries lProduct2;
lProduct2.mLabel = "Product B";
lProduct2.mValues = {70.0f, 90.0f, 80.0f, 65.0f, 85.0f};
lProduct2.mLineColor = Color{255, 120, 80, 255};
lProduct2.mFillColor = Color{255, 120, 80, 40};
lChart.addSeries(lProduct2);
```

### Per-Axis Normalization

```cpp
// Use when axes have different value ranges
std::vector<RLRadarAxis> lAxes = {
    {"Speed (mph)", 0.0f, 200.0f},
    {"Weight (kg)", 0.0f, 3000.0f},
    {"Price ($K)", 0.0f, 100.0f},
    {"Efficiency (%)", 0.0f, 100.0f},
    {"Safety (1-10)", 0.0f, 10.0f}
};
lChart.setAxes(lAxes);

RLRadarChartStyle lStyle;
lStyle.mNormMode = RLRadarNormMode::PER_AXIS;
lChart.setStyle(lStyle);
```

### Smooth Data Transitions

```cpp
// Initial data
RLRadarSeries lSeries;
lSeries.mValues = {50.0f, 50.0f, 50.0f, 50.0f, 50.0f};
lChart.addSeries(lSeries);

// Later, update with new values - animates smoothly
std::vector<float> lNewValues = {80.0f, 65.0f, 90.0f, 45.0f, 70.0f};
lChart.setSeriesData(0, lNewValues);

// In main loop
lChart.update(lDt);  // Animation happens automatically
```

### Dynamic Series Add/Remove

```cpp
// Add a new series (fades in)
RLRadarSeries lNewSeries;
lNewSeries.mLabel = "New Entry";
lNewSeries.mValues = {60.0f, 70.0f, 80.0f, 90.0f, 100.0f};
lNewSeries.mLineColor = Color{120, 220, 120, 255};
lChart.addSeries(lNewSeries);

// Remove a series (fades out and shrinks to center)
lChart.removeSeries(0);

// Series will animate out over several frames
```

### Custom Styling

```cpp
RLRadarChartStyle lStyle;
lStyle.mBackground = Color{15, 18, 24, 255};
lStyle.mShowGrid = true;
lStyle.mGridRings = 6;
lStyle.mGridColor = Color{40, 45, 55, 255};
lStyle.mGridThickness = 1.0f;
lStyle.mAxisColor = Color{60, 65, 75, 255};
lStyle.mAxisThickness = 1.5f;
lStyle.mLabelColor = Color{180, 190, 210, 255};
lStyle.mLabelFontSize = 14;
lStyle.mLabelOffset = 15.0f;
lStyle.mPadding = 70.0f;
lStyle.mShowLegend = true;
lStyle.mAnimateSpeed = 8.0f;
lStyle.mFadeSpeed = 5.0f;

RLRadarChart lChart(lBounds, lStyle);
```

## Animation System

The chart supports smooth animations for:

- **Entry animation**: New series fade in from center
- **Data transitions**: Polygon vertices smoothly morph to new positions
- **Exit animation**: Removed series fade out and shrink toward center
- **Color transitions**: Series colors interpolate smoothly

Animation is controlled by:
- `mSmoothAnimate` - Enable/disable all animations
- `mAnimateSpeed` - Speed of value interpolation (higher = faster)
- `mFadeSpeed` - Speed of series fade in/out effects

```cpp
// Disable animations for immediate updates
lStyle.mSmoothAnimate = false;

// Or make animations faster
lStyle.mAnimateSpeed = 10.0f;
lStyle.mFadeSpeed = 6.0f;
```

## Performance Notes

- Axis angles and ring geometry are precomputed and cached
- Per-series vertex positions are cached and only recomputed when values change
- Triangle fan rendering used for efficient filled polygons
- Minimal per-frame allocations after initialization
- Suitable for real-time dashboards and data visualization

## See Also

- [RLAreaChart](RLAreaChart.md) - For time-based series comparison
- [RLScatterPlot](RLScatterPlot.md) - For 2D point-based visualization
- [RLPieChart](RLPieChart.md) - For proportional data display

