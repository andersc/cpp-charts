// RLRadarChart.cpp
#include "RLRadarChart.h"
#include <cmath>
#include <algorithm>


// ============================================================================
// Constructor
// ============================================================================

RLRadarChart::RLRadarChart(Rectangle aBounds, const RLRadarChartStyle& rStyle)
    : mBounds(aBounds)
    , mStyle(rStyle)
{
    mGeomDirty = true;
    mRangeDirty = true;
}

// ============================================================================
// Configuration
// ============================================================================

void RLRadarChart::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
    mGeomDirty = true;
    // Mark all series caches dirty
    for (auto& rSeries : mSeries) {
        rSeries.mCacheDirty = true;
    }
}

void RLRadarChart::setStyle(const RLRadarChartStyle& rStyle) {
    mStyle = rStyle;
    mGeomDirty = true;
    mRangeDirty = true;
}

void RLRadarChart::setAxes(const std::vector<RLRadarAxis>& rAxes) {
    mAxes = rAxes;
    mGeomDirty = true;
    mRangeDirty = true;

    // Resize all series values to match axis count
    for (auto& rSeries : mSeries) {
        rSeries.mValues.resize(mAxes.size(), 0.0f);
        rSeries.mTargets.resize(mAxes.size(), 0.0f);
        rSeries.mCacheDirty = true;
    }
}

void RLRadarChart::setAxes(const std::vector<std::string>& rLabels, float aMin, float aMax) {
    std::vector<RLRadarAxis> lAxes;
    lAxes.reserve(rLabels.size());
    for (const auto& rLabel : rLabels) {
        RLRadarAxis lAxis;
        lAxis.mLabel = rLabel;
        lAxis.mMin = aMin;
        lAxis.mMax = aMax;
        lAxes.push_back(lAxis);
    }
    setAxes(lAxes);
}

// ============================================================================
// Series Management
// ============================================================================

void RLRadarChart::addSeries(const RLRadarSeries& rSeries) {
    SeriesDyn lDyn;
    lDyn.mLabel = rSeries.mLabel;
    lDyn.mLineColor = rSeries.mLineColor;
    lDyn.mFillColor = rSeries.mFillColor;
    lDyn.mLineColorTarget = rSeries.mLineColor;
    lDyn.mFillColorTarget = rSeries.mFillColor;
    lDyn.mLineThickness = rSeries.mLineThickness;
    lDyn.mLineThicknessTarget = rSeries.mLineThickness;
    lDyn.mShowFill = rSeries.mShowFill;
    lDyn.mShowMarkers = rSeries.mShowMarkers;
    lDyn.mMarkerScale = rSeries.mMarkerScale;
    lDyn.mVisibility = 0.0f;            // Start invisible, fade in
    lDyn.mVisibilityTarget = 1.0f;
    lDyn.mPendingRemoval = false;
    lDyn.mCacheDirty = true;

    // Initialize values - start at center (0) and animate to target
    size_t lAxisCount = mAxes.size();
    lDyn.mValues.resize(lAxisCount, 0.0f);
    lDyn.mTargets.resize(lAxisCount, 0.0f);

    // Copy target values from input series
    for (size_t i = 0; i < lAxisCount && i < rSeries.mValues.size(); ++i) {
        lDyn.mTargets[i] = rSeries.mValues[i];
    }

    mSeries.push_back(lDyn);
    mTargetSeriesCount = mSeries.size();
    mRangeDirty = true;
}

void RLRadarChart::setSeriesData(size_t aIndex, const std::vector<float>& rValues) {
    if (aIndex >= mSeries.size()) return;

    SeriesDyn& rSeries = mSeries[aIndex];
    size_t lAxisCount = mAxes.size();

    for (size_t i = 0; i < lAxisCount && i < rValues.size(); ++i) {
        rSeries.mTargets[i] = rValues[i];
    }
    rSeries.mCacheDirty = true;
    mRangeDirty = true;
}

void RLRadarChart::setSeriesData(size_t aIndex, const RLRadarSeries& rSeries) {
    if (aIndex >= mSeries.size()) return;

    SeriesDyn& rDyn = mSeries[aIndex];
    rDyn.mLabel = rSeries.mLabel;
    rDyn.mLineColorTarget = rSeries.mLineColor;
    rDyn.mFillColorTarget = rSeries.mFillColor;
    rDyn.mLineThicknessTarget = rSeries.mLineThickness;
    rDyn.mShowFill = rSeries.mShowFill;
    rDyn.mShowMarkers = rSeries.mShowMarkers;
    rDyn.mMarkerScale = rSeries.mMarkerScale;

    size_t lAxisCount = mAxes.size();
    for (size_t i = 0; i < lAxisCount && i < rSeries.mValues.size(); ++i) {
        rDyn.mTargets[i] = rSeries.mValues[i];
    }
    rDyn.mCacheDirty = true;
    mRangeDirty = true;
}

void RLRadarChart::removeSeries(size_t aIndex) {
    if (aIndex >= mSeries.size()) return;

    // Mark for removal with fade-out animation
    mSeries[aIndex].mVisibilityTarget = 0.0f;
    mSeries[aIndex].mPendingRemoval = true;

    // Also shrink toward center
    for (auto& rVal : mSeries[aIndex].mTargets) {
        rVal = mAxes.empty() ? 0.0f : mAxes[0].mMin;
    }

    // Update target count
    mTargetSeriesCount = 0;
    for (const auto& rS : mSeries) {
        if (!rS.mPendingRemoval) {
            mTargetSeriesCount++;
        }
    }
}

void RLRadarChart::clearSeries() {
    mSeries.clear();
    mTargetSeriesCount = 0;
}

// ============================================================================
// Update (Animation)
// ============================================================================

void RLRadarChart::update(float aDt) {
    if (!mStyle.mSmoothAnimate) {
        // Instant update
        for (auto& rSeries : mSeries) {
            rSeries.mValues = rSeries.mTargets;
            rSeries.mVisibility = rSeries.mVisibilityTarget;
            rSeries.mLineColor = rSeries.mLineColorTarget;
            rSeries.mFillColor = rSeries.mFillColorTarget;
            rSeries.mLineThickness = rSeries.mLineThicknessTarget;
            rSeries.mCacheDirty = true;
        }
    } else {
        float lValueSpeed = mStyle.mAnimateSpeed * aDt;
        float lFadeSpeed = mStyle.mFadeSpeed * aDt;

        for (auto& rSeries : mSeries) {
            bool lChanged = false;

            // Animate values
            for (size_t i = 0; i < rSeries.mValues.size(); ++i) {
                float lOld = rSeries.mValues[i];
                rSeries.mValues[i] = RLCharts::approach(rSeries.mValues[i], rSeries.mTargets[i], lValueSpeed);
                if (rSeries.mValues[i] != lOld) {
                    lChanged = true;
                }
            }

            // Animate visibility
            float lOldVis = rSeries.mVisibility;
            rSeries.mVisibility = RLCharts::approach(rSeries.mVisibility, rSeries.mVisibilityTarget, lFadeSpeed);
            if (rSeries.mVisibility != lOldVis) {
                lChanged = true;
            }

            // Animate colors
            rSeries.mLineColor = RLCharts::lerpColor(rSeries.mLineColor, rSeries.mLineColorTarget, lValueSpeed);
            rSeries.mFillColor = RLCharts::lerpColor(rSeries.mFillColor, rSeries.mFillColorTarget, lValueSpeed);

            // Animate line thickness
            rSeries.mLineThickness = RLCharts::approach(rSeries.mLineThickness, rSeries.mLineThicknessTarget, lValueSpeed);

            if (lChanged) {
                rSeries.mCacheDirty = true;
            }
        }
    }

    // Remove fully faded-out series
    mSeries.erase(
        std::remove_if(mSeries.begin(), mSeries.end(),
            [](const SeriesDyn& rS) {
                return rS.mPendingRemoval && rS.mVisibility < 0.001f;
            }),
        mSeries.end()
    );
}

// ============================================================================
// Draw
// ============================================================================

void RLRadarChart::draw() const {
    if (mAxes.size() < 3) return; // Need at least 3 axes for a radar chart

    computeGeometry();

    drawBackground();
    drawGrid();
    drawAxes();

    // Draw series (back to front for proper layering)
    for (const auto& rSeries : mSeries) {
        if (rSeries.mVisibility > 0.001f) {
            drawSeries(rSeries);
        }
    }

    drawAxisLabels();
    drawLegend();
}

// ============================================================================
// Geometry Computation
// ============================================================================

void RLRadarChart::computeGeometry() const {
    if (!mGeomDirty) return;

    // Compute center and radius
    float lPadding = mStyle.mPadding;
    float lWidth = mBounds.width - 2.0f * lPadding;
    float lHeight = mBounds.height - 2.0f * lPadding;
    mRadius = fminf(lWidth, lHeight) * 0.5f;
    mCenter.x = mBounds.x + mBounds.width * 0.5f;
    mCenter.y = mBounds.y + mBounds.height * 0.5f;

    // Compute axis angles (evenly distributed, starting from top)
    size_t lAxisCount = mAxes.size();
    mAxisAngles.resize(lAxisCount);
    mAxisEndpoints.resize(lAxisCount);

    constexpr float LOCAL_PI = 3.14159265358979323846f;
    constexpr float LOCAL_TWO_PI = 2.0f * LOCAL_PI;
    constexpr float LOCAL_HALF_PI = LOCAL_PI * 0.5f;

    float lAngleStep = LOCAL_TWO_PI / (float)lAxisCount;
    float lStartAngle = -LOCAL_HALF_PI; // Start from top (12 o'clock)

    for (size_t i = 0; i < lAxisCount; ++i) {
        float lAngle = lStartAngle + lAngleStep * (float)i;
        mAxisAngles[i] = lAngle;
        mAxisEndpoints[i] = {
            mCenter.x + cosf(lAngle) * mRadius,
            mCenter.y + sinf(lAngle) * mRadius
        };
    }

    mGeomDirty = false;

    // Recompute global range if needed
    if (mRangeDirty) {
        mGlobalMin = mAxes.empty() ? 0.0f : mAxes[0].mMin;
        mGlobalMax = mAxes.empty() ? 100.0f : mAxes[0].mMax;

        for (const auto& rAxis : mAxes) {
            mGlobalMin = fminf(mGlobalMin, rAxis.mMin);
            mGlobalMax = fmaxf(mGlobalMax, rAxis.mMax);
        }

        mRangeDirty = false;
    }
}

void RLRadarChart::computeSeriesPoints(const SeriesDyn& rSeries) const {
    if (!rSeries.mCacheDirty) return;

    size_t lAxisCount = mAxes.size();
    rSeries.mCachedPoints.resize(lAxisCount);

    for (size_t i = 0; i < lAxisCount; ++i) {
        float lValue = (i < rSeries.mValues.size()) ? rSeries.mValues[i] : 0.0f;
        float lNorm = normalizeValue(lValue, i);
        rSeries.mCachedPoints[i] = getPointOnAxis(i, lNorm);
    }

    rSeries.mCacheDirty = false;
}

float RLRadarChart::normalizeValue(float aValue, size_t aAxisIndex) const {
    float lMin, lMax;

    if (mStyle.mNormMode == RLRadarNormMode::PER_AXIS && aAxisIndex < mAxes.size()) {
        lMin = mAxes[aAxisIndex].mMin;
        lMax = mAxes[aAxisIndex].mMax;
    } else {
        lMin = mGlobalMin;
        lMax = mGlobalMax;
    }

    float lRange = lMax - lMin;
    if (lRange < 0.0001f) return 0.5f;

    float lNorm = (aValue - lMin) / lRange;
    return RLCharts::clamp01(lNorm);
}

Vector2 RLRadarChart::getPointOnAxis(size_t aAxisIndex, float aNormalizedValue) const {
    if (aAxisIndex >= mAxisAngles.size()) return mCenter;

    float lAngle = mAxisAngles[aAxisIndex];
    float lR = mRadius * aNormalizedValue;

    return {
        mCenter.x + cosf(lAngle) * lR,
        mCenter.y + sinf(lAngle) * lR
    };
}

// ============================================================================
// Drawing Helpers
// ============================================================================

void RLRadarChart::drawBackground() const {
    if (!mStyle.mShowBackground) return;

    DrawRectangleRec(mBounds, mStyle.mBackground);
}

void RLRadarChart::drawGrid() const {
    if (!mStyle.mShowGrid) return;

    size_t lAxisCount = mAxes.size();
    if (lAxisCount < 3) return;

    int lRings = mStyle.mGridRings;
    Color lGridColor = mStyle.mGridColor;
    float lThickness = mStyle.mGridThickness;

    // Draw concentric rings (polygons)
    for (int ring = 1; ring <= lRings; ++ring) {
        float lFrac = (float)ring / (float)lRings;

        // Draw polygon for this ring
        for (size_t i = 0; i < lAxisCount; ++i) {
            size_t lNext = (i + 1) % lAxisCount;
            Vector2 lP1 = getPointOnAxis(i, lFrac);
            Vector2 lP2 = getPointOnAxis(lNext, lFrac);
            DrawLineEx(lP1, lP2, lThickness, lGridColor);
        }
    }
}

void RLRadarChart::drawAxes() const {
    if (!mStyle.mShowAxes) return;

    size_t lAxisCount = mAxes.size();
    Color lAxisColor = mStyle.mAxisColor;
    float lThickness = mStyle.mAxisThickness;

    // Draw radial lines from center to perimeter
    for (size_t i = 0; i < lAxisCount; ++i) {
        DrawLineEx(mCenter, mAxisEndpoints[i], lThickness, lAxisColor);
    }
}

void RLRadarChart::drawAxisLabels() const {
    if (!mStyle.mShowLabels) return;

    size_t lAxisCount = mAxes.size();
    Font lFont = mStyle.mLabelFont;
    int lFontSize = mStyle.mLabelFontSize;
    Color lColor = mStyle.mLabelColor;
    float lOffset = mStyle.mLabelOffset;

    bool lUseDefaultFont = (lFont.texture.id == 0);

    for (size_t i = 0; i < lAxisCount; ++i) {
        const std::string& rLabel = mAxes[i].mLabel;
        if (rLabel.empty()) continue;

        // Position label beyond the axis endpoint
        float lAngle = mAxisAngles[i];
        Vector2 lPos = {
            mAxisEndpoints[i].x + cosf(lAngle) * lOffset,
            mAxisEndpoints[i].y + sinf(lAngle) * lOffset
        };

        // Measure text for centering
        Vector2 lTextSize;
        if (lUseDefaultFont) {
            lTextSize.x = (float)MeasureText(rLabel.c_str(), lFontSize);
            lTextSize.y = (float)lFontSize;
        } else {
            lTextSize = MeasureTextEx(lFont, rLabel.c_str(), (float)lFontSize, 1.0f);
        }

        // Adjust position based on angle for better placement
        // Top: center horizontally, above
        // Bottom: center horizontally, below
        // Left: right-align
        // Right: left-align
        float lCosA = cosf(lAngle);
        float lSinA = sinf(lAngle);

        // Horizontal adjustment
        if (lCosA < -0.3f) {
            // Left side - right align
            lPos.x -= lTextSize.x;
        } else if (lCosA > 0.3f) {
            // Right side - left align (no adjustment needed)
        } else {
            // Center horizontally
            lPos.x -= lTextSize.x * 0.5f;
        }

        // Vertical adjustment
        if (lSinA < -0.3f) {
            // Top - place above
            lPos.y -= lTextSize.y;
        } else if (lSinA > 0.3f) {
            // Bottom - place below (small offset)
            lPos.y += 2.0f;
        } else {
            // Center vertically
            lPos.y -= lTextSize.y * 0.5f;
        }

        if (lUseDefaultFont) {
            DrawText(rLabel.c_str(), (int)lPos.x, (int)lPos.y, lFontSize, lColor);
        } else {
            DrawTextEx(lFont, rLabel.c_str(), lPos, (float)lFontSize, 1.0f, lColor);
        }
    }
}

void RLRadarChart::drawSeries(const SeriesDyn& rSeries) const {
    computeSeriesPoints(rSeries);

    size_t lAxisCount = mAxes.size();
    if (lAxisCount < 3 || rSeries.mCachedPoints.size() < 3) return;

    float lAlpha = rSeries.mVisibility;

    // Apply visibility to colors
    Color lLineColor = rSeries.mLineColor;
    lLineColor.a = (unsigned char)(lLineColor.a * lAlpha);

    Color lFillColor = rSeries.mFillColor;
    lFillColor.a = (unsigned char)(lFillColor.a * lAlpha);

    // Draw filled polygon
    if (rSeries.mShowFill && lFillColor.a > 0) {
        // Draw as triangle fan from center
        // Note: DrawTriangle requires counter-clockwise vertex order
        for (size_t i = 0; i < lAxisCount; ++i) {
            size_t lNext = (i + 1) % lAxisCount;
            DrawTriangle(
                mCenter,
                rSeries.mCachedPoints[lNext],
                rSeries.mCachedPoints[i],
                lFillColor
            );
        }
    }

    // Draw outline
    float lThickness = rSeries.mLineThickness;
    for (size_t i = 0; i < lAxisCount; ++i) {
        size_t lNext = (i + 1) % lAxisCount;
        DrawLineEx(
            rSeries.mCachedPoints[i],
            rSeries.mCachedPoints[lNext],
            lThickness,
            lLineColor
        );
    }

    // Draw markers
    if (rSeries.mShowMarkers) {
        float lMarkerRadius = lThickness * rSeries.mMarkerScale;
        for (size_t i = 0; i < lAxisCount; ++i) {
            DrawCircleV(rSeries.mCachedPoints[i], lMarkerRadius, lLineColor);
        }
    }
}

void RLRadarChart::drawLegend() const {
    if (!mStyle.mShowLegend) return;
    if (mSeries.empty()) return;

    Font lFont = mStyle.mLabelFont;
    int lFontSize = mStyle.mLabelFontSize;
    float lPadding = mStyle.mLegendPadding;
    bool lUseDefaultFont = (lFont.texture.id == 0);

    // Position legend at bottom-right
    float lX = mBounds.x + mBounds.width - lPadding;
    float lY = mBounds.y + lPadding;

    float lBoxSize = (float)lFontSize;
    float lSpacing = 4.0f;
    float lLineHeight = lBoxSize + lSpacing;

    for (const auto& rSeries : mSeries) {
        if (rSeries.mVisibility < 0.01f) continue;
        if (rSeries.mLabel.empty()) continue;

        // Measure text
        Vector2 lTextSize;
        if (lUseDefaultFont) {
            lTextSize.x = (float)MeasureText(rSeries.mLabel.c_str(), lFontSize);
        } else {
            lTextSize = MeasureTextEx(lFont, rSeries.mLabel.c_str(), (float)lFontSize, 1.0f);
        }

        float lEntryWidth = lBoxSize + lSpacing + lTextSize.x;
        float lEntryX = lX - lEntryWidth;

        // Draw color box
        Color lBoxColor = rSeries.mLineColor;
        lBoxColor.a = (unsigned char)(lBoxColor.a * rSeries.mVisibility);
        DrawRectangle((int)lEntryX, (int)lY, (int)lBoxSize, (int)lBoxSize, lBoxColor);

        // Draw label
        Color lTextColor = mStyle.mLabelColor;
        lTextColor.a = (unsigned char)(lTextColor.a * rSeries.mVisibility);
        Vector2 lTextPos = {lEntryX + lBoxSize + lSpacing, lY};

        if (lUseDefaultFont) {
            DrawText(rSeries.mLabel.c_str(), (int)lTextPos.x, (int)lTextPos.y, lFontSize, lTextColor);
        } else {
            DrawTextEx(lFont, rSeries.mLabel.c_str(), lTextPos, (float)lFontSize, 1.0f, lTextColor);
        }

        lY += lLineHeight;
    }
}

