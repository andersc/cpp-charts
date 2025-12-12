# RLHeatMap3D

A scientific 3D plot visualization that renders a grid of scalar values as either a 3D surface mesh or scatter points, with a characteristic axis box, floor grid, and transparent back walls.

![3D Heat Map Example](gifs/RLHeatMap3D.gif)

## Features

- Two rendering modes: **Surface** (connected mesh) and **Scatter** (individual points)
- Scientific plot axis box with edges, tick marks, and labels
- Floor grid on the bottom plane
- Semi-transparent back walls that fade based on viewing angle
- Configurable color palette (3-4 color stops)
- Smooth transitions when data changes
- Support for both static and live-updating data
- **Live streaming data** support with smooth interpolation
- **Partial region updates** for efficient localized data changes
- Efficient vertex-only updates (no full mesh rebuild)
- Optional wireframe overlay

## Constructor

```cpp
RLHeatMap3D();
RLHeatMap3D(int aWidth, int aHeight);
```

**Parameters:**
- `aWidth` - Number of grid points in X direction (minimum 2)
- `aHeight` - Number of grid points in Y direction (minimum 2)

**Note:** Grid sizes larger than 256×256 (65536 cells) will trigger a performance warning via `TraceLog`.

## Rendering Modes

```cpp
enum class RLHeatMap3DMode {
    Surface,    // Connected surface mesh
    Scatter     // Individual points/spheres in 3D space
};
```

## Style Configuration

```cpp
struct RLHeatMap3DStyle {
    // Render mode
    RLHeatMap3DMode mMode = RLHeatMap3DMode::Surface;

    // Smoothing speed for transitions (higher = faster)
    float mSmoothingSpeed = 5.0f;

    // Surface mode options
    bool mShowWireframe = true;
    Color mWireframeColor{80, 80, 80, 200};
    float mSurfaceOpacity = 0.85f;

    // Scatter mode options
    float mPointSize = 0.15f;
    bool mShowPointOutline = false;

    // Axis box styling
    bool mShowAxisBox = true;
    Color mAxisColor{120, 120, 130, 255};
    Color mGridColor{60, 60, 70, 150};
    Color mBackWallColor{40, 44, 52, 80};  // Semi-transparent back walls
    float mAxisLineWidth = 1.5f;
    int mGridDivisions = 10;

    // Floor grid
    bool mShowFloorGrid = true;
    Color mFloorGridColor{50, 55, 65, 120};

    // Tick marks
    bool mShowTicks = true;
    int mTickCount = 5;
    Color mTickColor{150, 150, 160, 255};
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setGridSize(int aWidth, int aHeight)` | Set grid resolution |
| `setMode(RLHeatMap3DMode aMode)` | Set render mode (Surface/Scatter) |
| `setPalette(Color a, Color b, Color c)` | Set 3-color gradient |
| `setPalette(Color a, Color b, Color c, Color d)` | Set 4-color gradient |
| `setValueRange(float aMin, float aMax)` | Set manual value range (disables auto) |
| `setAutoRange(bool aEnabled)` | Enable/disable automatic value range detection |
| `setAxisRangeX(float aMin, float aMax)` | Set X-axis display range |
| `setAxisRangeY(float aMin, float aMax)` | Set Y-axis display range |
| `setAxisRangeZ(float aMin, float aMax)` | Set Z-axis display range |
| `setAxisLabels(const char* pX, const char* pY, const char* pZ)` | Set axis label strings |
| `setSmoothing(float aSpeed)` | Set transition smoothing speed |
| `setWireframe(bool aEnabled)` | Enable/disable wireframe overlay |
| `setPointSize(float aSize)` | Set scatter point size |
| `setStyle(const RLHeatMap3DStyle& rStyle)` | Apply a complete style configuration |

### Data Input

| Method | Description |
|--------|-------------|
| `setValues(const float* pValues, int aCount)` | Set all grid values (batch update) |
| `updatePartialValues(int aX, int aY, int aW, int aH, const float* pValues)` | Update a rectangular subregion |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animation/transitions (call each frame) |
| `draw(Vector3 aPosition, float aScale, const Camera3D& rCamera)` | Draw the 3D plot at position with scale |

### Getters

| Method | Description |
|--------|-------------|
| `getWidth() const` | Get grid width |
| `getHeight() const` | Get grid height |
| `getMinValue() const` | Get current minimum value |
| `getMaxValue() const` | Get current maximum value |
| `isAutoRange() const` | Check if auto-range is enabled |
| `getMode() const` | Get current render mode |

## Complete Example

```cpp
#include "raylib.h"
#include "RLHeatMap3D.h"
#include <vector>
#include <cmath>

int main() {
    InitWindow(1280, 720, "3D Scientific Plot Example");
    SetTargetFPS(60);

    // Setup 3D camera
    Camera3D lCamera = {0};
    lCamera.position = Vector3{2.0f, 1.5f, 2.0f};
    lCamera.target = Vector3{0.0f, 0.4f, 0.0f};
    lCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    lCamera.fovy = 45.0f;
    lCamera.projection = CAMERA_PERSPECTIVE;

    // Create 3D heat map with 40x40 grid
    RLHeatMap3D lHeatMap(40, 40);

    // Configure scientific plot style
    RLHeatMap3DStyle lStyle;
    lStyle.mMode = RLHeatMap3DMode::Surface;
    lStyle.mShowWireframe = true;
    lStyle.mSurfaceOpacity = 0.9f;
    lStyle.mShowAxisBox = true;
    lStyle.mShowFloorGrid = true;
    lStyle.mGridDivisions = 10;
    lHeatMap.setStyle(lStyle);

    // Set custom palette
    lHeatMap.setPalette(
        Color{30, 60, 180, 255},     // Blue (low)
        Color{0, 180, 200, 255},     // Cyan
        Color{100, 220, 100, 255},   // Green
        Color{255, 180, 50, 255}     // Orange (high)
    );

    // Prepare data buffer
    std::vector<float> lValues(40 * 40);
    float lTime = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Generate animated sine wave pattern
        for (int lY = 0; lY < 40; ++lY) {
            for (int lX = 0; lX < 40; ++lX) {
                float lNx = (float)lX / 40.0f;
                float lNy = (float)lY / 40.0f;
                float lWave1 = sinf(lNx * 4.0f * PI + lTime * 2.0f) * 0.3f;
                float lWave2 = sinf(lNy * 3.0f * PI + lTime * 1.5f) * 0.3f;
                lValues[(size_t)(lY * 40 + lX)] = 0.5f + lWave1 + lWave2;
            }
        }

        // Update heat map data and animation
        lHeatMap.setValues(lValues.data(), (int)lValues.size());
        lHeatMap.update(lDt);

        BeginDrawing();
        ClearBackground(Color{25, 28, 35, 255});

        BeginMode3D(lCamera);
        lHeatMap.draw(Vector3{0.0f, 0.0f, 0.0f}, 1.0f, lCamera);
        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Scatter Mode Example

```cpp
// Switch to scatter mode for point cloud visualization
lStyle.mMode = RLHeatMap3DMode::Scatter;
lStyle.mPointSize = 0.02f;
lStyle.mShowPointOutline = true;
lHeatMap.setStyle(lStyle);
```

## Camera Controls Example

For interactive demos with mouse-based camera controls:

```cpp
// Camera orbit state
float lCameraDistance = 3.0f;
float lCameraYaw = 0.8f;
float lCameraPitch = 0.5f;
Vector2 lLastMousePos = GetMousePosition();

// In update loop:
Vector2 lMousePos = GetMousePosition();
Vector2 lMouseDelta = {lMousePos.x - lLastMousePos.x, lMousePos.y - lLastMousePos.y};

// Mouse drag to rotate
if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    lCameraYaw -= lMouseDelta.x * 0.005f;
    lCameraPitch -= lMouseDelta.y * 0.005f;
    lCameraPitch = Clamp(lCameraPitch, 0.1f, 1.4f);
}

// Mouse wheel to zoom
float lWheel = GetMouseWheelMove();
lCameraDistance -= lWheel * 0.2f;
lCameraDistance = Clamp(lCameraDistance, 1.5f, 8.0f);

lLastMousePos = lMousePos;

// Update camera position (orbit around target)
lCamera.position.x = sinf(lCameraYaw) * cosf(lCameraPitch) * lCameraDistance;
lCamera.position.y = sinf(lCameraPitch) * lCameraDistance;
lCamera.position.z = cosf(lCameraYaw) * cosf(lCameraPitch) * lCameraDistance;
lCamera.target = Vector3{0.0f, 0.4f, 0.0f};
```

## Visual Features

### Axis Box
The axis box provides a 3D bounding frame with:
- 12 edges forming a rectangular prism
- Tick marks on each axis
- Configurable colors and visibility

### Back Walls
Semi-transparent walls on the back/side planes that:
- Automatically fade based on camera viewing angle
- Walls facing away from the camera become more transparent
- Creates a clean scientific visualization look

### Floor Grid
A grid on the bottom plane (Z=0) that:
- Provides spatial reference
- Configurable divisions and color
- Can be toggled on/off

## Live Streaming Data

For real-time data visualization (e.g., sensor feeds), update values frequently with smooth interpolation:

```cpp
// Streaming state with smoothing
std::vector<float> lCurrentValues(gridSize, 0.5f);
std::vector<float> lTargetValues(gridSize, 0.5f);

// In update loop - receive new data periodically
if (newDataAvailable) {
    // Copy incoming data to target buffer
    lTargetValues = incomingData;
}

// Smooth interpolation towards target
float lAlpha = 1.0f - expf(-8.0f * lDt);
for (size_t i = 0; i < lCurrentValues.size(); ++i) {
    lCurrentValues[i] += (lTargetValues[i] - lCurrentValues[i]) * lAlpha;
}

// Update heat map
lHeatMap.setValues(lCurrentValues.data(), (int)lCurrentValues.size());
lHeatMap.update(lDt);
```

## Partial Region Updates

For efficient updates to specific regions (e.g., moving hotspots, localized changes):

```cpp
// Define the region to update
int lRegionX = 10;      // Starting X position
int lRegionY = 15;      // Starting Y position
int lRegionW = 8;       // Region width
int lRegionH = 8;       // Region height

// Prepare region data (row-major order)
std::vector<float> lRegionValues(lRegionW * lRegionH);
for (int lY = 0; lY < lRegionH; ++lY) {
    for (int lX = 0; lX < lRegionW; ++lX) {
        // Generate hotspot value
        float lCx = lRegionW * 0.5f;
        float lCy = lRegionH * 0.5f;
        float lDist = sqrtf((lX - lCx) * (lX - lCx) + (lY - lCy) * (lY - lCy));
        lRegionValues[lY * lRegionW + lX] = expf(-lDist * 0.5f);
    }
}

// Apply partial update - only this region is modified
lHeatMap.updatePartialValues(lRegionX, lRegionY, lRegionW, lRegionH, lRegionValues.data());
```

This is much more efficient than updating the entire grid when only a small region changes.

## Fixed vs Auto Value Range

By default, the value range (Z-axis) is automatically calculated from the data. For stable visualizations where data may vary significantly, you can set a fixed range:

```cpp
// Auto-range mode (default) - range adjusts to data
lHeatMap.setAutoRange(true);

// Fixed range mode - range stays constant regardless of data
lHeatMap.setValueRange(0.0f, 1.0f);  // Z-axis always spans 0 to 1
```

Fixed range is useful when:
- Comparing multiple datasets with different value distributions
- Streaming data where you want consistent visual scale
- Preventing the visualization from "jumping" when data range changes

## Demo Modes

The `heatmap3d` demo showcases 6 different modes:

1. **Surface Static** - Static datasets (Gaussian hill, Saddle surface) with breathing effect
2. **Surface Animated** - Animated overlapping sine waves
3. **Surface Streaming** - Simulated live sensor data feed at 20 Hz
4. **Surface Partial** - Moving hotspot region updates every 1.5 seconds
5. **Scatter Static** - Static point cloud visualization
6. **Scatter Animated** - Animated ripple pattern as scatter points

### Demo Controls

- **SPACE**: Cycle through modes
- **W**: Toggle wireframe
- **G**: Toggle floor grid
- **B**: Toggle axis box
- **A**: Toggle auto-range (AUTO vs FIXED 0-1)
- **D**: Cycle datasets (static modes only)
- **R**: Reset camera
- **Mouse drag**: Rotate view
- **Mouse wheel**: Zoom

## Performance Notes

- Grid sizes up to 256×256 are recommended for smooth performance
- Both Surface and Scatter modes use GPU-uploaded meshes for efficient rendering
- Mesh vertices and colors are updated efficiently without full mesh rebuild
- Scatter mode builds a mesh of small cubes, one per grid point
- Use `setSmoothing()` to control transition animation speed

