// RLHeatMap3D.h
#pragma once
#include "raylib.h"
#include <vector>
#include <cstddef>

// Rendering mode for the 3D plot
enum class RLHeatMap3DMode {
    Surface,    // Connected surface mesh
    Scatter     // Individual points/spheres in 3D space
};

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

    // Axis labels
    bool mShowAxisLabels = true;
    float mLabelFontSize = 14.0f;
    Color mLabelColor{200, 200, 210, 255};

    // Tick marks
    bool mShowTicks = true;
    int mTickCount = 5;
    Color mTickColor{150, 150, 160, 255};
};

class RLHeatMap3D {
public:
    RLHeatMap3D();
    RLHeatMap3D(int aWidth, int aHeight);
    ~RLHeatMap3D();

    // Prevent copy (owns GPU resources)
    RLHeatMap3D(const RLHeatMap3D&) = delete;
    RLHeatMap3D& operator=(const RLHeatMap3D&) = delete;

    // Grid configuration
    void setGridSize(int aWidth, int aHeight);

    // Data input - batch update all values
    void setValues(const float* pValues, int aCount);

    // Data input - partial region update
    void updatePartialValues(int aX, int aY, int aW, int aH, const float* pValues);

    // Palette configuration (3-4 color stops)
    void setPalette(Color aColorA, Color aColorB, Color aColorC);
    void setPalette(Color aColorA, Color aColorB, Color aColorC, Color aColorD);

    // Value range configuration
    void setValueRange(float aMinValue, float aMaxValue);
    void setAutoRange(bool aEnabled);

    // Axis range configuration (X, Y are grid coords, Z is value)
    void setAxisRangeX(float aMin, float aMax);
    void setAxisRangeY(float aMin, float aMax);
    void setAxisRangeZ(float aMin, float aMax);
    void setAxisLabels(const char* pLabelX, const char* pLabelY, const char* pLabelZ);

    // Mode and style configuration
    void setMode(RLHeatMap3DMode aMode);
    void setSmoothing(float aSpeed);
    void setWireframe(bool aEnabled);
    void setPointSize(float aSize);
    void setStyle(const RLHeatMap3DStyle& rStyle);

    // Update animation (call each frame)
    void update(float aDt);

    // Draw the 3D plot (call within BeginMode3D/EndMode3D)
    void draw(Vector3 aPosition, float aScale, const Camera3D& rCamera) const;

    // Getters
    [[nodiscard]] int getWidth() const { return mWidth; }
    [[nodiscard]] int getHeight() const { return mHeight; }
    [[nodiscard]] float getMinValue() const { return mMinValue; }
    [[nodiscard]] float getMaxValue() const { return mMaxValue; }
    [[nodiscard]] bool isAutoRange() const { return mAutoRange; }
    [[nodiscard]] RLHeatMap3DMode getMode() const { return mStyle.mMode; }

private:
    // Grid dimensions
    int mWidth = 0;
    int mHeight = 0;

    // Value storage
    std::vector<float> mCurrentValues;
    std::vector<float> mTargetValues;

    // Value range (Z axis)
    float mMinValue = 0.0f;
    float mMaxValue = 1.0f;
    bool mAutoRange = true;

    // Axis ranges
    float mAxisMinX = 0.0f;
    float mAxisMaxX = 1.0f;
    float mAxisMinY = 0.0f;
    float mAxisMaxY = 1.0f;
    float mAxisMinZ = 0.0f;
    float mAxisMaxZ = 1.0f;

    // Axis labels
    const char* mpLabelX = "X";
    const char* mpLabelY = "Y";
    const char* mpLabelZ = "Z";

    // Style
    RLHeatMap3DStyle mStyle;

    // Palette LUT (256 entries)
    std::vector<Color> mPaletteStops;
    Color mLut[256]{};
    bool mLutDirty = true;

    // Mesh resources (for surface mode)
    Mesh mMesh{};
    Model mModel{};
    bool mMeshValid = false;
    bool mMeshDirty = false;

    // Internal methods
    void rebuildLut();
    void buildMesh();
    void updateMeshVertices();
    void freeMesh();
    float normalizeValue(float aValue) const;
    Color getColorForValue(float aNormalizedValue) const;

    // Drawing helpers
    void drawAxisBox(Vector3 aPosition, float aScale, const Camera3D& rCamera) const;
    void drawFloorGrid(Vector3 aPosition, float aScale) const;
    void drawBackWalls(Vector3 aPosition, float aScale, const Camera3D& rCamera) const;
    void drawSurface(Vector3 aPosition, float aScale) const;
    void drawScatterPoints(Vector3 aPosition, float aScale) const;
    void drawAxisLabelsAndTicks(Vector3 aPosition, float aScale, const Camera3D& rCamera) const;
    float calculateWallAlpha(Vector3 aWallNormal, const Camera3D& rCamera) const;
};

