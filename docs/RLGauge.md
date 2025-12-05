# RLGauge

A circular gauge display for raylib with smooth animations.

## Features

- Circular arc display with customizable range
- Animated needle
- Configurable tick marks (minor and major)
- Value text display
- Smooth animation to target values

## Constructor

```cpp
RLGauge(Rectangle bounds, float minValue, float maxValue, const RLGaugeStyle &style = {});
```

**Parameters:**
- `bounds` - The rectangle defining the gauge's position and size
- `minValue` - Minimum value on the gauge scale
- `maxValue` - Maximum value on the gauge scale
- `style` - Optional style configuration

## Style Configuration

```cpp
struct RLGaugeStyle {
    // Visuals
    Color mBackgroundColor{30, 30, 36, 255};
    Color mBaseArcColor{60, 60, 70, 255};
    Color mValueArcColor{0, 180, 255, 255};
    Color mTickColor{150, 150, 160, 255};
    Color mMajorTickColor{220, 220, 230, 255};
    Color mLabelColor{220, 220, 230, 255};
    Color mNeedleColor{255, 74, 74, 255};
    Color mCenterColor{230, 230, 240, 255};

    float mThickness = 18.0f;        // Ring thickness (pixels)
    float mStartAngle = 135.0f;      // Start angle in degrees
    float mEndAngle = 405.0f;        // End angle in degrees (end > start)
    int mTickCount = 60;             // Minor ticks
    int mMajorEvery = 5;             // Every Nth tick is major
    float mTickLen = 8.0f;           // Minor tick length
    float mMajorTickLen = 14.0f;     // Major tick length
    float mTickThickness = 2.0f;     // Minor tick thickness
    float mMajorTickThickness = 3.0f; // Major tick thickness
    float mNeedleWidth = 4.0f;       // Needle thickness
    float mNeedleRadiusScale = 0.86f; // As fraction of radius
    bool mShowValueText = true;
    bool mShowTicks = true;
    bool mShowNeedle = true;
    bool mSmoothAnimate = true;      // Smooth animation towards target
    Font mLabelFont{};               // Optional custom font; if .baseSize==0 use default
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle bounds)` | Set the gauge bounds |
| `setRange(float minValue, float maxValue)` | Set the value range |
| `setStyle(const RLGaugeStyle &style)` | Apply a style configuration |

### Value

| Method | Description |
|--------|-------------|
| `setValue(float value)` | Set value immediately (no animation) |
| `setTargetValue(float value)` | Set target value (animates to new value) |
| `getValue() const` | Get current displayed value |
| `getTarget() const` | Get target value |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float dt)` | Update animations (call each frame with delta time) |
| `draw() const` | Draw the gauge |

## Complete Example

```cpp
#include "raylib.h"
#include "RLGauge.h"

int main() {
    InitWindow(600, 600, "Gauge Example");
    SetTargetFPS(60);

    // Define gauge area (square works best)
    Rectangle lBounds = {100, 100, 400, 400};

    // Create style
    RLGaugeStyle lStyle;
    lStyle.mBackgroundColor = Color{30, 30, 36, 255};
    lStyle.mBaseArcColor = Color{60, 60, 70, 255};
    lStyle.mValueArcColor = Color{0, 180, 255, 255};
    lStyle.mNeedleColor = Color{255, 74, 74, 255};
    lStyle.mThickness = 20.0f;
    lStyle.mTickCount = 50;
    lStyle.mShowValueText = true;

    // Create gauge with range 0-100
    RLGauge lGauge(lBounds, 0.0f, 100.0f, lStyle);

    // Set initial value
    lGauge.setValue(25.0f);

    float lTimer = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTimer += lDt;

        // Animate to random value every 2 seconds
        if (lTimer > 2.0f) {
            lTimer = 0.0f;
            float lNewValue = (float)(rand() % 100);
            lGauge.setTargetValue(lNewValue);
        }

        // Update animation
        lGauge.update(lDt);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw gauge
        lGauge.draw();
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Customizing Appearance

### Semi-circular Gauge

```cpp
RLGaugeStyle lStyle;
lStyle.mStartAngle = 180.0f;  // Start at left
lStyle.mEndAngle = 360.0f;    // End at right
```

### Full Circle Gauge

```cpp
RLGaugeStyle lStyle;
lStyle.mStartAngle = 0.0f;
lStyle.mEndAngle = 360.0f;
```

### Minimal Gauge (No Ticks)

```cpp
RLGaugeStyle lStyle;
lStyle.mShowTicks = false;
lStyle.mShowNeedle = false;
lStyle.mThickness = 30.0f;
```

## Animation

The gauge smoothly animates between values when using `setTargetValue()`:

```cpp
// Set initial value
lGauge.setValue(0.0f);

// Later, animate to new value
lGauge.setTargetValue(75.0f);

// The needle and value arc will smoothly animate to 75
```

