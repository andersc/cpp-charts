# RLHeatMap

![RLHeatMap Animation](gifs/RLHeatMap.gif)

A heat map visualization for matrix data with color gradients.

## Features

- Configurable grid resolution
- Multiple update modes (Replace, Accumulate, Decay)
- Custom color gradient stops
- Normalized input coordinates
- GPU-accelerated rendering via textures

## Constructor

```cpp
RLHeatMap(Rectangle aBounds, int aCellsX, int aCellsY);
```

**Parameters:**
- `aBounds` - The rectangle defining the heat map's position and size
- `aCellsX` - Number of horizontal cells in the grid
- `aCellsY` - Number of vertical cells in the grid

## Update Modes

```cpp
enum class RLHeatMapUpdateMode {
    Replace,    // New points replace old values
    Accumulate, // Points add to existing values
    Decay       // Values decay over time (exponential)
};
```

## Style Configuration

```cpp
struct RLHeatMapStyle {
    bool mShowBackground = true;
    Color mBackground{20, 22, 28, 255};
    // Optional outline
    bool mShowBorder = false;
    Color mBorderColor{40, 44, 52, 255};
    float mBorderThickness = 1.0f;
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the heat map bounds |
| `setGrid(int aCellsX, int aCellsY)` | Set grid resolution |
| `setUpdateMode(RLHeatMapUpdateMode aMode)` | Set update mode |
| `setDecayHalfLifeSeconds(float aSeconds)` | Set decay rate (for Decay mode) |
| `setStyle(const RLHeatMapStyle &aStyle)` | Apply a style configuration |
| `setColorStops(const std::vector<Color> &aStops)` | Set gradient colors (3-4 stops) |

### Data

| Method | Description |
|--------|-------------|
| `bool addPoints(const std::vector<Vector2>& rPoints)` | Add points in normalized [-1,1] space. Returns `false` if `rPoints` is empty. |
| `clear()` | Clear all data |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update decay and texture (call each frame) |
| `draw() const` | Draw the heat map |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getCellsX() const` | Get horizontal cell count |
| `getCellsY() const` | Get vertical cell count |
| `getUpdateMode() const` | Get current update mode |

## Complete Example

```cpp
#include "raylib.h"
#include "RLHeatMap.h"
#include <vector>
#include <cmath>

int main() {
    InitWindow(800, 600, "Heat Map Example");
    SetTargetFPS(60);

    // Define heat map area
    Rectangle lBounds = {50, 50, 700, 500};

    // Create heat map with 64x64 grid
    RLHeatMap lHeatMap(lBounds, 64, 64);

    // Set accumulate mode
    lHeatMap.setUpdateMode(RLHeatMapUpdateMode::Accumulate);

    // Set color gradient (cold to hot)
    std::vector<Color> lColors = {
        Color{0, 0, 50, 255},     // Dark blue (cold)
        Color{0, 100, 200, 255},  // Blue
        Color{255, 200, 0, 255},  // Yellow
        Color{255, 50, 0, 255}    // Red (hot)
    };
    lHeatMap.setColorStops(lColors);

    float lTime = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Generate circular pattern of points
        std::vector<Vector2> lPoints;
        for (int i = 0; i < 100; ++i) {
            float lAngle = lTime + (float)i * 0.1f;
            float lRadius = 0.3f + 0.2f * sinf(lTime * 2.0f);
            Vector2 lPoint = {
                lRadius * cosf(lAngle),
                lRadius * sinf(lAngle)
            };
            lPoints.push_back(lPoint);
        }

        // Add points to heat map
        lHeatMap.addPoints(lPoints);

        // Update
        lHeatMap.update(lDt);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw heat map
        lHeatMap.draw();
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Decay Mode Example

Use decay mode for fading heat trails:

```cpp
// Enable decay mode
lHeatMap.setUpdateMode(RLHeatMapUpdateMode::Decay);

// Values halve every 0.5 seconds
lHeatMap.setDecayHalfLifeSeconds(0.5f);

// Points will gradually fade out over time
```

## Color Gradients

Custom color gradients can have 3 or 4 stops:

```cpp
// 3-stop gradient (low, mid, high)
std::vector<Color> lGradient3 = {
    Color{0, 0, 100, 255},   // Low
    Color{0, 255, 0, 255},   // Mid
    Color{255, 0, 0, 255}    // High
};

// 4-stop gradient (more control)
std::vector<Color> lGradient4 = {
    Color{0, 0, 50, 255},    // 0%
    Color{0, 100, 200, 255}, // 33%
    Color{255, 200, 0, 255}, // 66%
    Color{255, 50, 0, 255}   // 100%
};

lHeatMap.setColorStops(lGradient4);
```

## Coordinate System

Points are added in normalized space where:
- X ranges from -1.0 (left) to 1.0 (right)
- Y ranges from -1.0 (bottom) to 1.0 (top)

```cpp
Vector2 lCenter = {0.0f, 0.0f};      // Center of heat map
Vector2 lTopRight = {1.0f, 1.0f};    // Top-right corner
Vector2 lBottomLeft = {-1.0f, -1.0f}; // Bottom-left corner
```

