#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Animated Pie/Donut chart for raylib
// Usage: construct with bounds, setData(), then per-frame update(dt) and draw().

struct RLPieSliceData {
    float mValue{0.0f};
    Color mColor{80, 180, 255, 255};
    std::string mLabel; // optional
};

struct RLPieChartStyle {
    // Background
    bool mShowBackground{ true };
    Color mBackground{ 20, 22, 28, 255 };

    // Layout
    float mPadding{ 8.0f }; // inner padding inside bounds

    // Animation
    bool mSmoothAnimate{ true };
    float mAngleSpeed{ 8.0f }; // approach speed for angles (1/s)
    float mFadeSpeed{ 8.0f };  // approach speed for visibility (1/s)
    float mColorSpeed{ 6.0f }; // color blend speed (1/s)
};

class RLPieChart {
public:
    explicit RLPieChart(Rectangle aBounds, const RLPieChartStyle &aStyle = {});

    void setBounds(Rectangle aBounds);
    void setStyle(const RLPieChartStyle &aStyle);

    // Hollow factor [0..1]: innerRadius = outerRadius * factor
    void setHollowFactor(float aFactor);
    [[nodiscard]] float getHollowFactor() const { return mHollowFactor; }

    // Data setting (immediate) and target (animated)
    void setData(const std::vector<RLPieSliceData> &aData);
    void setTargetData(const std::vector<RLPieSliceData> &aData);

    // Per-frame update
    void update(float aDt);
    // Draw
    void draw() const;

    [[nodiscard]] Rectangle getBounds() const { return mBounds; }

private:
    struct SliceDyn {
        float mValue{0.0f};
        float mTarget{0.0f};
        Color mColor{80, 180, 255, 255};
        Color mColorTarget{80, 180, 255, 255};
        // Angles in degrees
        float mStart{0.0f};
        float mEnd{0.0f};
        float mStartTarget{0.0f};
        float mEndTarget{0.0f};
        float mVis{1.0f};        // [0..1]
        float mVisTarget{1.0f};  // [0..1]
        std::string mLabel;
    };

    Rectangle mBounds{};
    RLPieChartStyle mStyle{};
    std::vector<SliceDyn> mSlices;
    size_t mTargetCount{0};
    float mHollowFactor{0.0f};

    // Cached geometry
    mutable bool mGeomDirty{ true };
    mutable Vector2 mCenter{0,0};
    mutable float mOuterRadius{0.0f};

    void ensureSize(size_t aCount);
    void recomputeTargetsFromData(const std::vector<RLPieSliceData> &aData);
    void ensureGeometry() const;

    static inline float clamp01(float aX){ return aX < 0 ? 0 : (aX > 1 ? 1 : aX); }
    static inline float approach(float a, float b, float speedDt){ float d = b - a; return a + d * (d * d < 1e-8f ? 1.0f : clamp01(speedDt)); }
    [[nodiscard]] Color lerp(const Color &a, const Color &b, float t) const;
};
