// RLRadarChart.h
#pragma once
#include "raylib.h"
#include "RLCommon.h"
#include <vector>
#include <string>

// Radar / Spider chart for raylib - supports single and multiple series
// with smooth animations, configurable styling, and per-axis value ranges.
// Usage: construct with bounds, setAxes(), addSeries(), then per-frame update(dt) and draw().

// Normalization mode for mapping values to radius
enum class RLRadarNormMode {
    GLOBAL,     // Use global min/max across all axes
    PER_AXIS    // Use per-axis min/max ranges
};

// Axis definition
struct RLRadarAxis {
    std::string mLabel;
    float mMin{0.0f};       // Minimum value for this axis
    float mMax{100.0f};     // Maximum value for this axis
};

// Series definition
struct RLRadarSeries {
    std::string mLabel;
    std::vector<float> mValues;         // One value per axis
    Color mLineColor{80, 180, 255, 255};
    Color mFillColor{80, 180, 255, 80}; // Fill with alpha
    float mLineThickness{2.0f};
    bool mShowFill{true};
    bool mShowMarkers{true};
    float mMarkerScale{1.5f};           // Marker radius = lineThickness * markerScale
};

// Style configuration
struct RLRadarChartStyle {
    // Background
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};

    // Grid (spider web)
    bool mShowGrid{true};
    int mGridRings{5};                  // Number of concentric rings
    Color mGridColor{50, 55, 65, 255};
    float mGridThickness{1.0f};

    // Axis lines (radial spokes)
    bool mShowAxes{true};
    Color mAxisColor{70, 75, 85, 255};
    float mAxisThickness{1.0f};

    // Labels
    bool mShowLabels{true};
    Color mLabelColor{180, 190, 210, 255};
    Font mLabelFont{};
    int mLabelFontSize{12};
    float mLabelOffset{12.0f};          // Distance from chart edge to labels

    // Legend
    bool mShowLegend{true};
    float mLegendPadding{8.0f};

    // Chart area
    float mPadding{60.0f};              // Padding from bounds to chart area

    // Normalization
    RLRadarNormMode mNormMode{RLRadarNormMode::GLOBAL};

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{6.0f};          // Interpolation speed
    float mFadeSpeed{4.0f};             // Fade in/out speed for series add/remove
};

class RLRadarChart {
public:
    explicit RLRadarChart(Rectangle aBounds, const RLRadarChartStyle& rStyle = {});
    ~RLRadarChart() = default;

    // Configuration
    void setBounds(Rectangle aBounds);
    void setStyle(const RLRadarChartStyle& rStyle);

    // Axes configuration
    void setAxes(const std::vector<RLRadarAxis>& rAxes);
    void setAxes(const std::vector<std::string>& rLabels, float aMin = 0.0f, float aMax = 100.0f);

    // Series management
    void addSeries(const RLRadarSeries& rSeries);
    void setSeriesData(size_t aIndex, const std::vector<float>& rValues);
    void setSeriesData(size_t aIndex, const RLRadarSeries& rSeries);
    void removeSeries(size_t aIndex);
    void clearSeries();

    // Per-frame update and draw
    void update(float aDt);
    void draw() const;

    // Getters
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] size_t getAxisCount() const { return mAxes.size(); }
    [[nodiscard]] size_t getSeriesCount() const { return mTargetSeriesCount; }

private:
    // Internal series with animation state
    struct SeriesDyn {
        std::string mLabel;
        std::vector<float> mValues;         // Current animated values
        std::vector<float> mTargets;        // Target values
        Color mLineColor{80, 180, 255, 255};
        Color mFillColor{80, 180, 255, 80};
        Color mLineColorTarget{80, 180, 255, 255};
        Color mFillColorTarget{80, 180, 255, 80};
        float mLineThickness{2.0f};
        float mLineThicknessTarget{2.0f};
        bool mShowFill{true};
        bool mShowMarkers{true};
        float mMarkerScale{1.5f};
        float mVisibility{0.0f};            // 0 = hidden, 1 = fully visible
        float mVisibilityTarget{1.0f};
        bool mPendingRemoval{false};

        // Cached vertex positions (in screen coords)
        mutable std::vector<Vector2> mCachedPoints;
        mutable bool mCacheDirty{true};
    };

    // Geometry computation
    void computeGeometry() const;
    void computeSeriesPoints(const SeriesDyn& rSeries) const;
    float normalizeValue(float aValue, size_t aAxisIndex) const;
    Vector2 getPointOnAxis(size_t aAxisIndex, float aNormalizedValue) const;

    // Drawing helpers
    void drawBackground() const;
    void drawGrid() const;
    void drawAxes() const;
    void drawAxisLabels() const;
    void drawSeries(const SeriesDyn& rSeries) const;
    void drawLegend() const;


    // Member data
    Rectangle mBounds{};
    RLRadarChartStyle mStyle{};
    std::vector<RLRadarAxis> mAxes;
    std::vector<SeriesDyn> mSeries;
    size_t mTargetSeriesCount{0};

    // Cached geometry (recomputed when bounds change)
    mutable bool mGeomDirty{true};
    mutable Vector2 mCenter{0, 0};
    mutable float mRadius{0.0f};
    mutable std::vector<float> mAxisAngles;     // Precomputed angles in radians
    mutable std::vector<Vector2> mAxisEndpoints; // Outer points of each axis

    // Global range (computed from axes when in GLOBAL mode)
    mutable float mGlobalMin{0.0f};
    mutable float mGlobalMax{100.0f};
    mutable bool mRangeDirty{true};
};

