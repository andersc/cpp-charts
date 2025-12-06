#pragma once
#include "raylib.h"
#include <vector>
#include <functional>
#include <string>

// High-performance Log-Log Plot for raylib with streaming time series support.
// Designed for Allan variance-style analysis with dynamic confidence intervals.

// Confidence interval data for a single analysis point
struct RLLogPlotConfidence {
    float mLowerBound{ 0.0f };  // Lower bound (in log space or linear, depending on trace settings)
    float mUpperBound{ 0.0f };  // Upper bound
    bool mEnabled{ false };     // Whether to show this interval
};

// Individual trace style configuration
struct RLLogPlotTraceStyle {
    Color mLineColor{ 80, 180, 255, 255 };
    float mLineThickness{ 2.5f };
    bool mShowPoints{ true };
    float mPointRadius{ 4.0f };
    Color mPointColor{ 0, 0, 0, 0 }; // if a==0, derive from line color

    // Confidence interval styling
    bool mShowConfidenceIntervals{ true };
    Color mConfidenceColor{ 0, 0, 0, 0 }; // if a==0, derive from line color with transparency
    float mConfidenceAlpha{ 0.3f };       // Alpha multiplier for confidence regions
    bool mConfidenceAsBars{ false };      // true: error bars, false: shaded band
    float mConfidenceBarWidth{ 8.0f };    // Width of error bar caps (if bars)
};

// Single trace in the log-log plot
struct RLLogPlotTrace {
    std::vector<float> mXValues;  // X-axis values (e.g., tau for Allan variance)
    std::vector<float> mYValues;  // Y-axis values (e.g., Allan deviation)
    std::vector<RLLogPlotConfidence> mConfidence; // Per-point confidence intervals
    RLLogPlotTraceStyle mStyle{};

    // Animation state (internal, managed by RLLogPlot)
    mutable std::vector<float> mAnimX;
    mutable std::vector<float> mAnimY;
    mutable std::vector<float> mAnimConfLower;
    mutable std::vector<float> mAnimConfUpper;
    mutable std::vector<float> mVisibility; // 0..1 per point
    mutable bool mDirty{ true };
};

// Style for the log-log analysis plot
struct RLLogPlotStyle {
    bool mShowBackground{ true };
    Color mBackground{ 20, 22, 28, 255 };
    Color mAxesColor{ 120, 125, 135, 255 };
    Color mGridColor{ 45, 48, 55, 255 };
    Color mTextColor{ 180, 185, 195, 255 };
    float mPadding{ 50.0f };

    // Grid and tick configuration
    bool mShowGrid{ true };
    bool mShowMinorGrid{ false };
    Color mMinorGridColor{ 35, 38, 42, 255 };

    // Axis ranges (log10 space)
    bool mAutoScaleX{ true };
    bool mAutoScaleY{ true };
    float mMinLogX{ -2.0f };  // 10^-2
    float mMaxLogX{ 3.0f };   // 10^3
    float mMinLogY{ -6.0f };  // 10^-6
    float mMaxLogY{ 0.0f };   // 10^0 = 1

    // Animation
    bool mSmoothAnimate{ true };
    float mAnimSpeed{ 6.0f };  // Approach speed for smooth transitions

    // Title and labels
    std::string mTitle{};
    std::string mXAxisLabel{};
    std::string mYAxisLabel{};
    float mFontSize{ 14.0f };
    float mTitleFontSize{ 18.0f };
    Font mFont{};                 // Optional custom font; if .baseSize==0 use default
};

// Style for time series view
struct RLTimeSeriesStyle {
    bool mShowBackground{ true };
    Color mBackground{ 18, 20, 24, 255 };
    Color mLineColor{ 100, 200, 255, 255 };
    float mLineThickness{ 1.5f };
    Color mAxesColor{ 100, 105, 115, 255 };
    Color mGridColor{ 35, 38, 42, 255 };
    Color mTextColor{ 160, 165, 175, 255 };
    float mPadding{ 40.0f };

    bool mShowGrid{ true };
    bool mAutoScaleY{ true };
    float mMinY{ -1.0f };
    float mMaxY{ 1.0f };

    // Fill under curve
    bool mFillUnderCurve{ false };
    Color mFillColor{ 100, 200, 255, 60 };

    std::string mTitle{};
    std::string mYAxisLabel{};
    float mFontSize{ 12.0f };
    Font mFont{};                 // Optional custom font; if .baseSize==0 use default
};

// Main class: dual-view plot system with time series + log-log analysis
class RLLogPlot {
public:
    // Constructor: specify bounds for entire plot area
    explicit RLLogPlot(Rectangle aBounds);

    // Configuration
    void setBounds(Rectangle aBounds);
    void setTimeSeriesHeight(float aHeightFraction); // 0..1, default 0.35
    void setLogPlotStyle(const RLLogPlotStyle& rStyle);
    void setTimeSeriesStyle(const RLTimeSeriesStyle& rStyle);

    // Time series data management
    void setWindowSize(size_t aMaxSamples);
    void pushSample(float aValue);           // Add one sample (FIFO)
    void pushSamples(const std::vector<float>& rValues); // Add multiple
    void clearTimeSeries();
    [[nodiscard]] size_t getTimeSeriesSize() const { return mTimeSeries.size(); }
    [[nodiscard]] size_t getWindowSize() const { return mMaxWindowSize; }

    // Log-log trace management
    void clearTraces();
    size_t addTrace(const RLLogPlotTrace& rTrace);
    void setTrace(size_t aIndex, const RLLogPlotTrace& rTrace);
    void updateTraceData(size_t aIndex, const std::vector<float>& rXValues,
                         const std::vector<float>& rYValues,
                         const std::vector<RLLogPlotConfidence>* pConfidence = nullptr);
    [[nodiscard]] size_t getTraceCount() const { return mTraces.size(); }

    // Update animation state (call each frame)
    void update(float aDt);

    // Draw both plots
    void draw() const;

    // Helpers
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] Rectangle getTimeSeriesBounds() const;
    [[nodiscard]] Rectangle getLogPlotBounds() const;

    // Direct access for advanced usage
    [[nodiscard]] const std::vector<float>& getTimeSeries() const { return mTimeSeries; }
    [[nodiscard]] std::vector<RLLogPlotTrace>& getTraces() { return mTraces; }

private:
    Rectangle mBounds{};
    float mTimeSeriesHeightFraction{ 0.35f };
    float mGapBetweenPlots{ 20.0f };

    // Time series data
    std::vector<float> mTimeSeries;
    size_t mMaxWindowSize{ 1000 };

    // Log-log traces
    std::vector<RLLogPlotTrace> mTraces;

    // Styles
    RLLogPlotStyle mLogPlotStyle{};
    RLTimeSeriesStyle mTimeSeriesStyle{};

    // Cached layout
    mutable Rectangle mTimeSeriesRect{};
    mutable Rectangle mLogPlotRect{};
    mutable bool mLayoutDirty{ true };

    // Cached scale for log plot
    mutable float mLogMinX{ -2.0f };
    mutable float mLogMaxX{ 3.0f };
    mutable float mLogMinY{ -6.0f };
    mutable float mLogMaxY{ 0.0f };
    mutable bool mScaleDirty{ true };

    // Helper methods
    void updateLayout() const;
    void updateLogScale() const;
    void drawTimeSeries() const;
    void drawLogPlot() const;
    void drawLogGrid(Rectangle aPlotRect) const;
    void drawLogAxes(Rectangle aPlotRect) const;
    void drawLogTrace(const RLLogPlotTrace& rTrace, Rectangle aPlotRect) const;

    Vector2 mapLogPoint(float aLogX, float aLogY, Rectangle aRect) const;
    void ensureTraceAnimation(RLLogPlotTrace& rTrace) const;

    static inline float clamp01(float aX) { return aX < 0.0f ? 0.0f : (aX > 1.0f ? 1.0f : aX); }
    static inline float approach(float a, float b, float aSpeedDt);
    static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
    static inline Color fadeColor(Color aC, float aAlpha);
};

