// RLGauge.h
#pragma once
#include "raylib.h"
#include <vector>

// A lightweight, fast circular gauge for raylib.
// Optimized to avoid per-frame allocations and heavy math where possible.
// Usage: construct with bounds/min/max, call Update(dt) then Draw() each frame.

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

    float mThickness = 18.0f;     // ring thickness (pixels)
    float mStartAngle = 135.0f;   // in degrees
    float mEndAngle   = 405.0f;   // in degrees (end > start)
    int mTickCount    = 60;       // minor ticks
    int mMajorEvery   = 5;        // every Nth tick is major
    float mTickLen    = 8.0f;     // minor tick length
    float mMajorTickLen = 14.0f;  // major tick length
    float mTickThickness = 2.0f;  // minor tick thickness
    float mMajorTickThickness = 3.0f; // major tick thickness
    float mNeedleWidth = 4.0f;    // needle thickness
    float mNeedleRadiusScale = 0.86f; // as fraction of radius
    bool mShowValueText = true;
    bool mShowTicks = true;
    bool mShowNeedle = true;
    bool mSmoothAnimate = true;   // smooth animation towards target
    Font mLabelFont{};            // optional custom font; if .baseSize==0 use default
};

class RLGauge {
public:
    RLGauge(Rectangle bounds, float minValue, float maxValue, const RLGaugeStyle &style = {});

    void setValue(float value);            // set instantly
    void setTargetValue(float value);      // animate towards target
    [[nodiscard]] float getValue() const { return mValue; }
    [[nodiscard]] float getTarget() const { return mTargetValue; }

    void setBounds(Rectangle bounds);
    void setRange(float minValue, float maxValue);
    void setStyle(const RLGaugeStyle &style);

    // dt in seconds
    void update(float dt);
    void draw() const;

private:
    Rectangle mBounds{};
    Vector2   mCenter{};
    float     mRadius{}; // outer radius of ring

    float mMinValue{};
    float mMaxValue{};
    float mValue{};
    float mTargetValue{};

    RLGaugeStyle mStyle{};

    // Cached geometry for ticks to avoid per-frame trig
    struct TickGeom {
        float mAngleDeg;
        Vector2 mP0;
        Vector2 mP1;
        bool mMajor;
    };
    std::vector<TickGeom> mTicks;

    void recomputeGeometry();
    [[nodiscard]] float clamp(float t) const;
    [[nodiscard]] float valueToAngle(float v) const; // returns degrees
};
