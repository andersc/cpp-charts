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

// Mode for the linear gauge
enum class RLLinearGaugeMode {
    STANDARD,       // Normal gauge with single value
    VU_METER        // Multi-channel VU meter with peak hold and clip indicator
};

// Colored range band (e.g., green/yellow/red zones)
struct RLLinearGaugeRangeBand {
    float mMin{0.0f};
    float mMax{100.0f};
    Color mColor{GREEN};
};

// VU Meter channel data
struct RLVuMeterChannel {
    float mValue{0.0f};
    std::string mLabel{};
};

// VU Meter style configuration
struct RLVuMeterStyle {
    // Gradient colors (green -> yellow -> red)
    Color mLowColor{80, 200, 120, 255};      // Green zone
    Color mMidColor{255, 200, 80, 255};      // Yellow zone
    Color mHighColor{255, 80, 80, 255};      // Red zone

    // Thresholds for color zones (normalized 0.0 - 1.0)
    float mLowThreshold{0.6f};               // Below this: green
    float mMidThreshold{0.85f};              // Below this: yellow, above: red

    // Peak indicator
    Color mPeakMarkerColor{255, 255, 255, 255};
    float mPeakMarkerThickness{2.0f};
    float mPeakHoldTime{1.5f};               // Seconds to hold peak
    float mPeakDecaySpeed{0.5f};             // Units per second after hold

    // Clip indicator
    Color mClipIndicatorColor{255, 0, 0, 255};
    float mClipFlashDuration{0.3f};          // How long clip indicator flashes
    float mClipIndicatorSize{8.0f};          // Size of clip indicator

    // Channel layout
    float mChannelSpacing{4.0f};             // Gap between channel bars
    bool mShowChannelLabels{true};
    float mChannelLabelFontSize{10.0f};

    // dB scale option
    bool mUseDbScale{false};
    float mDbMin{-60.0f};                    // Minimum dB value (silence)
    float mDbMax{0.0f};                      // Maximum dB value (full scale)
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

    // VU Meter style (used when mode is VU_METER)
    RLVuMeterStyle mVuStyle{};
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

    // VU Meter mode
    void setMode(RLLinearGaugeMode aMode);
    [[nodiscard]] RLLinearGaugeMode getMode() const { return mMode; }
    void setVuMeterStyle(const RLVuMeterStyle &aStyle);

    // VU Meter channels
    void setChannels(const std::vector<RLVuMeterChannel> &aChannels);
    void setChannelValue(int aIndex, float aValue);
    void setChannelValues(const std::vector<float> &aValues);
    [[nodiscard]] int getChannelCount() const { return (int)mChannels.size(); }
    [[nodiscard]] float getPeakValue(int aIndex) const;
    [[nodiscard]] bool isClipping(int aIndex) const;
    void resetPeaks();
    void resetClip();

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
    RLLinearGaugeMode mMode{RLLinearGaugeMode::STANDARD};
    RLLinearGaugeStyle mStyle{};

    std::string mTitle{};
    std::string mUnit{};
    std::vector<RLLinearGaugeRangeBand> mRangeBands{};

    float mTargetMarkerValue{0.0f};
    bool mShowTargetMarker{false};

    // VU Meter state
    std::vector<RLVuMeterChannel> mChannels{};
    std::vector<float> mPeakValues{};
    std::vector<float> mPeakHoldTimers{};
    std::vector<bool> mClipStates{};
    std::vector<float> mClipTimers{};

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

    // VU Meter drawing
    void drawVuMeter() const;
    void drawVuMeterChannel(int aIndex, Rectangle aBounds) const;
    void drawVuMeterPeakMarker(int aIndex, Rectangle aBounds) const;
    void drawVuMeterClipIndicator(int aIndex, Rectangle aBounds) const;
    void drawVuMeterChannelLabel(int aIndex, Rectangle aBounds) const;

    // VU Meter helpers
    [[nodiscard]] float linearToDb(float aLinear) const;
    [[nodiscard]] float dbToLinear(float aDb) const;
    [[nodiscard]] Color getVuMeterColor(float aNormalizedValue) const;
    [[nodiscard]] Rectangle getChannelBounds(int aIndex) const;
};

