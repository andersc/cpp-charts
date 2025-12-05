// RLBarChart.h
#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Fast animated Bar Chart for raylib (vertical or horizontal).
// Usage: construct with bounds + orientation, call Update(dt) then Draw() each frame.

enum class RLBarOrientation {
    VERTICAL,
    HORIZONTAL
};

struct RLBarData {
    float value{0.0f};
    Color color{80, 180, 255, 255};
    bool showBorder{false};
    Color borderColor{0, 0, 0, 120};
    std::string label; // optional
};

struct RLBarChartStyle {
    // Background and grid
    bool mShowBackground = true;
    Color mBackground{20, 22, 28, 255};
    bool mShowGrid = false;
    Color mGridColor{40, 44, 52, 255};
    int mGridLines = 4; // per value axis

    // Bars
    float mPadding = 14.0f;      // inner padding within bounds
    float mSpacing = 10.0f;      // spacing between bars
    float mCornerRadius = 5.0f;  // rounded corner radius
    float mBorderThickness = 2.0f;

    // Labels
    bool mShowLabels = true;
    bool mAutoLabelColor = true; // choose white/black based on bar color
    Color mLabelColor{230, 230, 235, 255}; // used if autoLabelColor == false
    Font mLabelFont{}; // optional;
    int mLabelFontSize = 18;

    // Scaling & animation
    bool mAutoScale = true;      // compute max from data
    float mMinValue = 0.0f;      // used when autoScale == false
    float mMaxValue = 100.0f;    // used when autoScale == false
    bool mSmoothAnimate = true;
    float mAnimateSpeed = 8.0f;  // larger = snappier
};

class RLBarChart {
public:
    RLBarChart(Rectangle aBounds, RLBarOrientation aOrientation, const RLBarChartStyle &rStyle = {});

    void setBounds(Rectangle aBounds);
    void setOrientation(RLBarOrientation aOrientation);
    void setStyle(const RLBarChartStyle &rStyle);

    // Set current/target data
    void setData(const std::vector<RLBarData> &rData);
    void setTargetData(const std::vector<RLBarData> &rData);

    // Optional explicit scale (autoScale=false)
    void setScale(float aMinValue, float aMaxValue);

    // Update animation (seconds)
    void update(float aDt);
    // Draw chart
    void draw() const;

    // Helpers
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] RLBarOrientation getOrientation() const { return mOrientation; }

private:
    struct BarDyn {
        float mValue{0.0f};
        float mTarget{0.0f};
        Color mColor{80, 180, 255, 255};
        Color mColorTarget{80, 180, 255, 255};
        // Visibility animation [0..1]: drives fade and size grow/shrink
        float mVisAlpha{1.0f};
        float mVisTarget{1.0f};
        bool mShowBorder{false};
        Color mBorderColor{0, 0, 0, 120};
        std::string mLabel;
    };

    Rectangle mBounds{};
    RLBarOrientation mOrientation{RLBarOrientation::VERTICAL};
    RLBarChartStyle mStyle{};

    std::vector<BarDyn> mBars;
    float mScaleMin{0.0f};
    float mScaleMax{1.0f};
    float mScaleMaxTarget{1.0f};
    size_t mTargetCount{0};

    void ensureSize(size_t aCount);
    void recomputeScaleTargetsFromData(const std::vector<RLBarData> &rData);
    [[nodiscard]] float computeAutoMaxFromTargets() const;
    [[nodiscard]] float lerp(float a, float b, float t) const;
    [[nodiscard]] Color lerp(const Color &a, const Color &b, float t) const;
};
