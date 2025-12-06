# RLPieChart

![RLPieChart Animation](gifs/RLPieChart.gif)

An animated pie/donut chart for raylib.

## Features

- Solid pie or donut (hollow) style
- Smooth slice animations
- Value-based automatic slice sizing
- Color and visibility animations
- Optional labels

## Constructor

```cpp
explicit RLPieChart(Rectangle aBounds, const RLPieChartStyle &aStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the chart's position and size
- `aStyle` - Optional style configuration

## Data Structure

```cpp
struct RLPieSliceData {
    float mValue{0.0f};              // Slice value (proportional to total)
    Color mColor{80, 180, 255, 255}; // Slice color
    std::string mLabel;              // Optional label
};
```

## Style Configuration

```cpp
struct RLPieChartStyle {
    // Background
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};

    // Layout
    float mPadding{8.0f};  // Inner padding inside bounds

    // Animation
    bool mSmoothAnimate{true};
    float mAngleSpeed{8.0f};  // Approach speed for angles (1/s)
    float mFadeSpeed{8.0f};   // Approach speed for visibility (1/s)
    float mColorSpeed{6.0f};  // Color blend speed (1/s)
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the chart bounds |
| `setStyle(const RLPieChartStyle &aStyle)` | Apply a style configuration |
| `setHollowFactor(float aFactor)` | Set hollow factor (0=solid, 0.5=donut) |
| `getHollowFactor() const` | Get current hollow factor |

### Data

| Method | Description |
|--------|-------------|
| `setData(const std::vector<RLPieSliceData> &aData)` | Set data immediately |
| `setTargetData(const std::vector<RLPieSliceData> &aData)` | Set target data (animates) |

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
#include "RLPieChart.h"
#include <vector>

int main() {
    InitWindow(800, 600, "Pie Chart Example");
    SetTargetFPS(60);

    // Define chart area
    Rectangle lBounds = {150, 50, 500, 500};

    // Create style
    RLPieChartStyle lStyle;
    lStyle.mBackground = Color{24, 26, 32, 255};
    lStyle.mPadding = 16.0f;
    lStyle.mAngleSpeed = 8.0f;
    lStyle.mFadeSpeed = 8.0f;

    // Create chart
    RLPieChart lChart(lBounds, lStyle);

    // Prepare data
    std::vector<RLPieSliceData> lData;
    lData.push_back({30.0f, BLUE, "Sales"});
    lData.push_back({25.0f, GREEN, "Marketing"});
    lData.push_back({20.0f, RED, "R&D"});
    lData.push_back({15.0f, ORANGE, "Support"});
    lData.push_back({10.0f, PURPLE, "Other"});

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

## Donut Chart

Create a donut chart by setting a hollow factor:

```cpp
RLPieChart lDonut(lBounds, lStyle);

// Set hollow factor (0.0 = solid, 1.0 = fully hollow)
lDonut.setHollowFactor(0.5f);  // 50% hollow

lDonut.setData(lData);
```

## Animation Example

To animate between data states:

```cpp
// Initial data
lChart.setData(lInitialData);

// Later, animate to new data
std::vector<RLPieSliceData> lNewData;
lNewData.push_back({40.0f, BLUE, "Sales"});     // Changed value
lNewData.push_back({20.0f, GREEN, "Marketing"});
lNewData.push_back({25.0f, RED, "R&D"});        // Changed value
lNewData.push_back({15.0f, ORANGE, "Support"});
// "Other" slice removed - will fade out

lChart.setTargetData(lNewData);

// The chart will smoothly animate:
// - Slice angles will adjust to new proportions
// - Colors will blend if changed
// - Removed slices will fade out
// - New slices will fade in
```

## Dynamic Updates

Update slices dynamically:

```cpp
float lTimer = 0.0f;

while (!WindowShouldClose()) {
    float lDt = GetFrameTime();
    lTimer += lDt;

    // Update values every 3 seconds
    if (lTimer > 3.0f) {
        lTimer = 0.0f;
        
        // Generate new random values
        std::vector<RLPieSliceData> lNewData;
        lNewData.push_back({10.0f + rand() % 40, BLUE, "A"});
        lNewData.push_back({10.0f + rand() % 40, GREEN, "B"});
        lNewData.push_back({10.0f + rand() % 40, RED, "C"});
        
        lChart.setTargetData(lNewData);
    }

    lChart.update(lDt);
    
    // ... draw
}
```

