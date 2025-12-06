# RLBarChart

A fast, animated bar chart for raylib supporting both vertical and horizontal orientations.

![RLBarChart Animation](gifs/RLBarChart.gif)

## Features

- Smooth animations between data states
- Vertical and horizontal orientations
- Automatic or manual scaling
- Customizable colors, labels, and styling
- Animated bar additions and removals

## Constructor

```cpp
RLBarChart(Rectangle aBounds, RLBarOrientation aOrientation, const RLBarChartStyle &rStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the chart's position and size
- `aOrientation` - `RLBarOrientation::VERTICAL` or `RLBarOrientation::HORIZONTAL`
- `rStyle` - Optional style configuration

## Data Structure

```cpp
struct RLBarData {
    float value{0.0f};              // Bar value
    Color color{80, 180, 255, 255}; // Bar color
    bool showBorder{false};         // Show border around bar
    Color borderColor{0, 0, 0, 120};// Border color
    std::string label;              // Optional label text
};
```

## Style Configuration

```cpp
struct RLBarChartStyle {
    // Background and grid
    bool mShowBackground = true;
    Color mBackground{20, 22, 28, 255};
    bool mShowGrid = false;
    Color mGridColor{40, 44, 52, 255};
    int mGridLines = 4;

    // Bars
    float mPadding = 14.0f;
    float mSpacing = 10.0f;
    float mCornerRadius = 5.0f;
    float mBorderThickness = 2.0f;

    // Labels
    bool mShowLabels = true;
    bool mAutoLabelColor = true;
    Color mLabelColor{230, 230, 235, 255};
    Font mLabelFont{};
    int mLabelFontSize = 18;

    // Scaling & animation
    bool mAutoScale = true;
    float mMinValue = 0.0f;
    float mMaxValue = 100.0f;
    bool mSmoothAnimate = true;
    float mAnimateSpeed = 8.0f;
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the chart bounds |
| `setOrientation(RLBarOrientation aOrientation)` | Set vertical or horizontal orientation |
| `setStyle(const RLBarChartStyle &rStyle)` | Apply a style configuration |
| `setScale(float aMinValue, float aMaxValue)` | Set explicit scale (disables auto-scale) |

### Data

| Method | Description |
|--------|-------------|
| `setData(const std::vector<RLBarData> &rData)` | Set data immediately (no animation) |
| `setTargetData(const std::vector<RLBarData> &rData)` | Set target data (animates to new values) |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame with delta time) |
| `draw() const` | Draw the chart |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getOrientation() const` | Get current orientation |

## Complete Example

```cpp
#include "raylib.h"
#include "RLBarChart.h"
#include <vector>

int main() {
    InitWindow(800, 600, "Bar Chart Example");
    SetTargetFPS(60);

    // Define chart area
    Rectangle lBounds = {50, 50, 700, 500};

    // Create style
    RLBarChartStyle lStyle;
    lStyle.mBackground = Color{24, 26, 32, 255};
    lStyle.mShowGrid = true;
    lStyle.mGridLines = 4;
    lStyle.mSpacing = 12.0f;
    lStyle.mCornerRadius = 8.0f;

    // Create chart
    RLBarChart lChart(lBounds, RLBarOrientation::VERTICAL, lStyle);

    // Prepare data
    std::vector<RLBarData> lData;
    lData.push_back({75.0f, BLUE, false, BLACK, "Q1"});
    lData.push_back({92.0f, GREEN, false, BLACK, "Q2"});
    lData.push_back({68.0f, RED, false, BLACK, "Q3"});
    lData.push_back({85.0f, ORANGE, false, BLACK, "Q4"});

    // Set initial data
    lChart.setData(lData);

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

## Animation Example

To animate between data states, use `setTargetData()`:

```cpp
// Initial data
lChart.setData(lInitialData);

// Later, animate to new data
lChart.setTargetData(lNewData);

// The chart will smoothly animate:
// - Existing bars will animate to new values
// - New bars will fade in
// - Removed bars will fade out
```

