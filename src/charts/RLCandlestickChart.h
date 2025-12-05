// RLCandlestickChart.h
#pragma once

#include "raylib.h"
#include <deque>
#include <string>
#include <vector>

struct RLCandleStyle {
    // Layout
    float mPadding = 8.0f;                   // chart inner padding
    float mCandleSpacing = 4.0f;             // space between candles
    float mBodyMinWidth = 6.0f;              // minimal body width
    float mWickThickness = 2.0f;             // wick line thickness
    float mVolumeAreaRatio = 0.25f;          // part of total height reserved for volume

    // Colors
    Color mBackground{20, 22, 28, 255};
    Color mGridColor{40, 44, 52, 120};
    int mGridLines = 4;
    Color mUpBody{60, 190, 120, 255};
    Color mUpWick{180, 240, 200, 255};
    Color mDownBody{220, 90, 90, 255};
    Color mDownWick{255, 200, 200, 255};
    Color mSeparator{200, 200, 200, 90};     // daily separator
    Color mVolumeUp{90, 180, 120, 180};
    Color mVolumeDown{200, 90, 90, 180};

    // Animation
    float mSlideSpeed = 8.0f;                // larger = faster slide (units per second: bodyWidth)
    float mFadeSpeed = 6.0f;                 // alpha lerp speed

    // Scaling
    bool mAutoScale = true;
    float lMinPrice = 0.0f;
    float lMaxPrice = 1.0f;
    bool mIncludeWicksInScale = true;
};

class RLCandlestickChart {
public:
    struct CandleInput {
        float aOpen{0.0f};
        float aHigh{0.0f};
        float aLow{0.0f};
        float aClose{0.0f};
        float aVolume{0.0f};
        std::string aDate; // e.g. 2024-01-15 09:35:00
    };

    RLCandlestickChart(Rectangle bounds, int valuesPerCandle, int visibleCandles, const RLCandleStyle &style = {});

    void setBounds(Rectangle aBounds);
    void setValuesPerCandle(int aValuesPerCandle);
    void setVisibleCandles(int aVisibleCandles);
    void setStyle(const RLCandleStyle &rStyle);
    void setExplicitScale(float aMinPrice, float aMaxPrice);

    // Stream a single OHLCV sample. After mValuesPerCandle samples the current candle is finalized
    void addSample(const CandleInput &rSample);

    // Update time-based animations
    void update(float aDt);
    // Draw chart
    void draw() const;

private:
    struct CandleDyn {
        // Aggregated data
        float mOpen{0.0f};
        float mHigh{0.0f};
        float mLow{0.0f};
        float mClose{0.0f};
        float mVolume{0.0f};
        std::string mDate;      // full date-time string
        std::string mDayKey;    // yyyy-mm-dd
        bool mDaySeparator{false};
    };

    Rectangle mBounds{};        // total bounds (price + volume)
    RLCandleStyle mStyle{};
    int mValuesPerCandle{5};
    int mVisibleCandles{20};

    // Working/animated state
    std::deque<CandleDyn> mCandles; // finalized (visible) candles
    CandleDyn mWorking{};           // currently aggregating candle (not yet in deque)
    int mWorkingCount{0};
    bool mHasWorking{false};

    // Incoming candle waiting to be placed after slide completes
    CandleDyn mIncoming{};
    bool mHasIncoming{false};

    // Animation state when a new candle is finalized
    float mSlideProgress{0.0f}; // 0..1 of one candle width slide
    bool mIsSliding{false};

    // Scaling
    float mScaleMin{0.0f};
    float mScaleMax{1.0f};
    float mScaleTargetMax{1.0f};

    // Track last close price for single-value candles
    float mLastClose{0.0f};
    bool mHasLastClose{false};

    // Helpers
    [[nodiscard]] float extractPriceMax() const;
    [[nodiscard]] Rectangle priceArea() const;
    [[nodiscard]] Rectangle volumeArea() const;
    static std::string dayKeyFromDate(const std::string &aDate);
    void finalizeWorkingCandle();
    void ensureWindow();
};
