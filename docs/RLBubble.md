# RLBubble

A bubble chart for raylib with two display modes: Scatter and Gravity.

## Features

- **Scatter Mode**: Bubbles positioned by x,y coordinates with smooth animations
- **Gravity Mode**: Physics-based simulation where bubbles attract and collide
- Smooth transitions between data states
- Size-based bubble scaling
- Customizable colors and styling

## Constructor

```cpp
explicit RLBubble(Rectangle bounds, RLBubbleMode mode = RLBubbleMode::Scatter, const RLBubbleStyle &style = {});
```

**Parameters:**
- `bounds` - The rectangle defining the chart's position and size
- `mode` - `RLBubbleMode::Scatter` or `RLBubbleMode::Gravity`
- `style` - Optional style configuration

## Data Structure

```cpp
struct RLBubblePoint {
    float mX{0.0f};    // Normalized [0,1] x position (scatter mode)
    float mY{0.0f};    // Normalized [0,1] y position
    float mSize{1.0f}; // Arbitrary value; mapped to radius by sizeScale
    Color mColor{80, 180, 255, 255};
};
```

## Modes

```cpp
enum class RLBubbleMode {
    Scatter,  // Bubbles positioned by x,y coordinates
    Gravity   // Physics-based simulation with attraction and collision
};
```

## Style Configuration

```cpp
struct RLBubbleStyle {
    Color mBackground{20, 22, 28, 255};
    Color mAxesColor{70, 75, 85, 255};
    Color mGridColor{40, 44, 52, 255};
    int mGridLines = 4;
    float mSizeScale = 24.0f;   // Pixel radius per sqrt(size)
    float mMinRadius = 3.0f;    // Minimum visual radius
    float mOutline = 2.0f;      // Outline thickness
    Color mOutlineColor{0, 0, 0, 80};
    bool mShowAxes = true;
    bool mSmooth = true;
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle bounds)` | Set the chart bounds |
| `setStyle(const RLBubbleStyle &style)` | Apply a style configuration |
| `setMode(RLBubbleMode mode)` | Switch between Scatter and Gravity modes |

### Data

| Method | Description |
|--------|-------------|
| `setData(const std::vector<RLBubblePoint> &data)` | Set data immediately (no animation) |
| `setTargetData(const std::vector<RLBubblePoint> &data)` | Set target data (animates to new values) |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float dt)` | Update animations/physics (call each frame with delta time) |
| `draw() const` | Draw the chart |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getMode() const` | Get current mode |

## Complete Example

```cpp
#include "raylib.h"
#include "RLBubble.h"
#include <vector>

int main() {
    InitWindow(800, 600, "Bubble Chart Example");
    SetTargetFPS(60);

    // Define chart area
    Rectangle lBounds = {50, 50, 700, 500};

    // Create style
    RLBubbleStyle lStyle;
    lStyle.mBackground = Color{24, 26, 32, 255};
    lStyle.mGridLines = 5;
    lStyle.mSizeScale = 22.0f;
    lStyle.mMinRadius = 4.0f;
    lStyle.mOutline = 2.0f;
    lStyle.mShowAxes = true;

    // Create chart in Scatter mode
    RLBubble lChart(lBounds, RLBubbleMode::Scatter, lStyle);

    // Prepare data
    std::vector<RLBubblePoint> lData;
    lData.push_back({0.2f, 0.3f, 3.0f, BLUE});
    lData.push_back({0.5f, 0.7f, 5.0f, GREEN});
    lData.push_back({0.8f, 0.4f, 2.0f, RED});
    lData.push_back({0.3f, 0.8f, 4.0f, ORANGE});

    // Set initial data
    lChart.setData(lData);

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();

        // Update animation/physics
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

## Gravity Mode Example

In Gravity mode, bubbles attract to the center and collide with each other:

```cpp
// Create chart in Gravity mode
RLBubble lGravityChart(lBounds, RLBubbleMode::Gravity, lStyle);

// Data - positions are less important in gravity mode
// The largest bubble becomes the center of attraction
std::vector<RLBubblePoint> lData;
lData.push_back({0.5f, 0.5f, 10.0f, BLUE});   // Largest - becomes center
lData.push_back({0.2f, 0.3f, 3.0f, GREEN});
lData.push_back({0.8f, 0.7f, 4.0f, RED});

lGravityChart.setData(lData);

// In the update loop, physics simulation runs
lGravityChart.update(lDt);
```

## Animation Example

To animate between data states:

```cpp
// Initial data
lChart.setData(lInitialData);

// Later, animate to new data
lChart.setTargetData(lNewData);

// The chart will smoothly animate:
// - Existing bubbles move to new positions and sizes
// - New bubbles fade in
// - Removed bubbles fade out
```

