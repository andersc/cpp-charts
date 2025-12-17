// RLAreaChart.cpp
#include "RLAreaChart.h"
#include "RLCommon.h"
#include <cmath>
#include <algorithm>

RLAreaChart::RLAreaChart(Rectangle aBounds, RLAreaChartMode aMode,
                         const RLAreaChartStyle& rStyle)
    : mBounds(aBounds), mMode(aMode), mStyle(rStyle) {}

void RLAreaChart::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
}

void RLAreaChart::setMode(RLAreaChartMode aMode) {
    mMode = aMode;
    calculateMaxValue();
}

void RLAreaChart::setStyle(const RLAreaChartStyle& rStyle) {
    mStyle = rStyle;
}

void RLAreaChart::setXLabels(const std::vector<std::string>& rLabels) {
    mXLabels = rLabels;
}

void RLAreaChart::calculateMaxValue() {
    if (mMode == RLAreaChartMode::PERCENT) {
        mMaxValueTarget = 100.0f;
        return;
    }

    if (mSeriesData.empty()) {
        mMaxValueTarget = 100.0f;
        return;
    }

    size_t lNumPoints = mSeriesData[0].mValues.size();
    float lMax = 1.0f;

    for (size_t i = 0; i < lNumPoints; ++i) {
        if (mMode == RLAreaChartMode::STACKED) {
            float lSum = 0.0f;
            for (const auto& rS : mSeriesData) {
                if (i < rS.mValues.size()) {
                    lSum += rS.mValues[i];
                }
            }
            lMax = std::max(lSum, lMax);
        } else {
            for (const auto& rS : mSeriesData) {
                if (i < rS.mValues.size()) {
                    lMax = std::max(rS.mValues[i], lMax);
                }
            }
        }
    }

    mMaxValueTarget = lMax * 1.1f;
}

void RLAreaChart::setData(const std::vector<RLAreaSeries>& rSeries) {
    mSeriesData = rSeries;

    bool lIsFirstData = mSeries.empty();
    mSeries.resize(rSeries.size());

    for (size_t i = 0; i < rSeries.size(); ++i) {
        SeriesDyn& rS = mSeries[i];
        rS.mTargets = rSeries[i].mValues;
        rS.mColor = rSeries[i].mColor;
        rS.mLabel = rSeries[i].mLabel;
        rS.mAlpha = rSeries[i].mAlpha;

        // If first time setting data, start from 0 for entry animation
        if (lIsFirstData || rS.mValues.size() != rSeries[i].mValues.size()) {
            rS.mValues.resize(rSeries[i].mValues.size(), 0.0f);
        }
        // Otherwise keep current values and animate to new targets
    }

    calculateMaxValue();

    // If first data, also animate max value from a lower starting point
    if (lIsFirstData) {
        mMaxValue = mMaxValueTarget * 0.1f;
    }
}

void RLAreaChart::setTargetData(const std::vector<RLAreaSeries>& rSeries) {
    mSeriesData = rSeries;

    // Ensure we have the right number of series
    if (mSeries.size() != rSeries.size()) {
        mSeries.resize(rSeries.size());
    }

    for (size_t i = 0; i < rSeries.size(); ++i) {
        SeriesDyn& rS = mSeries[i];

        // Resize if needed (empty or size mismatch), fill new values with 0
        if (rS.mValues.size() != rSeries[i].mValues.size()) {
            rS.mValues.resize(rSeries[i].mValues.size(), 0.0f);
        }

        rS.mTargets = rSeries[i].mValues;
        rS.mColor = rSeries[i].mColor;
        rS.mLabel = rSeries[i].mLabel;
        rS.mAlpha = rSeries[i].mAlpha;
    }

    calculateMaxValue();
}

void RLAreaChart::update(float aDt) {
    if (!mStyle.mSmoothAnimate) {
        mMaxValue = mMaxValueTarget;
        for (auto& rS : mSeries) {
            rS.mValues = rS.mTargets;
        }
        return;
    }

    float lLambda = mStyle.mAnimateSpeed;
    float lAlpha = 1.0f - expf(-lLambda * fmaxf(0.0f, aDt));

    mMaxValue = RLCharts::lerpF(mMaxValue, mMaxValueTarget, lAlpha);

    for (auto& rS : mSeries) {
        for (size_t i = 0; i < rS.mValues.size() && i < rS.mTargets.size(); ++i) {
            rS.mValues[i] = RLCharts::lerpF(rS.mValues[i], rS.mTargets[i], lAlpha);
        }
    }
}

float RLAreaChart::getStackedValue(size_t aSeriesIndex, size_t aPointIndex) const {
    float lSum = 0.0f;
    for (size_t i = 0; i <= aSeriesIndex; ++i) {
        if (aPointIndex < mSeries[i].mValues.size()) {
            lSum += mSeries[i].mValues[aPointIndex];
        }
    }

    if (mMode == RLAreaChartMode::PERCENT) {
        float lTotal = 0.0f;
        for (const auto& rS : mSeries) {
            if (aPointIndex < rS.mValues.size()) {
                lTotal += rS.mValues[aPointIndex];
            }
        }
        return lTotal > 0.0f ? (lSum / lTotal) * 100.0f : 0.0f;
    }

    return lSum;
}

void RLAreaChart::draw() const {
    if (mStyle.mShowBackground) {
        DrawRectangleRec(mBounds, mStyle.mBackground);
    }

    if (mStyle.mShowGrid) {
        drawGrid();
    }

    drawAxes();

    // Draw areas from back to front
    if (mMode == RLAreaChartMode::OVERLAPPED) {
        for (size_t i = 0; i < mSeries.size(); ++i) {
            drawArea(i);
        }
    } else {
        for (int i = (int)mSeries.size() - 1; i >= 0; --i) {
            drawArea((size_t)i);
        }
    }

    if (mStyle.mShowLegend) {
        drawLegend();
    }
}

void RLAreaChart::drawArea(size_t aSeriesIndex) const {
    if (mSeries.empty() || mSeries[aSeriesIndex].mValues.empty() || mMaxValue <= 0.0f) {
        return;
    }

    const SeriesDyn& rS = mSeries[aSeriesIndex];
    float lChartWidth = mBounds.width - mStyle.mPadding * 2.0f;
    float lChartHeight = mBounds.height - mStyle.mPadding * 2.0f - 20.0f;
    float lBaseY = mBounds.y + mBounds.height - mStyle.mPadding;

    size_t lNumPoints = rS.mValues.size();
    if (lNumPoints < 2) {
        return;
    }

    float lPointSpacing = lChartWidth / (float)(lNumPoints - 1);

    Color lFillColor = rS.mColor;
    lFillColor.a = (unsigned char)(rS.mAlpha * 255.0f);

    // Build triangle strip for efficient rendering
    // Format: top0, bottom0, top1, bottom1, top2, bottom2, ...
    std::vector<Vector2> lStripPoints;
    lStripPoints.reserve(lNumPoints * 2);

    std::vector<Vector2> lTopPoints;
    lTopPoints.reserve(lNumPoints);

    for (size_t i = 0; i < lNumPoints; ++i) {
        float lX = mBounds.x + mStyle.mPadding + (float)i * lPointSpacing;
        float lValue;
        float lBottomValue = 0.0f;

        if (mMode == RLAreaChartMode::OVERLAPPED) {
            lValue = rS.mValues[i];
        } else {
            lValue = getStackedValue(aSeriesIndex, i);
            if (aSeriesIndex > 0) {
                lBottomValue = getStackedValue(aSeriesIndex - 1, i);
            }
        }

        float lY = lBaseY - (lValue / mMaxValue) * lChartHeight;
        float lBottomY = lBaseY - (lBottomValue / mMaxValue) * lChartHeight;

        lTopPoints.push_back({lX, lY});
        lStripPoints.push_back({lX, lY});
        lStripPoints.push_back({lX, lBottomY});
    }

    // Draw filled area using triangle strip (much faster than individual triangles)
    DrawTriangleStrip(lStripPoints.data(), (int)lStripPoints.size(), lFillColor);

    // Draw top line
    for (size_t i = 0; i < lNumPoints - 1; ++i) {
        DrawLineEx(lTopPoints[i], lTopPoints[i + 1], mStyle.mLineThickness, rS.mColor);
    }

    // Draw points
    if (mStyle.mShowPoints) {
        for (const auto& rP : lTopPoints) {
            DrawCircleV(rP, mStyle.mPointRadius, rS.mColor);
        }
    }
}

void RLAreaChart::drawAxes() const {
    float lChartHeight = mBounds.height - mStyle.mPadding * 2.0f - 20.0f;
    float lBaseY = mBounds.y + mBounds.height - mStyle.mPadding;

    // Y-axis
    DrawLine((int)(mBounds.x + mStyle.mPadding - 5.0f), (int)(mBounds.y + mStyle.mPadding),
             (int)(mBounds.x + mStyle.mPadding - 5.0f), (int)lBaseY, mStyle.mAxisColor);

    // X-axis
    DrawLine((int)(mBounds.x + mStyle.mPadding - 5.0f), (int)lBaseY,
             (int)(mBounds.x + mBounds.width - mStyle.mPadding), (int)lBaseY, mStyle.mAxisColor);

    // Y-axis labels
    for (int i = 0; i <= mStyle.mGridLines; ++i) {
        float lValue = (mMaxValue / (float)mStyle.mGridLines) * (float)i;
        float lY = lBaseY - (lChartHeight / (float)mStyle.mGridLines) * (float)i;

        char lBuf[32];
        if (mMode == RLAreaChartMode::PERCENT) {
            snprintf(lBuf, sizeof(lBuf), "%.0f%%", lValue);
        } else {
            snprintf(lBuf, sizeof(lBuf), "%.0f", lValue);
        }

        int lTextWidth = MeasureText(lBuf, mStyle.mLabelFontSize - 2);
        DrawText(lBuf, (int)(mBounds.x + mStyle.mPadding - 10.0f - (float)lTextWidth),
                 (int)(lY - (float)(mStyle.mLabelFontSize - 2) / 2.0f),
                 mStyle.mLabelFontSize - 2, mStyle.mLabelColor);
    }

    // X-axis labels
    if (!mXLabels.empty() && !mSeries.empty() && !mSeries[0].mValues.empty()) {
        float lChartWidth = mBounds.width - mStyle.mPadding * 2.0f;
        size_t lNumPoints = mSeries[0].mValues.size();
        float lPointSpacing = lChartWidth / (float)(lNumPoints - 1);

        size_t lLabelStep = mXLabels.size() > 10 ? mXLabels.size() / 10 : 1;
        for (size_t i = 0; i < mXLabels.size() && i < lNumPoints; i += lLabelStep) {
            float lX = mBounds.x + mStyle.mPadding + (float)i * lPointSpacing;
            int lTextWidth = MeasureText(mXLabels[i].c_str(), mStyle.mLabelFontSize - 2);
            DrawText(mXLabels[i].c_str(), (int)(lX - (float)lTextWidth / 2.0f),
                     (int)(lBaseY + 5.0f), mStyle.mLabelFontSize - 2, mStyle.mLabelColor);
        }
    }
}

void RLAreaChart::drawGrid() const {
    float lChartHeight = mBounds.height - mStyle.mPadding * 2.0f - 20.0f;
    float lBaseY = mBounds.y + mBounds.height - mStyle.mPadding;

    for (int i = 1; i <= mStyle.mGridLines; ++i) {
        float lY = lBaseY - (lChartHeight / (float)mStyle.mGridLines) * (float)i;
        DrawLine((int)(mBounds.x + mStyle.mPadding), (int)lY,
                 (int)(mBounds.x + mBounds.width - mStyle.mPadding), (int)lY, mStyle.mGridColor);
    }
}

void RLAreaChart::drawLegend() const {
    float lLegendX = mBounds.x + mBounds.width - mStyle.mPadding - 100.0f;
    float lLegendY = mBounds.y + mStyle.mPadding;

    for (size_t i = 0; i < mSeries.size(); ++i) {
        float lY = lLegendY + (float)i * 18.0f;
        DrawRectangle((int)lLegendX, (int)lY, 12, 12, mSeries[i].mColor);
        DrawText(mSeries[i].mLabel.c_str(), (int)(lLegendX + 18.0f), (int)lY,
                 mStyle.mLabelFontSize - 2, mStyle.mLabelColor);
    }
}

