# RLLinearGauge

A horizontal or vertical linear gauge display for raylib with smooth animations.

## Features

- Horizontal and vertical orientations
- Multiple pointer/indicator styles (fill bar, triangle, line marker)
- Colored range bands (e.g., green/yellow/red zones)
- Major and minor tick marks with labels
- Target/goal marker line
- Smooth animation to target values
- Title and unit display
- Customizable styling

## Constructor

```cpp
RLLinearGauge(Rectangle aBounds, float aMinValue, float aMaxValue,
              RLLinearGaugeOrientation aOrientation = RLLinearGaugeOrientation::HORIZONTAL,
              const RLLinearGaugeStyle &aStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the gauge's position and size
- `aMinValue` - Minimum value on the gauge scale
- `aMaxValue` - Maximum value on the gauge scale
- `aOrientation` - HORIZONTAL or VERTICAL orientation
- `aStyle` - Optional style configuration

## Orientation

```cpp
enum class RLLinearGaugeOrientation {
    HORIZONTAL,  // Left-to-right gauge
    VERTICAL     // Bottom-to-top gauge
};
```

## Pointer Styles

```cpp
enum class RLLinearGaugePointerStyle {
    FILL_BAR,      // Filled bar from min to current value (default)
    TRIANGLE,      // Triangle pointer at current value
    LINE_MARKER    // Line marker at current value
};
```

## Range Bands

```cpp
struct RLLinearGaugeRangeBand {
    float mMin{0.0f};      // Start of the range
    float mMax{100.0f};    // End of the range
    Color mColor{GREEN};   // Color for this range
};
```

## Style Configuration

```cpp
struct RLLinearGaugeStyle {
    // Track appearance
    Color mTrackColor{60, 60, 70, 255};
    Color mTrackBorderColor{80, 80, 90, 255};
    float mTrackThickness{24.0f};
    float mTrackBorderThickness{1.0f};
    float mCornerRadius{4.0f};

    // Fill/indicator appearance
    Color mFillColor{0, 180, 255, 255};
    Color mPointerColor{255, 74, 74, 255};
    float mPointerSize{12.0f};

    // Target marker appearance
    Color mTargetMarkerColor{255, 220, 80, 255};
    float mTargetMarkerThickness{3.0f};
    float mTargetMarkerLength{8.0f};

    // Tick marks
    int mMajorTickCount{5};
    int mMinorTicksPerMajor{4};
    Color mMajorTickColor{220, 220, 230, 255};
    Color mMinorTickColor{150, 150, 160, 255};
    float mMajorTickLength{12.0f};
    float mMinorTickLength{6.0f};
    float mMajorTickThickness{2.0f};
    float mMinorTickThickness{1.0f};

    // Labels
    Color mLabelColor{220, 220, 230, 255};
    Color mTitleColor{180, 190, 210, 255};
    Color mValueColor{255, 255, 255, 255};
    float mLabelFontSize{12.0f};
    float mTitleFontSize{16.0f};
    float mValueFontSize{18.0f};
    Font mLabelFont{};

    // Layout padding
    float mPadding{10.0f};
    float mTickLabelGap{4.0f};

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{10.0f};

    // Display options
    bool mShowTicks{true};
    bool mShowTickLabels{true};
    bool mShowTitle{true};
    bool mShowValueText{true};
    bool mShowRangeBands{true};
    bool mShowTargetMarker{false};
    int mValueDecimals{1};

    // Background
    Color mBackgroundColor{30, 30, 36, 255};
    bool mShowBackground{true};
};
```

## Methods

### Value Control

| Method | Description |
|--------|-------------|
| `setValue(float aValue)` | Set value immediately (no animation) |
| `setTargetValue(float aValue)` | Set target value (animates to new value) |
| `getValue() const` | Get current displayed value |
| `getTargetValue() const` | Get target value |

### Configuration

| Method | Description |
|--------|-------------|
| `setRange(float aMinValue, float aMaxValue)` | Set the value range |
| `setBounds(Rectangle aBounds)` | Set the gauge bounds |
| `setOrientation(RLLinearGaugeOrientation aOrientation)` | Set horizontal/vertical orientation |
| `setStyle(const RLLinearGaugeStyle &aStyle)` | Apply a style configuration |
| `setPointerStyle(RLLinearGaugePointerStyle aStyle)` | Set pointer/indicator style |
| `setAnimationEnabled(bool aEnabled)` | Enable/disable smooth animation |

### Tick Marks

| Method | Description |
|--------|-------------|
| `setTicks(int aMajorCount, int aMinorPerMajor)` | Configure tick mark counts |

### Labels

| Method | Description |
|--------|-------------|
| `setLabel(const std::string &aTitle)` | Set the gauge title |
| `setUnit(const std::string &aUnit)` | Set the unit suffix (e.g., "°C", "%") |

### Range Bands

| Method | Description |
|--------|-------------|
| `setRanges(const std::vector<RLLinearGaugeRangeBand> &aRanges)` | Set colored range bands |
| `clearRanges()` | Remove all range bands |

### Target Marker

| Method | Description |
|--------|-------------|
| `setTargetMarker(float aValue)` | Show target/goal marker at value |
| `hideTargetMarker()` | Hide the target marker |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame with delta time) |
| `draw() const` | Draw the gauge |

## Complete Example

```cpp
#include "raylib.h"
#include "RLLinearGauge.h"

int main() {
    InitWindow(800, 400, "Linear Gauge Example");
    SetTargetFPS(60);

    // Create horizontal gauge
    Rectangle lHorizBounds = {50, 50, 300, 80};
    RLLinearGaugeStyle lStyle;
    lStyle.mBackgroundColor = Color{30, 30, 36, 255};
    lStyle.mTrackColor = Color{50, 55, 65, 255};
    lStyle.mFillColor = Color{80, 200, 120, 255};
    lStyle.mMajorTickCount = 10;
    lStyle.mMinorTicksPerMajor = 1;
    lStyle.mSmoothAnimate = true;

    RLLinearGauge lHorizGauge(lHorizBounds, 0.0f, 100.0f,
                              RLLinearGaugeOrientation::HORIZONTAL, lStyle);
    lHorizGauge.setLabel("Temperature");
    lHorizGauge.setUnit("°C");

    // Add colored range bands
    std::vector<RLLinearGaugeRangeBand> lRanges = {
        {0.0f, 60.0f, Color{80, 200, 120, 255}},   // Green: Safe
        {60.0f, 80.0f, Color{255, 200, 80, 255}},  // Yellow: Warning
        {80.0f, 100.0f, Color{255, 80, 80, 255}}   // Red: Danger
    };
    lHorizGauge.setRanges(lRanges);
    lHorizGauge.setTargetMarker(75.0f);
    lHorizGauge.setValue(45.0f);

    // Create vertical gauge
    Rectangle lVertBounds = {450, 50, 100, 300};
    RLLinearGauge lVertGauge(lVertBounds, 0.0f, 100.0f,
                             RLLinearGaugeOrientation::VERTICAL, lStyle);
    lVertGauge.setPointerStyle(RLLinearGaugePointerStyle::TRIANGLE);
    lVertGauge.setLabel("Level");
    lVertGauge.setUnit("%");
    lVertGauge.setValue(70.0f);

    float lTimer = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTimer += lDt;

        // Animate to random value every 2 seconds
        if (lTimer > 2.0f) {
            lTimer = 0.0f;
            lHorizGauge.setTargetValue((float)(rand() % 100));
            lVertGauge.setTargetValue((float)(rand() % 100));
        }

        // Update animations
        lHorizGauge.update(lDt);
        lVertGauge.update(lDt);

        BeginDrawing();
        ClearBackground(Color{20, 22, 28, 255});

        lHorizGauge.draw();
        lVertGauge.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Customizing Appearance

### Horizontal Progress Bar

```cpp
RLLinearGaugeStyle lStyle;
lStyle.mTrackThickness = 20.0f;
lStyle.mCornerRadius = 10.0f;  // Rounded ends
lStyle.mShowTicks = false;
lStyle.mShowTickLabels = false;
lStyle.mFillColor = Color{100, 180, 255, 255};

RLLinearGauge lGauge(lBounds, 0.0f, 100.0f,
                     RLLinearGaugeOrientation::HORIZONTAL, lStyle);
```

### Vertical Thermometer Style

```cpp
RLLinearGaugeStyle lStyle;
lStyle.mTrackThickness = 16.0f;
lStyle.mMajorTickCount = 10;
lStyle.mMinorTicksPerMajor = 4;
lStyle.mShowValueText = true;
lStyle.mValueDecimals = 1;

RLLinearGauge lGauge(lBounds, -20.0f, 50.0f,
                     RLLinearGaugeOrientation::VERTICAL, lStyle);
lGauge.setLabel("Temperature");
lGauge.setUnit("°C");
```

### Dashboard Gauge with Zones

```cpp
RLLinearGaugeStyle lStyle;
lStyle.mTrackThickness = 24.0f;
lStyle.mMajorTickCount = 5;
lStyle.mMinorTicksPerMajor = 3;

RLLinearGauge lGauge(lBounds, 0.0f, 100.0f,
                     RLLinearGaugeOrientation::HORIZONTAL, lStyle);
lGauge.setPointerStyle(RLLinearGaugePointerStyle::TRIANGLE);

std::vector<RLLinearGaugeRangeBand> lRanges = {
    {0.0f, 50.0f, Color{80, 180, 255, 255}},   // Blue
    {50.0f, 80.0f, Color{255, 180, 80, 255}},  // Orange
    {80.0f, 100.0f, Color{255, 80, 100, 255}}  // Red
};
lGauge.setRanges(lRanges);
lGauge.setTargetMarker(75.0f);  // Goal line
```

## Performance Notes

- Tick positions are precomputed when geometry changes (bounds, range, or tick count)
- Minimal per-frame allocations
- Efficient raylib primitive drawing
- Suitable for multiple gauges on screen simultaneously

## VU Meter Mode

The linear gauge supports a VU Meter mode for audio-style volume visualization with multi-channel support.

### VU Meter Mode Enum

```cpp
enum class RLLinearGaugeMode {
    STANDARD,   // Normal single-value gauge (default)
    VU_METER    // Multi-channel VU meter with peak hold
};
```

### VU Meter Channel

```cpp
struct RLVuMeterChannel {
    float mValue{0.0f};      // Current channel value
    std::string mLabel{};    // Channel label (e.g., "L", "R", "C", "LFE")
};
```

### VU Meter Style

```cpp
struct RLVuMeterStyle {
    // Gradient colors (green -> yellow -> red)
    Color mLowColor{80, 200, 120, 255};      // Green zone
    Color mMidColor{255, 200, 80, 255};      // Yellow zone
    Color mHighColor{255, 80, 80, 255};      // Red zone

    // Thresholds for color zones (normalized 0.0 - 1.0)
    float mLowThreshold{0.6f};               // Below this: green
    float mMidThreshold{0.85f};              // Below this: yellow, above: red

    // Peak indicator
    Color mPeakMarkerColor{255, 255, 255, 255};
    float mPeakMarkerThickness{2.0f};
    float mPeakHoldTime{1.5f};               // Seconds to hold peak
    float mPeakDecaySpeed{0.5f};             // Units per second after hold

    // Clip indicator
    Color mClipIndicatorColor{255, 0, 0, 255};
    float mClipFlashDuration{0.3f};          // How long clip indicator flashes
    float mClipIndicatorSize{8.0f};          // Size of clip indicator

    // Channel layout
    float mChannelSpacing{4.0f};             // Gap between channel bars
    bool mShowChannelLabels{true};
    float mChannelLabelFontSize{10.0f};

    // dB scale option
    bool mUseDbScale{false};
    float mDbMin{-60.0f};                    // Minimum dB value (silence)
    float mDbMax{0.0f};                      // Maximum dB value (full scale)
};
```

### VU Meter Methods

| Method | Description |
|--------|-------------|
| `setMode(RLLinearGaugeMode aMode)` | Switch between STANDARD and VU_METER modes |
| `getMode() const` | Get current mode |
| `setVuMeterStyle(const RLVuMeterStyle &aStyle)` | Configure VU meter appearance |
| `setChannels(const std::vector<RLVuMeterChannel> &aChannels)` | Set up channels with labels |
| `setChannelValue(int aIndex, float aValue)` | Set value for a single channel |
| `setChannelValues(const std::vector<float> &aValues)` | Set values for all channels |
| `getChannelCount() const` | Get number of channels |
| `getPeakValue(int aIndex) const` | Get peak value for a channel |
| `isClipping(int aIndex) const` | Check if a channel is clipping |
| `resetPeaks()` | Reset all peak indicators |
| `resetClip()` | Clear all clip indicators |

### VU Meter Example: Stereo Meter

```cpp
#include "RLLinearGauge.h"

// Create vertical stereo VU meter
RLLinearGaugeStyle lStyle;
lStyle.mBackgroundColor = Color{28, 32, 40, 255};
lStyle.mTrackColor = Color{40, 44, 52, 255};
lStyle.mTrackThickness = 80.0f;
lStyle.mShowTicks = false;
lStyle.mShowValueText = false;

// Configure VU meter colors and behavior
lStyle.mVuStyle.mLowColor = Color{80, 200, 120, 255};   // Green
lStyle.mVuStyle.mMidColor = Color{255, 200, 80, 255};   // Yellow
lStyle.mVuStyle.mHighColor = Color{255, 80, 80, 255};   // Red
lStyle.mVuStyle.mLowThreshold = 0.6f;
lStyle.mVuStyle.mMidThreshold = 0.85f;
lStyle.mVuStyle.mPeakHoldTime = 1.5f;
lStyle.mVuStyle.mPeakDecaySpeed = 0.4f;
lStyle.mVuStyle.mChannelSpacing = 6.0f;
lStyle.mVuStyle.mShowChannelLabels = true;

Rectangle lBounds = {50, 50, 100, 400};
RLLinearGauge lVuMeter(lBounds, 0.0f, 1.0f,
                       RLLinearGaugeOrientation::VERTICAL, lStyle);
lVuMeter.setMode(RLLinearGaugeMode::VU_METER);
lVuMeter.setLabel("Stereo");

// Set up stereo channels
std::vector<RLVuMeterChannel> lChannels = {
    {0.0f, "L"},
    {0.0f, "R"}
};
lVuMeter.setChannels(lChannels);

// In update loop:
lVuMeter.setChannelValue(0, lLeftLevel);   // Left channel
lVuMeter.setChannelValue(1, lRightLevel);  // Right channel
lVuMeter.update(lDeltaTime);
lVuMeter.draw();
```

### VU Meter Example: 5.1 Surround

```cpp
// 6-channel surround sound meter
std::vector<RLVuMeterChannel> lChannels = {
    {0.0f, "L"},    // Front Left
    {0.0f, "R"},    // Front Right
    {0.0f, "C"},    // Center
    {0.0f, "LFE"},  // Subwoofer
    {0.0f, "Ls"},   // Rear Left
    {0.0f, "Rs"}    // Rear Right
};
lVuMeter.setChannels(lChannels);
```

### VU Meter Example: dB Scale

```cpp
// Enable logarithmic dB scale for authentic audio metering
lStyle.mVuStyle.mUseDbScale = true;
lStyle.mVuStyle.mDbMin = -60.0f;  // -60 dB = silence
lStyle.mVuStyle.mDbMax = 0.0f;    // 0 dB = full scale
lStyle.mVuStyle.mLowThreshold = 0.7f;   // Adjust for dB curve
lStyle.mVuStyle.mMidThreshold = 0.9f;
```

### VU Meter Example: Custom Multi-Source Data

```cpp
// Use for non-audio data (sensors, network, etc.)
RLLinearGaugeStyle lStyle;
lStyle.mVuStyle.mLowColor = Color{100, 180, 255, 255};   // Blue
lStyle.mVuStyle.mMidColor = Color{180, 120, 255, 255};   // Purple
lStyle.mVuStyle.mHighColor = Color{255, 100, 150, 255};  // Pink

RLLinearGauge lMeter(lBounds, 0.0f, 100.0f,
                     RLLinearGaugeOrientation::HORIZONTAL, lStyle);
lMeter.setMode(RLLinearGaugeMode::VU_METER);

std::vector<RLVuMeterChannel> lChannels = {
    {0.0f, "Sensor 1"},
    {0.0f, "Sensor 2"},
    {0.0f, "Sensor 3"},
    {0.0f, "Sensor 4"}
};
lMeter.setChannels(lChannels);
```
