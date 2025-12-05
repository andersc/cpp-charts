# RLOrderBookVis

A full depth-of-market (DOM) order book visualization with both 2D heatmap and 3D landscape views.

## Features

- Real-time streaming order book snapshots
- 2D heatmap view (time × price, color = liquidity)
- 3D heightmap landscape view
- Configurable price range filtering (full depth, spread-based, or explicit range)
- Separate color gradients for bids and asks
- Mid-price and spread visualization
- Smooth scrolling as new data arrives
- Efficient GPU-accelerated rendering

## Constructor

```cpp
RLOrderBookVis(Rectangle aBounds, size_t aHistoryLength, size_t aPriceLevels);
```

**Parameters:**
- `aBounds` - The rectangle defining the visualization's position and size
- `aHistoryLength` - Number of snapshots to keep in history (width of heatmap)
- `aPriceLevels` - Number of price levels to display (height of heatmap)

## Data Structure

```cpp
struct RLOrderBookSnapshot {
    std::vector<std::pair<float, float>> mBids; // (price, size) pairs, descending
    std::vector<std::pair<float, float>> mAsks; // (price, size) pairs, ascending
    float mTimestamp{0.0f};                      // Optional timestamp
};
```

## Price Filter Modes

```cpp
enum class RLOrderBookPriceMode {
    FullDepth,      // Show all price levels from the feed
    SpreadTicks,    // Show ±N ticks around mid-price
    ExplicitRange   // Show explicit price range [min, max]
};
```

## Style Configuration

```cpp
struct RLOrderBookVisStyle {
    // Background and border
    Color mBackground{20, 22, 28, 255};
    bool mShowBorder{true};
    Color mBorderColor{40, 44, 52, 255};
    float mBorderThickness{1.0f};
    float mPadding{8.0f};

    // Grid
    bool mShowGrid{true};
    Color mGridColor{40, 44, 52, 120};
    int mGridLinesX{8};
    int mGridLinesY{6};

    // Mid-price / spread visualization
    bool mShowMidLine{true};
    Color mMidLineColor{255, 255, 255, 180};
    float mMidLineThickness{1.5f};
    bool mShowSpreadArea{true};
    Color mSpreadAreaColor{255, 255, 255, 30};

    // Intensity scaling
    float mIntensityScale{1.0f};      // Multiplier for size-to-intensity
    bool mLogScale{false};            // Use logarithmic scaling
    float mMaxIntensity{0.0f};        // Clamp intensity (auto if 0)

    // Animation
    float mScrollSpeed{8.0f};
    float mScaleSpeed{4.0f};

    // 3D specific
    float mHeightScale{1.0f};
    bool mShow3DGrid{true};
    Color m3DGridColor{60, 60, 70, 100};
    float m3DCellSize{1.0f};
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the visualization bounds |
| `setHistoryLength(size_t aLength)` | Set number of snapshots in history |
| `setPriceLevels(size_t aLevels)` | Set number of price levels displayed |
| `setStyle(const RLOrderBookVisStyle &rStyle)` | Apply a style configuration |

### Price Filtering

| Method | Description |
|--------|-------------|
| `setPriceMode(RLOrderBookPriceMode aMode)` | Set price filtering mode |
| `setSpreadTicks(int aTicks)` | Set ±N ticks around mid-price (SpreadTicks mode) |
| `setPriceRange(float aMin, float aMax)` | Set explicit price range (ExplicitRange mode) |

### Color Configuration

| Method | Description |
|--------|-------------|
| `setBidColorStops(const std::vector<Color> &rStops)` | Set bid gradient (2-4 colors) |
| `setAskColorStops(const std::vector<Color> &rStops)` | Set ask gradient (2-4 colors) |

### Data Input

| Method | Description |
|--------|-------------|
| `pushSnapshot(const RLOrderBookSnapshot &rSnapshot)` | Push a new order book snapshot |
| `clear()` | Clear all history data |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations and textures (call each frame) |
| `draw2D() const` | Draw the 2D heatmap view |
| `draw3D(const Camera3D &rCamera) const` | Draw the 3D landscape view |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getHistoryLength() const` | Get history length |
| `getPriceLevels() const` | Get number of price levels |
| `getSnapshotCount() const` | Get current snapshot count |
| `getCurrentMidPrice() const` | Get current mid-price |
| `getCurrentSpread() const` | Get current bid-ask spread |
| `getSpreadTicks() const` | Get spread ticks setting |

## Complete Example

```cpp
#include "raylib.h"
#include "RLOrderBookVis.h"
#include <vector>
#include <cmath>

int main() {
    InitWindow(1200, 800, "Order Book Visualization Example");
    SetTargetFPS(60);

    // Create order book visualization
    Rectangle lBounds = {50, 50, 800, 600};
    RLOrderBookVis lOrderBook(lBounds, 100, 50);

    // Configure style
    RLOrderBookVisStyle lStyle;
    lStyle.mBackground = Color{20, 22, 28, 255};
    lStyle.mShowMidLine = true;
    lStyle.mIntensityScale = 1.5f;
    lOrderBook.setStyle(lStyle);

    // Use spread-based filtering: show ±30 ticks around mid-price
    lOrderBook.setPriceMode(RLOrderBookPriceMode::SpreadTicks);
    lOrderBook.setSpreadTicks(30);

    // Custom colors
    std::vector<Color> lBidColors = {
        Color{0, 20, 40, 255},
        Color{0, 100, 80, 255},
        Color{50, 200, 120, 255}
    };
    std::vector<Color> lAskColors = {
        Color{40, 10, 10, 255},
        Color{150, 50, 30, 255},
        Color{255, 100, 60, 255}
    };
    lOrderBook.setBidColorStops(lBidColors);
    lOrderBook.setAskColorStops(lAskColors);

    // Simulate initial data
    float lMidPrice = 100.0f;
    for (int i = 0; i < 50; ++i) {
        RLOrderBookSnapshot lSnap;
        float lBestBid = lMidPrice - 0.05f;
        float lBestAsk = lMidPrice + 0.05f;

        // Generate depth levels
        for (int j = 0; j < 30; ++j) {
            float lDecay = expf(-(float)j * 0.1f);
            float lBidSize = (100.0f + rand() % 2000) * lDecay;
            float lAskSize = (100.0f + rand() % 2000) * lDecay;

            lSnap.mBids.push_back({lBestBid - j * 0.01f, lBidSize});
            lSnap.mAsks.push_back({lBestAsk + j * 0.01f, lAskSize});
        }

        lOrderBook.pushSnapshot(lSnap);
        lMidPrice += ((rand() % 100) - 50) * 0.0001f;
    }

    // 3D camera (for 3D view)
    Camera3D lCamera = {0};
    lCamera.position = Vector3{15.0f, 10.0f, 15.0f};
    lCamera.target = Vector3{0.0f, 0.0f, 0.0f};
    lCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    lCamera.fovy = 45.0f;
    lCamera.projection = CAMERA_PERSPECTIVE;

    bool lShow3D = false;
    float lTimer = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();

        // Toggle view with TAB
        if (IsKeyPressed(KEY_TAB)) {
            lShow3D = !lShow3D;
        }

        // Push new snapshot periodically
        lTimer += lDt;
        if (lTimer > 0.1f) {
            lTimer = 0.0f;

            RLOrderBookSnapshot lSnap;
            float lBestBid = lMidPrice - 0.05f;
            float lBestAsk = lMidPrice + 0.05f;

            for (int j = 0; j < 30; ++j) {
                float lDecay = expf(-(float)j * 0.1f);
                lSnap.mBids.push_back({lBestBid - j * 0.01f, (100.0f + rand() % 2000) * lDecay});
                lSnap.mAsks.push_back({lBestAsk + j * 0.01f, (100.0f + rand() % 2000) * lDecay});
            }

            lOrderBook.pushSnapshot(lSnap);
            lMidPrice += ((rand() % 100) - 50) * 0.0002f;
        }

        // Update
        lOrderBook.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground(Color{15, 17, 20, 255});

        if (lShow3D) {
            lOrderBook.draw3D(lCamera);
        } else {
            lOrderBook.draw2D();
        }

        DrawText(lShow3D ? "3D View (TAB to switch)" : "2D View (TAB to switch)",
                 10, 10, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Visualization Concepts

### 2D Heatmap View
- **X-axis**: Time (oldest left, newest right)
- **Y-axis**: Price (highest top, lowest bottom)
- **Color intensity**: Liquidity/size at that price level
- **Green hues**: Bid (buy) orders
- **Red hues**: Ask (sell) orders
- **White line**: Mid-price / spread area

### 3D Landscape View
- **X-axis**: Time
- **Z-axis**: Price
- **Y-axis (height)**: Liquidity/size
- Separate meshes for bids and asks
- Color corresponds to height/intensity

### Interpreting the Display
- **Bright walls**: Large resting orders (support/resistance)
- **Dark areas**: Low liquidity
- **Moving patterns**: Price drift over time
- **Sudden changes**: Order fills or cancellations
- **Imbalance**: More bids than asks (or vice versa) suggests directional pressure

