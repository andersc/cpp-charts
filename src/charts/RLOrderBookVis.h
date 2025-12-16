// RLOrderBookVis.h
#pragma once
#include "raylib.h"
#include <vector>
#include <cstddef>
#include <utility>

// Order book snapshot: lists of (price, size) pairs for bids and asks
struct RLOrderBookSnapshot {
    std::vector<std::pair<float, float>> mBids; // Descending by price (best bid first)
    std::vector<std::pair<float, float>> mAsks; // Ascending by price (best ask first)
    float mTimestamp{0.0f};                      // Optional timestamp for labeling
};

// Price filtering mode for limiting displayed depth
enum class RLOrderBookPriceMode {
    FullDepth,      // Show all price levels
    SpreadTicks,    // Show ±N ticks around mid-price
    ExplicitRange   // Show explicit price range [minPrice, maxPrice]
};

// Visual style configuration
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
    float mIntensityScale{1.0f};      // Multiplier for size-to-intensity mapping
    bool mLogScale{false};            // Use logarithmic scaling for sizes
    float mMaxIntensity{0.0f};        // If > 0, clamp intensity to this value (auto if 0)

    // Animation
    float mScrollSpeed{8.0f};         // Speed of smooth scrolling
    float mScaleSpeed{4.0f};          // Speed of intensity scale transitions

    // 3D specific
    float mHeightScale{1.0f};         // Z-axis scale for 3D heightmap
    bool mShow3DGrid{true};
    Color m3DGridColor{60, 60, 70, 100};
    float m3DCellSize{1.0f};          // Base size of cells in 3D space
};

// Main order book visualization class
class RLOrderBookVis {
public:
    // Constructor: bounds for 2D, history length (number of snapshots), price levels to display
    RLOrderBookVis(Rectangle aBounds, size_t aHistoryLength, size_t aPriceLevels);
    ~RLOrderBookVis();

    // Prevent copy (owns GPU resources)
    RLOrderBookVis(const RLOrderBookVis&) = delete;
    RLOrderBookVis& operator=(const RLOrderBookVis&) = delete;

    // Configuration
    void setBounds(Rectangle aBounds);
    void setHistoryLength(size_t aLength);
    void setPriceLevels(size_t aLevels);
    void setStyle(const RLOrderBookVisStyle& rStyle);

    // Price filtering
    void setPriceMode(RLOrderBookPriceMode aMode);
    void setSpreadTicks(int aTicks);                           // For SpreadTicks mode
    void setPriceRange(float aMinPrice, float aMaxPrice);      // For ExplicitRange mode

    // Color configuration (gradient stops, 2-4 colors each)
    void setBidColorStops(const std::vector<Color>& rStops);
    void setAskColorStops(const std::vector<Color>& rStops);

    // Data input
    void pushSnapshot(const RLOrderBookSnapshot& rSnapshot);
    void clear();

    // Update and rendering
    void update(float aDt);
    void draw2D() const;
    void draw3D(const Camera3D& rCamera) const;

    // Getters
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] size_t getHistoryLength() const { return mHistoryLength; }
    [[nodiscard]] size_t getPriceLevels() const { return mPriceLevels; }
    [[nodiscard]] size_t getSnapshotCount() const { return mSnapshotCount; }
    [[nodiscard]] float getCurrentMidPrice() const { return mCurrentMidPrice; }
    [[nodiscard]] float getCurrentSpread() const { return mCurrentSpread; }
    [[nodiscard]] int getSpreadTicks() const { return mSpreadTicks; }

private:
    // Bounds and dimensions
    Rectangle mBounds{};
    size_t mHistoryLength{100};
    size_t mPriceLevels{50};
    RLOrderBookVisStyle mStyle{};

    // Price filtering
    RLOrderBookPriceMode mPriceMode{RLOrderBookPriceMode::SpreadTicks};
    int mSpreadTicks{20};
    float mExplicitMinPrice{0.0f};
    float mExplicitMaxPrice{100.0f};

    // Dynamic price range (computed from data)
    float mCurrentMinPrice{0.0f};
    float mCurrentMaxPrice{100.0f};
    float mTargetMinPrice{0.0f};
    float mTargetMaxPrice{100.0f};

    // Color LUTs (256 entries each)
    std::vector<Color> mBidStops;
    std::vector<Color> mAskStops;
    Color mBidLut[256]{};
    Color mAskLut[256]{};
    bool mLutDirty{true};

    // Ring buffer for snapshot history
    // Each column (snapshot) has mPriceLevels rows
    // Separate grids for bids and asks
    std::vector<float> mBidGrid;      // [historyLength * priceLevels]
    std::vector<float> mAskGrid;      // [historyLength * priceLevels]
    size_t mHead{0};                  // Next write position in ring buffer
    size_t mSnapshotCount{0};         // Current number of snapshots

    // Current market state
    float mCurrentMidPrice{50.0f};
    float mCurrentSpread{0.1f};
    float mCurrentBestBid{49.95f};
    float mCurrentBestAsk{50.05f};

    // Auto-scaling for intensity
    float mMaxBidSize{1.0f};
    float mMaxAskSize{1.0f};
    float mCurrentMaxBid{1.0f};
    float mCurrentMaxAsk{1.0f};

    // 2D texture resources
    std::vector<unsigned char> mPixels;  // RGBA pixels
    Texture2D mTexture{};
    bool mTextureValid{false};
    bool mTextureDirty{true};

    // 3D mesh resources
    Mesh mBidMesh{};
    Mesh mAskMesh{};
    bool mMeshValid{false};
    bool mMeshDirty{true};

    // Internal helpers
    void ensureBuffers();
    void rebuildLUT();
    void rebuildTexture();
    void updateTexturePixels();
    void rebuildMesh();
    void updateMeshData();
    void cleanupTexture();
    void cleanupMesh();

    // Price mapping helpers
    [[nodiscard]] float priceToNormalized(float aPrice) const;
    [[nodiscard]] float normalizedToPrice(float aNorm) const;
    [[nodiscard]] int priceToGridRow(float aPrice) const;

    // Grid access helpers
    [[nodiscard]] size_t gridIndex(size_t aTimeIdx, size_t aPriceIdx) const;
    [[nodiscard]] size_t ringTimeIndex(size_t aOffset) const;

    // Drawing helpers
    void drawBackground() const;
    void drawGrid2D() const;
    void drawMidLine2D() const;
    void drawHeatmap2D() const;
    [[nodiscard]] Rectangle getPlotArea() const;
};

