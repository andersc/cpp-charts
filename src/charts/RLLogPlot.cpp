#include "RLLogPlot.h"
#include "RLCommon.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

RLLogPlot::RLLogPlot(Rectangle aBounds)
    : mBounds(aBounds) {
    mLayoutDirty = true;
    mScaleDirty = true;
}

void RLLogPlot::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
    mLayoutDirty = true;
}

void RLLogPlot::setTimeSeriesHeight(float aHeightFraction) {
    mTimeSeriesHeightFraction = RLCharts::clamp01(aHeightFraction);
    mLayoutDirty = true;
}

void RLLogPlot::setLogPlotStyle(const RLLogPlotStyle& rStyle) {
    mLogPlotStyle = rStyle;
    mScaleDirty = true;
}

void RLLogPlot::setTimeSeriesStyle(const RLTimeSeriesStyle& rStyle) {
    mTimeSeriesStyle = rStyle;
}

void RLLogPlot::setWindowSize(size_t aMaxSamples) {
    mMaxWindowSize = aMaxSamples;
    if (mTimeSeries.size() > mMaxWindowSize) {
        // Trim oldest samples
        const size_t lExcess = mTimeSeries.size() - mMaxWindowSize;
        mTimeSeries.erase(mTimeSeries.begin(), mTimeSeries.begin() + (int)lExcess);
    }
}

void RLLogPlot::pushSample(float aValue) {
    mTimeSeries.push_back(aValue);
    if (mTimeSeries.size() > mMaxWindowSize) {
        mTimeSeries.erase(mTimeSeries.begin());
    }
}

void RLLogPlot::pushSamples(const std::vector<float>& rValues) {
    for (const float lVal : rValues) {
        pushSample(lVal);
    }
}

void RLLogPlot::clearTimeSeries() {
    mTimeSeries.clear();
}

void RLLogPlot::clearTraces() {
    mTraces.clear();
    mScaleDirty = true;
}

size_t RLLogPlot::addTrace(const RLLogPlotTrace& rTrace) {
    mTraces.push_back(rTrace);
    mTraces.back().mDirty = true;
    mScaleDirty = true;
    return mTraces.size() - 1;
}

void RLLogPlot::setTrace(size_t aIndex, const RLLogPlotTrace& rTrace) {
    if (aIndex >= mTraces.size()) {
        return;
    }
    mTraces[aIndex] = rTrace;
    mTraces[aIndex].mDirty = true;
    mScaleDirty = true;
}

void RLLogPlot::updateTraceData(size_t aIndex, const std::vector<float>& rXValues,
                                const std::vector<float>& rYValues,
                                const std::vector<RLLogPlotConfidence>* pConfidence) {
    if (aIndex >= mTraces.size()) {
        return;
    }
    auto& lTrace = mTraces[aIndex];
    lTrace.mXValues = rXValues;
    lTrace.mYValues = rYValues;
    if (pConfidence) {
        lTrace.mConfidence = *pConfidence;
    }
    lTrace.mDirty = true;
    mScaleDirty = true;
}

void RLLogPlot::updateLayout() const {
    if (!mLayoutDirty) {
        return;
    }

    const float lTotalHeight = mBounds.height;
    const float lTimeSeriesH = lTotalHeight * mTimeSeriesHeightFraction;
    const float lLogPlotH = lTotalHeight - lTimeSeriesH - mGapBetweenPlots;

    // Time series on bottom
    mTimeSeriesRect = Rectangle{
        mBounds.x,
        mBounds.y + lLogPlotH + mGapBetweenPlots,
        mBounds.width,
        lTimeSeriesH
    };

    // Log plot on top
    mLogPlotRect = Rectangle{
        mBounds.x,
        mBounds.y,
        mBounds.width,
        lLogPlotH
    };

    mLayoutDirty = false;
}

void RLLogPlot::updateLogScale() const {
    if (!mScaleDirty) {
        return;
    }

    if (!mLogPlotStyle.mAutoScaleX) {
        mLogMinX = mLogPlotStyle.mMinLogX;
        mLogMaxX = mLogPlotStyle.mMaxLogX;
    } else {
        // Auto-scale from trace data
        mLogMinX = 0.0f;
        mLogMaxX = 1.0f;
        bool lFirst = true;
        for (const auto& lTrace : mTraces) {
            for (const float lX : lTrace.mXValues) {
                if (lX > 0.0f) {
                    const float lLogX = log10f(lX);
                    if (lFirst) {
                        mLogMinX = mLogMaxX = lLogX;
                        lFirst = false;
                    } else {
                        mLogMinX = std::min(lLogX, mLogMinX);
                        mLogMaxX = std::max(lLogX, mLogMaxX);
                    }
                }
            }
        }
        // Add some padding
        float lRangeX = mLogMaxX - mLogMinX;
        if (lRangeX < 1e-6f) {
            lRangeX = 1.0f;
        }
        mLogMinX -= lRangeX * 0.05f;
        mLogMaxX += lRangeX * 0.05f;
    }

    if (!mLogPlotStyle.mAutoScaleY) {
        mLogMinY = mLogPlotStyle.mMinLogY;
        mLogMaxY = mLogPlotStyle.mMaxLogY;
    } else {
        // Auto-scale from trace data
        mLogMinY = 0.0f;
        mLogMaxY = 1.0f;
        bool lFirst = true;
        for (const auto& lTrace : mTraces) {
            for (const float lY : lTrace.mYValues) {
                if (lY > 0.0f) {
                    const float lLogY = log10f(lY);
                    if (lFirst) {
                        mLogMinY = mLogMaxY = lLogY;
                        lFirst = false;
                    } else {
                        mLogMinY = std::min(lLogY, mLogMinY);
                        mLogMaxY = std::max(lLogY, mLogMaxY);
                    }
                }
            }
            // Also consider confidence intervals
            for (const auto& lConf : lTrace.mConfidence) {
                if (lConf.mEnabled) {
                    if (lConf.mLowerBound > 0.0f) {
                        const float lLogY = log10f(lConf.mLowerBound);
                        if (lFirst) {
                            mLogMinY = mLogMaxY = lLogY;
                            lFirst = false;
                        } else {
                            mLogMinY = std::min(lLogY, mLogMinY);
                            mLogMaxY = std::max(lLogY, mLogMaxY);
                        }
                    }
                    if (lConf.mUpperBound > 0.0f) {
                        const float lLogY = log10f(lConf.mUpperBound);
                        mLogMinY = std::min(lLogY, mLogMinY);
                        mLogMaxY = std::max(lLogY, mLogMaxY);
                    }
                }
            }
        }
        // Add some padding
        float lRangeY = mLogMaxY - mLogMinY;
        if (lRangeY < 1e-6f) {
            lRangeY = 1.0f;
        }
        mLogMinY -= lRangeY * 0.08f;
        mLogMaxY += lRangeY * 0.08f;
    }

    mScaleDirty = false;
}

Vector2 RLLogPlot::mapLogPoint(float aLogX, float aLogY, Rectangle aRect) const {
    const float lNormX = (aLogX - mLogMinX) / (mLogMaxX - mLogMinX);
    const float lNormY = (aLogY - mLogMinY) / (mLogMaxY - mLogMinY);
    return Vector2{
        aRect.x + lNormX * aRect.width,
        aRect.y + aRect.height - lNormY * aRect.height  // Flip Y
    };
}

void RLLogPlot::ensureTraceAnimation(RLLogPlotTrace& rTrace) const {
    const size_t lN = rTrace.mXValues.size();

    if (rTrace.mAnimX.size() != lN) {
        rTrace.mAnimX.resize(lN);
        rTrace.mAnimY.resize(lN);
        rTrace.mAnimConfLower.resize(lN);
        rTrace.mAnimConfUpper.resize(lN);
        rTrace.mVisibility.resize(lN);

        // Initialize to current values
        for (size_t i = 0; i < lN; ++i) {
            rTrace.mAnimX[i] = rTrace.mXValues[i];
            rTrace.mAnimY[i] = rTrace.mYValues[i];
            rTrace.mVisibility[i] = 1.0f;
            if (i < rTrace.mConfidence.size()) {
                rTrace.mAnimConfLower[i] = rTrace.mConfidence[i].mLowerBound;
                rTrace.mAnimConfUpper[i] = rTrace.mConfidence[i].mUpperBound;
            }
        }
    }
}


void RLLogPlot::update(float aDt) {
    if (!mLogPlotStyle.mSmoothAnimate) {
        return;
    }

    const float lSpeed = mLogPlotStyle.mAnimSpeed * aDt;

    for (auto& lTrace : mTraces) {
        ensureTraceAnimation(lTrace);
        const size_t lN = lTrace.mXValues.size();

        // Animate each point
        for (size_t i = 0; i < lN; ++i) {
            if (i < lTrace.mAnimX.size()) {
                lTrace.mAnimX[i] = RLCharts::approach(lTrace.mAnimX[i], lTrace.mXValues[i], lSpeed);
                lTrace.mAnimY[i] = RLCharts::approach(lTrace.mAnimY[i], lTrace.mYValues[i], lSpeed);

                if (i < lTrace.mConfidence.size() && lTrace.mConfidence[i].mEnabled) {
                    lTrace.mAnimConfLower[i] = RLCharts::approach(lTrace.mAnimConfLower[i],
                                                        lTrace.mConfidence[i].mLowerBound, lSpeed);
                    lTrace.mAnimConfUpper[i] = RLCharts::approach(lTrace.mAnimConfUpper[i],
                                                        lTrace.mConfidence[i].mUpperBound, lSpeed);
                }

                // Fade in
                lTrace.mVisibility[i] = RLCharts::approach(lTrace.mVisibility[i], 1.0f, lSpeed);
            }
        }

        lTrace.mDirty = false;
    }
}

Rectangle RLLogPlot::getTimeSeriesBounds() const {
    updateLayout();
    return mTimeSeriesRect;
}

Rectangle RLLogPlot::getLogPlotBounds() const {
    updateLayout();
    return mLogPlotRect;
}

void RLLogPlot::draw() const {
    updateLayout();
    updateLogScale();

    drawLogPlot();
    drawTimeSeries();
}

void RLLogPlot::drawTimeSeries() const {
    if (mTimeSeries.empty()) {
        return;
    }

    const Rectangle lBounds = mTimeSeriesRect;
    const float lPad = mTimeSeriesStyle.mPadding;
    const Rectangle lPlotRect = { lBounds.x + lPad, lBounds.y + lPad,
                           lBounds.width - 2*lPad, lBounds.height - 2*lPad };

    // Background
    if (mTimeSeriesStyle.mShowBackground) {
        DrawRectangleRec(lBounds, mTimeSeriesStyle.mBackground);
    }

    // Find Y range
    float lMinY = 0.0f, lMaxY = 1.0f;
    if (mTimeSeriesStyle.mAutoScaleY && !mTimeSeries.empty()) {
        lMinY = lMaxY = mTimeSeries[0];
        for (const float lV : mTimeSeries) {
            lMinY = std::min(lV, lMinY);
            lMaxY = std::max(lV, lMaxY);
        }
        const float lRange = lMaxY - lMinY;
        if (lRange < 1e-6f) {
            lMinY -= 0.5f;
            lMaxY += 0.5f;
        } else {
            lMinY -= lRange * 0.1f;
            lMaxY += lRange * 0.1f;
        }
    } else {
        lMinY = mTimeSeriesStyle.mMinY;
        lMaxY = mTimeSeriesStyle.mMaxY;
    }

    // Grid
    if (mTimeSeriesStyle.mShowGrid) {
        const int lGridLines = 4;
        for (int i = 0; i <= lGridLines; ++i) {
            const float lY = lPlotRect.y + ((float)i / (float)lGridLines) * lPlotRect.height;
            DrawLineEx(Vector2{lPlotRect.x, lY},
                      Vector2{lPlotRect.x + lPlotRect.width, lY},
                      1.0f, mTimeSeriesStyle.mGridColor);
        }
    }

    // Axes
    DrawLineEx(Vector2{lPlotRect.x, lPlotRect.y},
              Vector2{lPlotRect.x, lPlotRect.y + lPlotRect.height},
              2.0f, mTimeSeriesStyle.mAxesColor);
    DrawLineEx(Vector2{lPlotRect.x, lPlotRect.y + lPlotRect.height},
              Vector2{lPlotRect.x + lPlotRect.width, lPlotRect.y + lPlotRect.height},
              2.0f, mTimeSeriesStyle.mAxesColor);

    // Map points to screen space
    const size_t lN = mTimeSeries.size();
    if (lN < 2) {
        return;
    }

    std::vector<Vector2> lPoints;
    lPoints.reserve(lN);

    for (size_t i = 0; i < lN; ++i) {
        const float lX = lPlotRect.x + ((float)i / (float)(lN - 1)) * lPlotRect.width;
        const float lNormY = (mTimeSeries[i] - lMinY) / (lMaxY - lMinY);
        const float lY = lPlotRect.y + lPlotRect.height - lNormY * lPlotRect.height;
        lPoints.push_back(Vector2{lX, lY});
    }

    // Fill under curve
    if (mTimeSeriesStyle.mFillUnderCurve && lN >= 2) {
        for (size_t i = 0; i < lN - 1; ++i) {
            const Vector2 lP1 = lPoints[i];
            const Vector2 lP2 = lPoints[i + 1];
            const float lBottom = lPlotRect.y + lPlotRect.height;

            DrawTriangle(
                Vector2{lP1.x, lBottom},
                lP1,
                lP2,
                mTimeSeriesStyle.mFillColor
            );
            DrawTriangle(
                Vector2{lP1.x, lBottom},
                lP2,
                Vector2{lP2.x, lBottom},
                mTimeSeriesStyle.mFillColor
            );
        }
    }

    // Draw line
    for (size_t i = 0; i < lN - 1; ++i) {
        DrawLineEx(lPoints[i], lPoints[i + 1],
                  mTimeSeriesStyle.mLineThickness,
                  mTimeSeriesStyle.mLineColor);
    }

    // Title
    if (!mTimeSeriesStyle.mTitle.empty()) {
        const int lTitleSize = (int)mTimeSeriesStyle.mFontSize + 2;
        const Font& lFont = (mTimeSeriesStyle.mFont.baseSize > 0) ? mTimeSeriesStyle.mFont : GetFontDefault();
        DrawTextEx(lFont, mTimeSeriesStyle.mTitle.c_str(),
                Vector2{lBounds.x + 10, lBounds.y + 5},
                (float)lTitleSize, 0,
                mTimeSeriesStyle.mTextColor);
    }

    // Y-axis label
    if (!mTimeSeriesStyle.mYAxisLabel.empty()) {
        const Font& lFont = (mTimeSeriesStyle.mFont.baseSize > 0) ? mTimeSeriesStyle.mFont : GetFontDefault();
        DrawTextEx(lFont, mTimeSeriesStyle.mYAxisLabel.c_str(),
                Vector2{lBounds.x - 5, lPlotRect.y + lPlotRect.height * 0.5f},
                mTimeSeriesStyle.mFontSize, 0,
                mTimeSeriesStyle.mTextColor);
    }
}

void RLLogPlot::drawLogPlot() const {
    const Rectangle lBounds = mLogPlotRect;
    const float lPad = mLogPlotStyle.mPadding;
    const Rectangle lPlotRect = { lBounds.x + lPad, lBounds.y + lPad,
                           lBounds.width - 2*lPad, lBounds.height - 2*lPad };

    // Background
    if (mLogPlotStyle.mShowBackground) {
        DrawRectangleRec(lBounds, mLogPlotStyle.mBackground);
    }

    // Grid and axes
    drawLogGrid(lPlotRect);
    drawLogAxes(lPlotRect);

    // Draw all traces
    for (const auto& lTrace : mTraces) {
        drawLogTrace(lTrace, lPlotRect);
    }

    // Title
    if (!mLogPlotStyle.mTitle.empty()) {
        const int lTitleSize = (int)mLogPlotStyle.mTitleFontSize;
        const Font& lFont = (mLogPlotStyle.mFont.baseSize > 0) ? mLogPlotStyle.mFont : GetFontDefault();
        const Vector2 lTitleSize2 = MeasureTextEx(lFont, mLogPlotStyle.mTitle.c_str(), (float)lTitleSize, 0);
        DrawTextEx(lFont, mLogPlotStyle.mTitle.c_str(),
                Vector2{lBounds.x + lBounds.width * 0.5f - lTitleSize2.x * 0.5f, lBounds.y + 8},
                (float)lTitleSize, 0,
                mLogPlotStyle.mTextColor);
    }
}

void RLLogPlot::drawLogGrid(Rectangle aPlotRect) const {
    if (!mLogPlotStyle.mShowGrid) {
        return;
    }


    // X grid
    const int lStartDecadeX = (int)floorf(mLogMinX);
    const int lEndDecadeX = (int)ceilf(mLogMaxX);
    for (int lDec = lStartDecadeX; lDec <= lEndDecadeX; ++lDec) {
        const auto lLogX = (float)lDec;
        if (lLogX < mLogMinX || lLogX > mLogMaxX) {
            continue;
        }

        const Vector2 lP1 = mapLogPoint(lLogX, mLogMinY, aPlotRect);
        const Vector2 lP2 = mapLogPoint(lLogX, mLogMaxY, aPlotRect);
        DrawLineEx(lP1, lP2, 1.5f, mLogPlotStyle.mGridColor);

        // Minor grid
        if (mLogPlotStyle.mShowMinorGrid) {
            for (int lMinor = 2; lMinor <= 9; ++lMinor) {
                const float lLogXMinor = lLogX + log10f((float)lMinor);
                if (lLogXMinor < mLogMinX || lLogXMinor > mLogMaxX) {
                    continue;
                }

                const Vector2 lPM1 = mapLogPoint(lLogXMinor, mLogMinY, aPlotRect);
                const Vector2 lPM2 = mapLogPoint(lLogXMinor, mLogMaxY, aPlotRect);
                DrawLineEx(lPM1, lPM2, 0.8f, mLogPlotStyle.mMinorGridColor);
            }
        }
    }

    // Y grid
    const int lStartDecadeY = (int)floorf(mLogMinY);
    const int lEndDecadeY = (int)ceilf(mLogMaxY);
    for (int lDec = lStartDecadeY; lDec <= lEndDecadeY; ++lDec) {
        const auto lLogY = (float)lDec;
        if (lLogY < mLogMinY || lLogY > mLogMaxY) {
            continue;
        }

        const Vector2 lP1 = mapLogPoint(mLogMinX, lLogY, aPlotRect);
        const Vector2 lP2 = mapLogPoint(mLogMaxX, lLogY, aPlotRect);
        DrawLineEx(lP1, lP2, 1.5f, mLogPlotStyle.mGridColor);

        // Minor grid
        if (mLogPlotStyle.mShowMinorGrid) {
            for (int lMinor = 2; lMinor <= 9; ++lMinor) {
                const float lLogYMinor = lLogY + log10f((float)lMinor);
                if (lLogYMinor < mLogMinY || lLogYMinor > mLogMaxY) {
                    continue;
                }

                const Vector2 lPM1 = mapLogPoint(mLogMinX, lLogYMinor, aPlotRect);
                const Vector2 lPM2 = mapLogPoint(mLogMaxX, lLogYMinor, aPlotRect);
                DrawLineEx(lPM1, lPM2, 0.8f, mLogPlotStyle.mMinorGridColor);
            }
        }
    }
}

void RLLogPlot::drawLogAxes(Rectangle aPlotRect) const {
    // Draw axes frame
    DrawLineEx(Vector2{aPlotRect.x, aPlotRect.y},
              Vector2{aPlotRect.x, aPlotRect.y + aPlotRect.height},
              2.5f, mLogPlotStyle.mAxesColor);
    DrawLineEx(Vector2{aPlotRect.x, aPlotRect.y + aPlotRect.height},
              Vector2{aPlotRect.x + aPlotRect.width, aPlotRect.y + aPlotRect.height},
              2.5f, mLogPlotStyle.mAxesColor);

    // Tick labels
    const int lFontSize = (int)mLogPlotStyle.mFontSize;
    const Font& lFont = (mLogPlotStyle.mFont.baseSize > 0) ? mLogPlotStyle.mFont : GetFontDefault();

    // X-axis labels
    const int lStartDecadeX = (int)floorf(mLogMinX);
    const int lEndDecadeX = (int)ceilf(mLogMaxX);
    for (int lDec = lStartDecadeX; lDec <= lEndDecadeX; ++lDec) {
        const auto lLogX = (float)lDec;
        if (lLogX < mLogMinX || lLogX > mLogMaxX) {
            continue;
        }

        const Vector2 lPos = mapLogPoint(lLogX, mLogMinY, aPlotRect);
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "10^%d", lDec);
        const Vector2 lTextSize = MeasureTextEx(lFont, lBuf, (float)lFontSize, 0);
        DrawTextEx(lFont, lBuf, Vector2{lPos.x - lTextSize.x * 0.5f, lPos.y + 8},
                (float)lFontSize, 0, mLogPlotStyle.mTextColor);
    }

    // Y-axis labels
    const int lStartDecadeY = (int)floorf(mLogMinY);
    const int lEndDecadeY = (int)ceilf(mLogMaxY);
    for (int lDec = lStartDecadeY; lDec <= lEndDecadeY; ++lDec) {
        const auto lLogY = (float)lDec;
        if (lLogY < mLogMinY || lLogY > mLogMaxY) {
            continue;
        }

        const Vector2 lPos = mapLogPoint(mLogMinX, lLogY, aPlotRect);
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "10^%d", lDec);
        const Vector2 lTextSize = MeasureTextEx(lFont, lBuf, (float)lFontSize, 0);
        DrawTextEx(lFont, lBuf, Vector2{lPos.x - lTextSize.x - 10, lPos.y - (float)lFontSize * 0.5f},
                (float)lFontSize, 0, mLogPlotStyle.mTextColor);
    }

    // Axis labels
    if (!mLogPlotStyle.mXAxisLabel.empty()) {
        const int lLabelSize = lFontSize + 2;
        const Vector2 lTextSize = MeasureTextEx(lFont, mLogPlotStyle.mXAxisLabel.c_str(), (float)lLabelSize, 0);
        DrawTextEx(lFont, mLogPlotStyle.mXAxisLabel.c_str(),
                Vector2{aPlotRect.x + aPlotRect.width * 0.5f - lTextSize.x * 0.5f, aPlotRect.y + aPlotRect.height + 35},
                (float)lLabelSize, 0,
                mLogPlotStyle.mTextColor);
    }

    if (!mLogPlotStyle.mYAxisLabel.empty()) {
        // Vertical text (FIXME: simplified - draw horizontally for now)
        const int lLabelSize = lFontSize + 2;
        DrawTextEx(lFont, mLogPlotStyle.mYAxisLabel.c_str(),
                Vector2{aPlotRect.x - mLogPlotStyle.mPadding + 5, aPlotRect.y + aPlotRect.height * 0.5f},
                (float)lLabelSize, 0,
                mLogPlotStyle.mTextColor);
    }
}

void RLLogPlot::drawLogTrace(const RLLogPlotTrace& rTrace, Rectangle aPlotRect) const {
    if (rTrace.mXValues.empty() || rTrace.mYValues.empty()) {
        return;
    }

    // Ensure animation data is initialized
    const_cast<RLLogPlot*>(this)->ensureTraceAnimation(const_cast<RLLogPlotTrace&>(rTrace));

    const size_t lN = RLCharts::minVal(rTrace.mXValues.size(), rTrace.mYValues.size());
    if (lN == 0) {
        return;
    }

    // Use animated values if available
    const std::vector<float>* pXVals = &rTrace.mXValues;
    const std::vector<float>* pYVals = &rTrace.mYValues;
    if (mLogPlotStyle.mSmoothAnimate && !rTrace.mAnimX.empty()) {
        pXVals = &rTrace.mAnimX;
        pYVals = &rTrace.mAnimY;
    }

    // Map points to screen space
    std::vector<Vector2> lScreenPoints;
    lScreenPoints.reserve(lN);

    for (size_t i = 0; i < lN; ++i) {
        if ((*pXVals)[i] > 0.0f && (*pYVals)[i] > 0.0f) {
            const float lLogX = log10f((*pXVals)[i]);
            const float lLogY = log10f((*pYVals)[i]);
            lScreenPoints.push_back(mapLogPoint(lLogX, lLogY, aPlotRect));
        }
    }

    if (lScreenPoints.empty()) {
        return;
    }

    // Draw confidence intervals first (so they're behind the line)
    if (rTrace.mStyle.mShowConfidenceIntervals) {
        Color lConfColor = rTrace.mStyle.mConfidenceColor;
        if (lConfColor.a == 0) {
            lConfColor = rTrace.mStyle.mLineColor;
        }
        lConfColor.a = (unsigned char)((float)lConfColor.a * rTrace.mStyle.mConfidenceAlpha);

        for (size_t i = 0; i < lN && i < rTrace.mConfidence.size(); ++i) {
            if (!rTrace.mConfidence[i].mEnabled) {
                continue;
            }
            if ((*pXVals)[i] <= 0.0f) {
                continue;
            }

            float lLower = rTrace.mConfidence[i].mLowerBound;
            float lUpper = rTrace.mConfidence[i].mUpperBound;

            // Use animated values
            if (mLogPlotStyle.mSmoothAnimate && i < rTrace.mAnimConfLower.size()) {
                lLower = rTrace.mAnimConfLower[i];
                lUpper = rTrace.mAnimConfUpper[i];
            }

            if (lLower <= 0.0f || lUpper <= 0.0f) {
                continue;
            }

            const float lLogX = log10f((*pXVals)[i]);
            const float lLogLower = log10f(lLower);
            const float lLogUpper = log10f(lUpper);

            const Vector2 lLowerPt = mapLogPoint(lLogX, lLogLower, aPlotRect);
            const Vector2 lUpperPt = mapLogPoint(lLogX, lLogUpper, aPlotRect);

            const float lVis = (i < rTrace.mVisibility.size()) ? rTrace.mVisibility[i] : 1.0f;
            const Color lDrawColor = RLCharts::fadeColor(lConfColor, lVis);

            if (rTrace.mStyle.mConfidenceAsBars) {
                // Error bars
                DrawLineEx(lLowerPt, lUpperPt, 2.0f, lDrawColor);
                const float lCapW = rTrace.mStyle.mConfidenceBarWidth * 0.5f;
                DrawLineEx(Vector2{lLowerPt.x - lCapW, lLowerPt.y},
                          Vector2{lLowerPt.x + lCapW, lLowerPt.y}, 2.0f, lDrawColor);
                DrawLineEx(Vector2{lUpperPt.x - lCapW, lUpperPt.y},
                          Vector2{lUpperPt.x + lCapW, lUpperPt.y}, 2.0f, lDrawColor);
            } else {
                // Shaded band (draw to next point if available)
                if (i + 1 < lN && i + 1 < rTrace.mConfidence.size() &&
                    rTrace.mConfidence[i + 1].mEnabled && (*pXVals)[i + 1] > 0.0f) {

                    float lNextLower = rTrace.mConfidence[i + 1].mLowerBound;
                    float lNextUpper = rTrace.mConfidence[i + 1].mUpperBound;

                    if (mLogPlotStyle.mSmoothAnimate && i + 1 < rTrace.mAnimConfLower.size()) {
                        lNextLower = rTrace.mAnimConfLower[i + 1];
                        lNextUpper = rTrace.mAnimConfUpper[i + 1];
                    }

                    if (lNextLower > 0.0f && lNextUpper > 0.0f) {
                        const float lNextLogX = log10f((*pXVals)[i + 1]);
                        const Vector2 lNextLowerPt = mapLogPoint(lNextLogX, log10f(lNextLower), aPlotRect);
                        const Vector2 lNextUpperPt = mapLogPoint(lNextLogX, log10f(lNextUpper), aPlotRect);

                        // Draw quad as two triangles
                        DrawTriangle(lLowerPt, lUpperPt, lNextUpperPt, lDrawColor);
                        DrawTriangle(lLowerPt, lNextUpperPt, lNextLowerPt, lDrawColor);
                    }
                }
            }
        }
    }

    // Draw connecting lines
    for (size_t i = 0; i < lScreenPoints.size() - 1; ++i) {
        const float lVis = (i < rTrace.mVisibility.size()) ? rTrace.mVisibility[i] : 1.0f;
        const Color lDrawColor = RLCharts::fadeColor(rTrace.mStyle.mLineColor, lVis);
        DrawLineEx(lScreenPoints[i], lScreenPoints[i + 1],
                  rTrace.mStyle.mLineThickness, lDrawColor);
    }

    // Draw points
    if (rTrace.mStyle.mShowPoints) {
        Color lPointColor = rTrace.mStyle.mPointColor;
        if (lPointColor.a == 0) {
            lPointColor = rTrace.mStyle.mLineColor;
        }

        for (size_t i = 0; i < lScreenPoints.size(); ++i) {
            const float lVis = (i < rTrace.mVisibility.size()) ? rTrace.mVisibility[i] : 1.0f;
            const Color lDrawColor = RLCharts::fadeColor(lPointColor, lVis);
            DrawCircleV(lScreenPoints[i], rTrace.mStyle.mPointRadius, lDrawColor);

            // Outline for visibility
            const Color lOutline = Color{20, 22, 28, (unsigned char)(255.0f * lVis)};
            DrawCircleLines((int)lScreenPoints[i].x, (int)lScreenPoints[i].y,
                          rTrace.mStyle.mPointRadius, lOutline);
        }
    }
}

