// RLTimeSeries.h
#pragma once
#include "raylib.h"
#include "RLCommon.h"
#include <vector>
#include <cstddef>

// High-performance streaming time series visualizer for raylib.
// Supports multiple overlapping traces, each updated independently.
// Features: configurable window size, build-up/scroll behavior, multiple line modes.

// Line rendering modes for traces
enum class RLTimeSeriesLineMode {
    Raw,     // Simple connected line segments
    Linear,  // Points with linear connecting lines
    Spline   // Points with Catmull-Rom spline interpolation
};

// Per-trace visual style
struct RLTimeSeriesTraceStyle {
    Color mColor{ 80, 180, 255, 255 };
    float mLineThickness{ 2.0f };
    RLTimeSeriesLineMode mLineMode{ RLTimeSeriesLineMode::Linear };
    bool mShowPoints{ false };
    float mPointRadius{ 3.0f };
    bool mVisible{ true };
};

// Single trace data and state
struct RLTimeSeriesTrace {
    RLTimeSeriesTraceStyle mStyle{};

    // Ring buffer for samples
    std::vector<float> mSamples;
    size_t mHead{ 0 };       // Next write position
    size_t mCount{ 0 };      // Current number of samples in buffer

    // Cached screen-space points for drawing
    mutable std::vector<Vector2> mScreenPoints;
    mutable std::vector<Vector2> mSplineCache;
    mutable bool mDirty{ true };
};

// Overall chart style
struct RLTimeSeriesChartStyle {
    // Background
    bool mShowBackground{ true };
    Color mBackground{ 20, 22, 28, 255 };

    // Grid and axes
    bool mShowGrid{ true };
    Color mGridColor{ 40, 44, 52, 255 };
    int mGridLinesX{ 8 };
    int mGridLinesY{ 4 };
    bool mShowAxes{ true };
    Color mAxesColor{ 70, 75, 85, 255 };

    // Padding inside bounds
    float mPadding{ 10.0f };

    // Y-axis scaling
    bool mAutoScaleY{ true };
    float mMinY{ -1.0f };
    float mMaxY{ 1.0f };
    float mAutoScaleMargin{ 0.1f }; // 10% margin above/below data range

    // Spline quality (pixels per segment)
    float mSplinePixels{ 4.0f };

    // Smooth scale transitions
    bool mSmoothScale{ true };
    float mScaleSpeed{ 4.0f };
};

// Main time series visualizer class
class RLTimeSeries {
public:
    explicit RLTimeSeries(Rectangle aBounds, size_t aWindowSize = 500);

    // Configuration
    void setBounds(Rectangle aBounds);
    void setStyle(const RLTimeSeriesChartStyle& rStyle);
    void setWindowSize(size_t aWindowSize);
    [[nodiscard]] size_t getWindowSize() const { return mWindowSize; }

    // Trace management
    size_t addTrace(const RLTimeSeriesTraceStyle& aStyle = {});
    void setTraceStyle(size_t aIndex, const RLTimeSeriesTraceStyle& rStyle);
    void setTraceVisible(size_t aIndex, bool aVisible);
    void clearTrace(size_t aIndex);
    void clearAllTraces();
    [[nodiscard]] size_t getTraceCount() const { return mTraces.size(); }
    [[nodiscard]] size_t getTraceSampleCount(size_t aIndex) const;

    // Sample input
    void pushSample(size_t aTraceIndex, float aValue);
    void pushSamples(size_t aTraceIndex, const float* pValues, size_t aCount);
    void pushSamples(size_t aTraceIndex, const std::vector<float>& rValues);

    // Update and draw
    void update(float aDt);
    void draw() const;

    // Helpers
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] Rectangle getPlotArea() const;

private:
    Rectangle mBounds{};
    size_t mWindowSize{ 500 };
    RLTimeSeriesChartStyle mStyle{};
    std::vector<RLTimeSeriesTrace> mTraces;

    // Animated scale state
    float mCurrentMinY{ -1.0f };
    float mCurrentMaxY{ 1.0f };
    float mTargetMinY{ -1.0f };
    float mTargetMaxY{ 1.0f };

    // Internal helpers
    void updateScale(float aDt);
    void rebuildScreenPoints(size_t aTraceIndex) const;
    void drawTrace(const RLTimeSeriesTrace& rTrace) const;
    void drawGrid() const;
    void drawAxes() const;

    // Ring buffer helpers
    static float getSample(const RLTimeSeriesTrace& rTrace, size_t aIndex);
};

