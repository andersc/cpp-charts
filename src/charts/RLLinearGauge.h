// RLLinearGauge.h
#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// Orientation for the linear gauge
enum class RLLinearGaugeOrientation {
    HORIZONTAL,
    VERTICAL
};

// Pointer/indicator style
enum class RLLinearGaugePointerStyle {
    FILL_BAR,       // Filled bar from min to current value
    TRIANGLE,       // Triangle pointer at current value
    LINE_MARKER     // Line marker at current value
};

// Colored range band (e.g., green/yellow/red zones)
struct RLLinearGaugeRangeBand {
    float mMin{0.0f};
    float mMax{100.0f};
    Color mColor{GREEN};
};

// Style configuration for the linear gauge
struct RLLinearGaugeStyle {
    // Track (background bar) appearance
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

// A lightweight, performant linear gauge for raylib.
// Supports horizontal/vertical orientation, colored range bands, smooth animation.
class RLLinearGauge {
public:
    RLLinearGauge(Rectangle aBounds, float aMinValue, float aMaxValue,
                  RLLinearGaugeOrientation aOrientation = RLLinearGaugeOrientation::HORIZONTAL,
                  const RLLinearGaugeStyle &aStyle = {});

    // Value control
    void setValue(float aValue);              // Set value immediately (no animation)
    void setTargetValue(float aValue);        // Animate towards target
    [[nodiscard]] float getValue() const { return mValue; }
    [[nodiscard]] float getTargetValue() const { return mTargetValue; }

    // Range and configuration
    void setRange(float aMinValue, float aMaxValue);
    void setBounds(Rectangle aBounds);
    void setOrientation(RLLinearGaugeOrientation aOrientation);
    void setStyle(const RLLinearGaugeStyle &aStyle);
    void setPointerStyle(RLLinearGaugePointerStyle aStyle);
    void setAnimationEnabled(bool aEnabled);  // Toggle smooth animation

    // Tick marks
    void setTicks(int aMajorCount, int aMinorPerMajor);

    // Labels
    void setLabel(const std::string &aTitle);
    void setUnit(const std::string &aUnit);

    // Range bands (colored zones)
    void setRanges(const std::vector<RLLinearGaugeRangeBand> &aRanges);
    void clearRanges();

    // Target marker (goal line)
    void setTargetMarker(float aValue);
    void hideTargetMarker();

    // Rendering
    void update(float aDt);
    void draw() const;

private:
    Rectangle mBounds{};
    float mMinValue{0.0f};
    float mMaxValue{100.0f};
    float mValue{0.0f};
    float mTargetValue{0.0f};

    RLLinearGaugeOrientation mOrientation{RLLinearGaugeOrientation::HORIZONTAL};
    RLLinearGaugePointerStyle mPointerStyle{RLLinearGaugePointerStyle::FILL_BAR};
    RLLinearGaugeStyle mStyle{};

    std::string mTitle{};
    std::string mUnit{};
    std::vector<RLLinearGaugeRangeBand> mRangeBands{};

    float mTargetMarkerValue{0.0f};
    bool mShowTargetMarker{false};

    // Cached geometry for ticks to avoid per-frame recalculation
    struct TickGeom {
        Vector2 mP0{};
        Vector2 mP1{};
        float mValue{0.0f};
        bool mMajor{false};
    };
    std::vector<TickGeom> mTicks{};

    // Cached track geometry
    Rectangle mTrackRect{};

    void recomputeGeometry();
    [[nodiscard]] float valueToPosition(float aValue) const;
    [[nodiscard]] float clampValue(float aValue) const;
    void drawBackground() const;
    void drawRangeBands() const;
    void drawTrack() const;
    void drawFill() const;
    void drawPointer() const;
    void drawTicks() const;
    void drawLabels() const;
    void drawTargetMarker() const;
    void drawValueText() const;
    void drawTitle() const;
};

