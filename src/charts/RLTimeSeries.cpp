// RLTimeSeries.cpp
#include "RLTimeSeries.h"
#include "RLCommon.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// Constructor
// ============================================================================

RLTimeSeries::RLTimeSeries(Rectangle aBounds, size_t aWindowSize)
    : mBounds(aBounds)
    , mWindowSize(aWindowSize > 0 ? aWindowSize : 500) {
}

// ============================================================================
// Configuration
// ============================================================================

void RLTimeSeries::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
    for (auto& lTrace : mTraces) {
        lTrace.mDirty = true;
    }
}

void RLTimeSeries::setStyle(const RLTimeSeriesChartStyle& aStyle) {
    mStyle = aStyle;
    for (auto& lTrace : mTraces) {
        lTrace.mDirty = true;
    }
}

void RLTimeSeries::setWindowSize(size_t aWindowSize) {
    if (aWindowSize == 0) aWindowSize = 1;
    if (aWindowSize == mWindowSize) return;

    size_t lOldWindowSize = mWindowSize;
    mWindowSize = aWindowSize;

    // Resize all trace buffers
    for (auto& lTrace : mTraces) {
        if (lTrace.mSamples.size() != mWindowSize) {
            // Need to reorganize ring buffer
            std::vector<float> lNewBuffer(mWindowSize, 0.0f);
            size_t lCopyCount = lTrace.mCount < mWindowSize ? lTrace.mCount : mWindowSize;

            // Copy most recent samples
            for (size_t i = 0; i < lCopyCount; ++i) {
                size_t lOldIdx = (lTrace.mHead + lTrace.mCount - lCopyCount + i) % lOldWindowSize;
                lNewBuffer[i] = lTrace.mSamples[lOldIdx];
            }

            lTrace.mSamples = std::move(lNewBuffer);
            lTrace.mHead = lCopyCount % mWindowSize;
            lTrace.mCount = lCopyCount;
            lTrace.mDirty = true;
        }
    }
}

// ============================================================================
// Trace Management
// ============================================================================

size_t RLTimeSeries::addTrace(const RLTimeSeriesTraceStyle& aStyle) {
    RLTimeSeriesTrace lTrace;
    lTrace.mStyle = aStyle;
    lTrace.mSamples.resize(mWindowSize, 0.0f);
    lTrace.mHead = 0;
    lTrace.mCount = 0;
    lTrace.mDirty = true;

    mTraces.push_back(std::move(lTrace));
    return mTraces.size() - 1;
}

void RLTimeSeries::setTraceStyle(size_t aIndex, const RLTimeSeriesTraceStyle& aStyle) {
    if (aIndex >= mTraces.size()) return;
    mTraces[aIndex].mStyle = aStyle;
    mTraces[aIndex].mDirty = true;
}

void RLTimeSeries::setTraceVisible(size_t aIndex, bool aVisible) {
    if (aIndex >= mTraces.size()) return;
    mTraces[aIndex].mStyle.mVisible = aVisible;
}

void RLTimeSeries::clearTrace(size_t aIndex) {
    if (aIndex >= mTraces.size()) return;
    mTraces[aIndex].mHead = 0;
    mTraces[aIndex].mCount = 0;
    mTraces[aIndex].mDirty = true;
}

void RLTimeSeries::clearAllTraces() {
    for (auto& lTrace : mTraces) {
        lTrace.mHead = 0;
        lTrace.mCount = 0;
        lTrace.mDirty = true;
    }
}

size_t RLTimeSeries::getTraceSampleCount(size_t aIndex) const {
    if (aIndex >= mTraces.size()) return 0;
    return mTraces[aIndex].mCount;
}

// ============================================================================
// Sample Input
// ============================================================================

void RLTimeSeries::pushSample(size_t aTraceIndex, float aValue) {
    if (aTraceIndex >= mTraces.size()) return;

    RLTimeSeriesTrace& rTrace = mTraces[aTraceIndex];
    rTrace.mSamples[rTrace.mHead] = aValue;
    rTrace.mHead = (rTrace.mHead + 1) % mWindowSize;
    if (rTrace.mCount < mWindowSize) {
        rTrace.mCount++;
    }
    rTrace.mDirty = true;
}

void RLTimeSeries::pushSamples(size_t aTraceIndex, const float* pValues, size_t aCount) {
    if (aTraceIndex >= mTraces.size() || pValues == nullptr || aCount == 0) return;

    RLTimeSeriesTrace& rTrace = mTraces[aTraceIndex];
    for (size_t i = 0; i < aCount; ++i) {
        rTrace.mSamples[rTrace.mHead] = pValues[i];
        rTrace.mHead = (rTrace.mHead + 1) % mWindowSize;
        if (rTrace.mCount < mWindowSize) {
            rTrace.mCount++;
        }
    }
    rTrace.mDirty = true;
}

// ============================================================================
// Update
// ============================================================================

void RLTimeSeries::update(float aDt) {
    updateScale(aDt);
}

void RLTimeSeries::updateScale(float aDt) {
    if (!mStyle.mAutoScaleY) {
        mTargetMinY = mStyle.mMinY;
        mTargetMaxY = mStyle.mMaxY;
    } else {
        // Compute min/max from all visible traces
        float lDataMin = 0.0f;
        float lDataMax = 0.0f;
        bool lHasData = false;

        for (const auto& lTrace : mTraces) {
            if (!lTrace.mStyle.mVisible || lTrace.mCount == 0) continue;

            for (size_t i = 0; i < lTrace.mCount; ++i) {
                float lVal = getSample(lTrace, i);
                if (!lHasData) {
                    lDataMin = lDataMax = lVal;
                    lHasData = true;
                } else {
                    if (lVal < lDataMin) lDataMin = lVal;
                    if (lVal > lDataMax) lDataMax = lVal;
                }
            }
        }

        if (lHasData) {
            float lRange = lDataMax - lDataMin;
            if (lRange < 0.001f) lRange = 1.0f; // Avoid zero range
            float lMargin = lRange * mStyle.mAutoScaleMargin;
            mTargetMinY = lDataMin - lMargin;
            mTargetMaxY = lDataMax + lMargin;
        }
    }

    // Smooth transition
    if (mStyle.mSmoothScale) {
        float lSpeed = mStyle.mScaleSpeed * aDt;
        mCurrentMinY = approach(mCurrentMinY, mTargetMinY, lSpeed);
        mCurrentMaxY = approach(mCurrentMaxY, mTargetMaxY, lSpeed);
    } else {
        mCurrentMinY = mTargetMinY;
        mCurrentMaxY = mTargetMaxY;
    }

    // Mark traces dirty if scale changed significantly
    float lEps = 0.0001f;
    if (fabsf(mCurrentMinY - mTargetMinY) > lEps || fabsf(mCurrentMaxY - mTargetMaxY) > lEps) {
        for (auto& lTrace : mTraces) {
            lTrace.mDirty = true;
        }
    }
}

// ============================================================================
// Draw
// ============================================================================

void RLTimeSeries::draw() const {
    Rectangle lPlotArea = getPlotArea();

    // Background
    if (mStyle.mShowBackground) {
        DrawRectangleRec(mBounds, mStyle.mBackground);
    }

    // Grid
    if (mStyle.mShowGrid) {
        drawGrid();
    }

    // Axes
    if (mStyle.mShowAxes) {
        drawAxes();
    }

    // Clip to plot area
    BeginScissorMode((int)lPlotArea.x, (int)lPlotArea.y,
                     (int)lPlotArea.width, (int)lPlotArea.height);

    // Draw all visible traces
    for (size_t i = 0; i < mTraces.size(); ++i) {
        const RLTimeSeriesTrace& rTrace = mTraces[i];
        if (!rTrace.mStyle.mVisible || rTrace.mCount < 2) continue;

        if (rTrace.mDirty) {
            rebuildScreenPoints(i);
            rTrace.mDirty = false;
        }

        drawTrace(rTrace);
    }

    EndScissorMode();
}

void RLTimeSeries::drawGrid() const {
    Rectangle lPlotArea = getPlotArea();

    // Vertical grid lines
    for (int i = 0; i <= mStyle.mGridLinesX; ++i) {
        float lX = lPlotArea.x + (lPlotArea.width * (float)i / (float)mStyle.mGridLinesX);
        DrawLineEx({ lX, lPlotArea.y }, { lX, lPlotArea.y + lPlotArea.height },
                   1.0f, mStyle.mGridColor);
    }

    // Horizontal grid lines
    for (int i = 0; i <= mStyle.mGridLinesY; ++i) {
        float lY = lPlotArea.y + (lPlotArea.height * (float)i / (float)mStyle.mGridLinesY);
        DrawLineEx({ lPlotArea.x, lY }, { lPlotArea.x + lPlotArea.width, lY },
                   1.0f, mStyle.mGridColor);
    }
}

void RLTimeSeries::drawAxes() const {
    Rectangle lPlotArea = getPlotArea();

    // Left axis
    DrawLineEx({ lPlotArea.x, lPlotArea.y },
               { lPlotArea.x, lPlotArea.y + lPlotArea.height },
               1.5f, mStyle.mAxesColor);

    // Bottom axis
    DrawLineEx({ lPlotArea.x, lPlotArea.y + lPlotArea.height },
               { lPlotArea.x + lPlotArea.width, lPlotArea.y + lPlotArea.height },
               1.5f, mStyle.mAxesColor);
}

void RLTimeSeries::drawTrace(const RLTimeSeriesTrace& rTrace) const {
    const RLTimeSeriesTraceStyle& rStyle = rTrace.mStyle;

    if (rStyle.mLineMode == RLTimeSeriesLineMode::Spline && rTrace.mSplineCache.size() >= 2) {
        // Draw spline
        for (size_t i = 0; i < rTrace.mSplineCache.size() - 1; ++i) {
            DrawLineEx(rTrace.mSplineCache[i], rTrace.mSplineCache[i + 1],
                       rStyle.mLineThickness, rStyle.mColor);
        }
    } else if (rTrace.mScreenPoints.size() >= 2) {
        // Draw linear segments
        for (size_t i = 0; i < rTrace.mScreenPoints.size() - 1; ++i) {
            DrawLineEx(rTrace.mScreenPoints[i], rTrace.mScreenPoints[i + 1],
                       rStyle.mLineThickness, rStyle.mColor);
        }
    }

    // Draw points if enabled
    if (rStyle.mShowPoints && !rTrace.mScreenPoints.empty()) {
        for (const auto& lPt : rTrace.mScreenPoints) {
            DrawCircleV(lPt, rStyle.mPointRadius, rStyle.mColor);
        }
    }
}

// ============================================================================
// Helpers
// ============================================================================

Rectangle RLTimeSeries::getPlotArea() const {
    float lPad = mStyle.mPadding;
    return {
        mBounds.x + lPad,
        mBounds.y + lPad,
        mBounds.width - 2.0f * lPad,
        mBounds.height - 2.0f * lPad
    };
}

float RLTimeSeries::getSample(const RLTimeSeriesTrace& rTrace, size_t aIndex) {
    // Get sample at logical index (0 = oldest, mCount-1 = newest)
    if (aIndex >= rTrace.mCount) return 0.0f;
    size_t lBufferSize = rTrace.mSamples.size();
    size_t lStart = (rTrace.mHead + lBufferSize - rTrace.mCount) % lBufferSize;
    size_t lActualIdx = (lStart + aIndex) % lBufferSize;
    return rTrace.mSamples[lActualIdx];
}

void RLTimeSeries::rebuildScreenPoints(size_t aTraceIndex) const {
    if (aTraceIndex >= mTraces.size()) return;

    const RLTimeSeriesTrace& rTrace = mTraces[aTraceIndex];
    if (rTrace.mCount < 2) {
        rTrace.mScreenPoints.clear();
        rTrace.mSplineCache.clear();
        return;
    }

    Rectangle lPlotArea = getPlotArea();
    float lYRange = mCurrentMaxY - mCurrentMinY;
    if (lYRange < 0.0001f) lYRange = 1.0f;

    // Build screen points
    rTrace.mScreenPoints.resize(rTrace.mCount);

    // Calculate X spacing based on window size (full width = full window)
    // During build-up phase, data fills from left toward right
    float lXStep = lPlotArea.width / (float)(mWindowSize - 1);

    for (size_t i = 0; i < rTrace.mCount; ++i) {
        float lVal = getSample(rTrace, i);

        // X: position based on sample index within the window
        // Oldest sample at left, newest at right
        float lX = lPlotArea.x + lXStep * (float)i;

        // Y: map value to screen (inverted: higher values = lower Y)
        float lNormY = (lVal - mCurrentMinY) / lYRange;
        float lY = lPlotArea.y + lPlotArea.height * (1.0f - lNormY);

        rTrace.mScreenPoints[i] = { lX, lY };
    }

    // Build spline cache if needed
    if (rTrace.mStyle.mLineMode == RLTimeSeriesLineMode::Spline && rTrace.mCount >= 4) {
        // Estimate total spline points needed
        float lTotalDist = 0.0f;
        for (size_t i = 1; i < rTrace.mScreenPoints.size(); ++i) {
            lTotalDist += RLCharts::distance(rTrace.mScreenPoints[i - 1], rTrace.mScreenPoints[i]);
        }

        size_t lSplinePoints = (size_t)(lTotalDist / mStyle.mSplinePixels) + rTrace.mCount;
        if (lSplinePoints < rTrace.mCount) lSplinePoints = rTrace.mCount;
        if (lSplinePoints > 10000) lSplinePoints = 10000; // Reasonable limit

        rTrace.mSplineCache.resize(lSplinePoints);

        size_t lNumSegments = rTrace.mScreenPoints.size() - 1;
        size_t lPointsPerSegment = lSplinePoints / lNumSegments;
        if (lPointsPerSegment < 2) lPointsPerSegment = 2;

        size_t lOutIdx = 0;
        for (size_t seg = 0; seg < lNumSegments && lOutIdx < lSplinePoints; ++seg) {
            // Get control points for Catmull-Rom
            size_t lI0 = (seg > 0) ? seg - 1 : 0;
            size_t lI1 = seg;
            size_t lI2 = seg + 1;
            size_t lI3 = (seg + 2 < rTrace.mScreenPoints.size()) ? seg + 2 : rTrace.mScreenPoints.size() - 1;

            Vector2 lP0 = rTrace.mScreenPoints[lI0];
            Vector2 lP1 = rTrace.mScreenPoints[lI1];
            Vector2 lP2 = rTrace.mScreenPoints[lI2];
            Vector2 lP3 = rTrace.mScreenPoints[lI3];

            size_t lSteps = (seg == lNumSegments - 1) ? (lSplinePoints - lOutIdx) : lPointsPerSegment;
            for (size_t s = 0; s < lSteps && lOutIdx < lSplinePoints; ++s) {
                float lT = (float)s / (float)lSteps;
                rTrace.mSplineCache[lOutIdx++] = RLCharts::catmullRom(lP0, lP1, lP2, lP3, lT);
            }
        }

        // Ensure last point is included
        if (lOutIdx > 0 && lOutIdx <= lSplinePoints) {
            rTrace.mSplineCache[lOutIdx - 1] = rTrace.mScreenPoints.back();
        }
        rTrace.mSplineCache.resize(lOutIdx);
    } else {
        rTrace.mSplineCache.clear();
    }
}

float RLTimeSeries::approach(float a, float b, float aSpeedDt) {
    float lDiff = b - a;
    if (fabsf(lDiff) < 0.0001f) return b;
    return a + lDiff * clamp01(aSpeedDt);
}

