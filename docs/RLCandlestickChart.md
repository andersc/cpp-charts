# RLCandlestickChart

![RLCandlestick Animation](gifs/RLCandlestick.gif)

A candlestick chart for financial data visualization (OHLCV) with streaming support.

## Features

- Real-time streaming of OHLCV data
- Configurable candle aggregation (multiple samples per candle)
- Volume display area
- Smooth sliding animations for new candles
- Day separators
- Auto-scaling or manual price range

## Constructor

```cpp
RLCandlestickChart(Rectangle bounds, int valuesPerCandle, int visibleCandles, const RLCandleStyle &style = {});
```

**Parameters:**
- `bounds` - The rectangle defining the chart's position and size
- `valuesPerCandle` - Number of samples to aggregate into each candle
- `visibleCandles` - Maximum number of candles visible at once
- `style` - Optional style configuration

## Data Structure

```cpp
struct CandleInput {
    float aOpen{0.0f};   // Opening price
    float aHigh{0.0f};   // Highest price
    float aLow{0.0f};    // Lowest price
    float aClose{0.0f};  // Closing price
    float aVolume{0.0f}; // Trading volume
    std::string aDate;   // Date string (e.g., "2024-01-15 09:35:00")
};
```

## Style Configuration

```cpp
struct RLCandleStyle {
    // Layout
    float mPadding = 8.0f;                   // Chart inner padding
    float mCandleSpacing = 4.0f;             // Space between candles
    float mBodyMinWidth = 6.0f;              // Minimal body width
    float mWickThickness = 2.0f;             // Wick line thickness
    float mVolumeAreaRatio = 0.25f;          // Part of total height reserved for volume

    // Colors
    Color mBackground{20, 22, 28, 255};
    Color mGridColor{40, 44, 52, 120};
    int mGridLines = 4;
    Color mUpBody{60, 190, 120, 255};
    Color mUpWick{180, 240, 200, 255};
    Color mDownBody{220, 90, 90, 255};
    Color mDownWick{255, 200, 200, 255};
    Color mSeparator{200, 200, 200, 90};     // Daily separator
    Color mVolumeUp{90, 180, 120, 180};
    Color mVolumeDown{200, 90, 90, 180};

    // Animation
    float mSlideSpeed = 8.0f;                // Larger = faster slide (units per second: bodyWidth)
    float mFadeSpeed = 6.0f;                 // Alpha lerp speed

    // Scaling
    bool mAutoScale = true;
    float lMinPrice = 0.0f;
    float lMaxPrice = 1.0f;
    bool mIncludeWicksInScale = true;
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the chart bounds |
| `setValuesPerCandle(int aValuesPerCandle)` | Set samples per candle |
| `setVisibleCandles(int aVisibleCandles)` | Set number of visible candles |
| `setStyle(const RLCandleStyle &aStyle)` | Apply a style configuration |
| `setExplicitScale(float aMinPrice, float aMaxPrice)` | Set explicit price scale |

### Data

| Method | Description |
|--------|-------------|
| `addSample(const CandleInput &aSample)` | Stream a single OHLCV sample |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame with delta time) |
| `draw() const` | Draw the chart |

## Complete Example

```cpp
#include "raylib.h"
#include "RLCandlestickChart.h"

int main() {
    InitWindow(1200, 600, "Candlestick Chart Example");
    SetTargetFPS(60);

    // Define chart area
    Rectangle lBounds = {50, 50, 1100, 500};

    // Create style
    RLCandleStyle lStyle;
    lStyle.mBackground = Color{20, 22, 28, 255};
    lStyle.mGridLines = 4;
    lStyle.mCandleSpacing = 3.0f;
    lStyle.mBodyMinWidth = 8.0f;

    // Create chart: 5 samples per candle, 30 visible candles
    RLCandlestickChart lChart(lBounds, 5, 30, lStyle);

    // Simulate initial price data
    float lPrice = 100.0f;
    for (int i = 0; i < 25; ++i) {
        RLCandlestickChart::CandleInput lCandle;
        lCandle.aOpen = lPrice;
        float lChange = (float)(rand() % 10 - 5);
        lCandle.aClose = lPrice + lChange;
        lCandle.aHigh = fmaxf(lCandle.aOpen, lCandle.aClose) + (float)(rand() % 3);
        lCandle.aLow = fminf(lCandle.aOpen, lCandle.aClose) - (float)(rand() % 3);
        lCandle.aVolume = 1000.0f + (float)(rand() % 4000);
        lCandle.aDate = "2024-01-" + std::to_string(i + 1);
        lChart.addSample(lCandle);
        lPrice = lCandle.aClose;
    }

    float lTimer = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTimer += lDt;

        // Add new sample every 0.5 seconds
        if (lTimer > 0.5f) {
            lTimer = 0.0f;
            RLCandlestickChart::CandleInput lCandle;
            lCandle.aOpen = lPrice;
            float lChange = (float)(rand() % 10 - 5);
            lCandle.aClose = lPrice + lChange;
            lCandle.aHigh = fmaxf(lCandle.aOpen, lCandle.aClose) + (float)(rand() % 3);
            lCandle.aLow = fminf(lCandle.aOpen, lCandle.aClose) - (float)(rand() % 3);
            lCandle.aVolume = 1000.0f + (float)(rand() % 4000);
            lChart.addSample(lCandle);
            lPrice = lCandle.aClose;
        }

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

## Streaming Data

The chart is designed for real-time streaming. Each call to `addSample()` either:
1. Aggregates the sample into the current working candle
2. Or finalizes the candle and starts a new one (when `valuesPerCandle` is reached)

When a candle is finalized, the chart smoothly slides left to make room for new data.

```cpp
// Stream samples one at a time
for (const auto& lSample : lIncomingData) {
    lChart.addSample(lSample);
}
```

