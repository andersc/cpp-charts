#pragma once
#include "raylib.h"
#include "RLCommon.h"
#include <vector>

// High-performance Scatter Plot for raylib.
// Supports single and multiple series, with linear or spline lines and point markers.

enum class RLScatterLineMode {
    None,     // scatter-only (no connecting line)
    Linear,   // straight segments
    Spline    // Catmull-Rom spline
};

struct RLScatterSeriesStyle {
    Color mLineColor{ 80, 180, 255, 255 };
    float mLineThickness{ 2.0f };
    RLScatterLineMode mLineMode{ RLScatterLineMode::Linear };

    // Points
    bool mShowPoints{ true };
    Color mPointColor{ 0, 0, 0, 0 }; // if a==0, derive from line color
    float mPointSizePx{ 0.0f };      // if <= 0, derived from thickness * pointScale
    float mPointScale{ 1.5f };       // radius = max(1, thickness * pointScale)
};

struct RLScatterPlotStyle {
    // Background and grid/axes
    bool mShowBackground{ true };
    Color mBackground{ 20, 22, 28, 255 };
    bool mShowAxes{ true };
    Color mAxesColor{ 70, 75, 85, 255 };
    bool mShowGrid{ false };
    Color mGridColor{ 40, 44, 52, 255 };
    int mGridLines{ 4 };

    // Padding inside bounds (for labels if any, or just breathing room)
    float mPadding{ 10.0f };

    // Scaling
    bool mAutoScale{ true };
    float mMinX{ 0.0f };
    float mMaxX{ 1.0f };
    float mMinY{ 0.0f };
    float mMaxY{ 1.0f };

    // Spline quality (higher = smoother, more points). Base target pixels per segment.
    float mSplinePixels{ 6.0f }; // approximate pixels between samples

    // Animation
    bool mSmoothAnimate{ true };
    float mMoveSpeed{ 8.0f };  // position approach speed (1/s)
    float mFadeSpeed{ 6.0f };  // visibility fade speed (1/s)
};

struct RLScatterSeries {
    // Raw data points (unscaled). If you pass normalized [0,1], set scale accordingly or keep auto.
    std::vector<Vector2> mData;
    // Target data used for animation and autoscale when animating
    std::vector<Vector2> mTargetData;
    RLScatterSeriesStyle mStyle{};

    // Internal cache (mutable, maintained by RLScatterPlot)
    mutable std::vector<Vector2> mCache; // mapped to screen space for fast drawing
    mutable std::vector<Vector2> mSpline; // sampled spline polyline (screen space)
    mutable std::vector<float> mSplineVis; // visibility along spline samples
    mutable std::vector<float> mCacheVis; // per cached point visibility [0..1]
    mutable bool mDirty{ true };

    // Animation state (screen space independent; works in data space then mapped)
    mutable std::vector<Vector2> mDynPos;       // current data-space positions
    mutable std::vector<Vector2> mDynTarget;    // target data-space positions
    mutable std::vector<float> mVis;            // [0..1]
    mutable std::vector<float> mVisTarget;      // [0..1]
};

class RLScatterPlot {
public:
    explicit RLScatterPlot(Rectangle aBounds, const RLScatterPlotStyle &rStyle = {});

    void setBounds(Rectangle aBounds);
    void setStyle(const RLScatterPlotStyle &rStyle);

    // Scaling: if autoScale==false in style, these are used
    void setScale(float aMinX, float aMaxX, float aMinY, float aMaxY);

    // Series management
    void clearSeries();
    size_t addSeries(const RLScatterSeries &rSeries); // returns index
    void setSeries(size_t aIndex, const RLScatterSeries &rSeries);
    [[nodiscard]] size_t seriesCount() const { return mSeries.size(); }

    // Convenience: single-series API
    void setSingleSeries(const std::vector<Vector2> &rData, const RLScatterSeriesStyle &aStyle = {});

    // Animated data update APIs
    void setSeriesTargetData(size_t aIndex, const std::vector<Vector2> &rData);
    void setSingleSeriesTargetData(const std::vector<Vector2> &rData);

    // Step animation (call each frame with dt seconds)
    void update(float aDt);

    // Draw chart
    void draw() const;

    // Helpers
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }

private:
    Rectangle mBounds{};
    RLScatterPlotStyle mStyle{};
    std::vector<RLScatterSeries> mSeries;

    // Cached global scale derived from style and data
    mutable float mScaleMinX{ 0.0f };
    mutable float mScaleMaxX{ 1.0f };
    mutable float mScaleMinY{ 0.0f };
    mutable float mScaleMaxY{ 1.0f };
    mutable bool mScaleDirty{ true };
    mutable Rectangle mPlotRect{}; // bounds minus padding
    mutable bool mGeomDirty{ true };

    void markAllDirty() const;
    [[nodiscard]] Rectangle plotRect() const;
    void ensureScale() const;
    [[nodiscard]] Vector2 mapPoint(const Vector2 &rPt) const;

    static Vector2 catmullRom(const Vector2 &rP0, const Vector2 &rP1, const Vector2 &rP2, const Vector2 &rP3, float aT);
    static float dist(const Vector2 &a, const Vector2 &b);
    void buildCaches() const;

    void ensureDynInitialized(const RLScatterSeries &rSeries) const;
};
