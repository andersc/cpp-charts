// RLAreaChart.h
#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Area Chart for raylib - supports overlapped, stacked, and 100% stacked modes.
// Usage: construct with bounds + mode, call update(dt) then draw() each frame.

enum class RLAreaChartMode {
    OVERLAPPED,     // Areas overlap (transparency shows layering)
    STACKED,        // Areas stack on top of each other
    PERCENT         // 100% stacked (normalized to percentage)
};

struct RLAreaSeries {
    std::vector<float> mValues;
    Color mColor{80, 180, 255, 255};
    std::string mLabel;
    float mAlpha{0.6f};         // Fill opacity
};

struct RLAreaChartStyle {
    // Background and grid
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};
    bool mShowGrid{true};
    Color mGridColor{40, 44, 52, 255};
    int mGridLines{5};

    // Axes
    Color mAxisColor{180, 180, 180, 255};
    Color mLabelColor{200, 200, 200, 255};

    // Chart area
    float mPadding{40.0f};
    float mLineThickness{2.0f};
    bool mShowPoints{false};
    float mPointRadius{4.0f};

    // Labels
    bool mShowLabels{true};
    Font mLabelFont{};
    int mLabelFontSize{12};

    // Legend
    bool mShowLegend{true};

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{6.0f};
};

class RLAreaChart {
public:
    RLAreaChart(Rectangle aBounds, RLAreaChartMode aMode = RLAreaChartMode::STACKED,
                const RLAreaChartStyle& rStyle = {});
    ~RLAreaChart() = default;

    void setBounds(Rectangle aBounds);
    void setMode(RLAreaChartMode aMode);
    void setStyle(const RLAreaChartStyle& rStyle);

    void setData(const std::vector<RLAreaSeries>& rSeries);
    void setTargetData(const std::vector<RLAreaSeries>& rSeries);
    void setXLabels(const std::vector<std::string>& rLabels);

    void update(float aDt);
    void draw() const;

    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] RLAreaChartMode getMode() const { return mMode; }
    [[nodiscard]] float getMaxValue() const { return mMaxValue; }

private:
    struct SeriesDyn {
        std::vector<float> mValues;
        std::vector<float> mTargets;
        Color mColor{80, 180, 255, 255};
        std::string mLabel;
        float mAlpha{0.6f};
    };

    void calculateMaxValue();
    void drawArea(size_t aSeriesIndex) const;
    void drawAxes() const;
    void drawGrid() const;
    void drawLegend() const;
    float getStackedValue(size_t aSeriesIndex, size_t aPointIndex) const;

    Rectangle mBounds{};
    RLAreaChartMode mMode{RLAreaChartMode::STACKED};
    RLAreaChartStyle mStyle{};
    std::vector<RLAreaSeries> mSeriesData;
    std::vector<SeriesDyn> mSeries;
    std::vector<std::string> mXLabels;
    float mMaxValue{100.0f};
    float mMaxValueTarget{100.0f};
};

