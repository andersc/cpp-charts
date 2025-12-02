// RLHeatMap.h
#pragma once
#include "raylib.h"
#include <vector>

enum class RLHeatMapUpdateMode {
    Replace,
    Accumulate,
    Decay
};

struct RLHeatMapStyle {
    bool mShowBackground = true;
    Color mBackground{20, 22, 28, 255};
    // Optional outline
    bool mShowBorder = false;
    Color mBorderColor{40, 44, 52, 255};
    float mBorderThickness = 1.0f;
};

class RLHeatMap {
public:
    RLHeatMap(Rectangle aBounds, int aCellsX, int aCellsY);
    ~RLHeatMap();

    void setBounds(Rectangle aBounds);
    void setGrid(int aCellsX, int aCellsY);
    void setUpdateMode(RLHeatMapUpdateMode aMode);
    void setDecayHalfLifeSeconds(float aSeconds);
    void setStyle(const RLHeatMapStyle &aStyle);

    // Provide 3 or 4 color stops; interpolated evenly across [0..1]
    void setColorStops(const std::vector<Color> &aStops);

    // Add points in normalized space [-1,1] for both x and y
    void addPoints(const Vector2 *pPoints, size_t aCount);
    void clear();

    void update(float aDt);
    void draw() const;

    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] int getCellsX() const { return mCellsX; }
    [[nodiscard]] int getCellsY() const { return mCellsY; }
    [[nodiscard]] RLHeatMapUpdateMode getUpdateMode() const { return mMode; }

private:
    Rectangle mBounds{};
    int mCellsX{64};
    int mCellsY{64};
    RLHeatMapUpdateMode mMode{RLHeatMapUpdateMode::Accumulate};
    RLHeatMapStyle mStyle{};

    // Aggregation grid
    std::vector<float> mCounts;
    float mMaxValue{1.0f};
    bool mCountsDirty{false};

    // Color mapping
    std::vector<Color> mStops; // 3 or 4
    Color mLut[256]{};
    bool mLutDirty{true};

    // Texture resources (cpu pixels reused)
    std::vector<unsigned char> mPixels; // RGBA
    Texture2D mTexture{};
    bool mTextureValid{false};
    bool mTextureDirty{false};

    // Decay (exponential by half-life)
    float mDecayHalfLife{0.0f};

    void ensureGrid(int aCellsX, int aCellsY);
    void rebuildLUT();
    void rebuildTextureIfNeeded();
    void updateTexturePixels();
};
