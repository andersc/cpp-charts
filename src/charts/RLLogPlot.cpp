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

void RLLogPlot::setLogPlotStyle(const RLLogPlotStyle& aStyle) {
    mLogPlotStyle = aStyle;
    mScaleDirty = true;
}

void RLLogPlot::setTimeSeriesStyle(const RLTimeSeriesStyle& aStyle) {
    mTimeSeriesStyle = aStyle;
}

void RLLogPlot::setWindowSize(size_t aMaxSamples) {
    mMaxWindowSize = aMaxSamples;
    if (mTimeSeries.size() > mMaxWindowSize) {
        // Trim oldest samples
        size_t lExcess = mTimeSeries.size() - mMaxWindowSize;
        mTimeSeries.erase(mTimeSeries.begin(), mTimeSeries.begin() + (int)lExcess);
    }
}

void RLLogPlot::pushSample(float aValue) {
    mTimeSeries.push_back(aValue);
    if (mTimeSeries.size() > mMaxWindowSize) {
        mTimeSeries.erase(mTimeSeries.begin());
    }
}

void RLLogPlot::pushSamples(const std::vector<float>& aValues) {
    for (float lVal : aValues) {
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

size_t RLLogPlot::addTrace(const RLLogPlotTrace& aTrace) {
    mTraces.push_back(aTrace);
    mTraces.back().mDirty = true;
    mScaleDirty = true;
    return mTraces.size() - 1;
}

void RLLogPlot::setTrace(size_t aIndex, const RLLogPlotTrace& aTrace) {
    if (aIndex >= mTraces.size()) return;
    mTraces[aIndex] = aTrace;
    mTraces[aIndex].mDirty = true;
    mScaleDirty = true;
}

void RLLogPlot::updateTraceData(size_t aIndex, const std::vector<float>& aXValues,
                                const std::vector<float>& aYValues,
                                const std::vector<RLLogPlotConfidence>* pConfidence) {
    if (aIndex >= mTraces.size()) return;
    auto& lTrace = mTraces[aIndex];
    lTrace.mXValues = aXValues;
    lTrace.mYValues = aYValues;
    if (pConfidence) {
        lTrace.mConfidence = *pConfidence;
    }
    lTrace.mDirty = true;
    mScaleDirty = true;
}

void RLLogPlot::updateLayout() const {
    if (!mLayoutDirty) return;

    float lTotalHeight = mBounds.height;
    float lTimeSeriesH = lTotalHeight * mTimeSeriesHeightFraction;
    float lLogPlotH = lTotalHeight - lTimeSeriesH - mGapBetweenPlots;

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
    if (!mScaleDirty) return;

    if (!mLogPlotStyle.mAutoScaleX) {
        mLogMinX = mLogPlotStyle.mMinLogX;
        mLogMaxX = mLogPlotStyle.mMaxLogX;
    } else {
        // Auto-scale from trace data
        mLogMinX = 0.0f;
        mLogMaxX = 1.0f;
        bool lFirst = true;
        for (const auto& lTrace : mTraces) {
            for (float lX : lTrace.mXValues) {
                if (lX > 0.0f) {
                    float lLogX = log10f(lX);
                    if (lFirst) {
                        mLogMinX = mLogMaxX = lLogX;
                        lFirst = false;
                    } else {
                        if (lLogX < mLogMinX) mLogMinX = lLogX;
                        if (lLogX > mLogMaxX) mLogMaxX = lLogX;
                    }
                }
            }
        }
        // Add some padding
        float lRangeX = mLogMaxX - mLogMinX;
        if (lRangeX < 1e-6f) lRangeX = 1.0f;
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
            for (float lY : lTrace.mYValues) {
                if (lY > 0.0f) {
                    float lLogY = log10f(lY);
                    if (lFirst) {
                        mLogMinY = mLogMaxY = lLogY;
                        lFirst = false;
                    } else {
                        if (lLogY < mLogMinY) mLogMinY = lLogY;
                        if (lLogY > mLogMaxY) mLogMaxY = lLogY;
                    }
                }
            }
            // Also consider confidence intervals
            for (const auto& lConf : lTrace.mConfidence) {
                if (lConf.mEnabled) {
                    if (lConf.mLowerBound > 0.0f) {
                        float lLogY = log10f(lConf.mLowerBound);
                        if (lFirst) {
                            mLogMinY = mLogMaxY = lLogY;
                            lFirst = false;
                        } else {
                            if (lLogY < mLogMinY) mLogMinY = lLogY;
                            if (lLogY > mLogMaxY) mLogMaxY = lLogY;
                        }
                    }
                    if (lConf.mUpperBound > 0.0f) {
                        float lLogY = log10f(lConf.mUpperBound);
                        if (lLogY < mLogMinY) mLogMinY = lLogY;
                        if (lLogY > mLogMaxY) mLogMaxY = lLogY;
                    }
                }
            }
        }
        // Add some padding
        float lRangeY = mLogMaxY - mLogMinY;
        if (lRangeY < 1e-6f) lRangeY = 1.0f;
        mLogMinY -= lRangeY * 0.08f;
        mLogMaxY += lRangeY * 0.08f;
    }

    mScaleDirty = false;
}

Vector2 RLLogPlot::mapLogPoint(float aLogX, float aLogY, Rectangle aRect) const {
    float lNormX = (aLogX - mLogMinX) / (mLogMaxX - mLogMinX);
    float lNormY = (aLogY - mLogMinY) / (mLogMaxY - mLogMinY);
    return Vector2{
        aRect.x + lNormX * aRect.width,
        aRect.y + aRect.height - lNormY * aRect.height  // Flip Y
    };
}

void RLLogPlot::ensureTraceAnimation(RLLogPlotTrace& rTrace) const {
    size_t lN = rTrace.mXValues.size();

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

float RLLogPlot::approach(float a, float b, float aSpeedDt) {
    float lDiff = b - a;
    if (fabsf(lDiff) < 1e-6f) return b;
    return a + lDiff * clamp01(aSpeedDt);
}

Color RLLogPlot::fadeColor(Color aC, float aAlpha) {
    aC.a = (unsigned char)(aC.a * clamp01(aAlpha));
    return aC;
}

void RLLogPlot::update(float aDt) {
    if (!mLogPlotStyle.mSmoothAnimate) return;

    float lSpeed = mLogPlotStyle.mAnimSpeed * aDt;

    for (auto& lTrace : mTraces) {
        ensureTraceAnimation(lTrace);
        size_t lN = lTrace.mXValues.size();

        // Animate each point
        for (size_t i = 0; i < lN; ++i) {
            if (i < lTrace.mAnimX.size()) {
                lTrace.mAnimX[i] = approach(lTrace.mAnimX[i], lTrace.mXValues[i], lSpeed);
                lTrace.mAnimY[i] = approach(lTrace.mAnimY[i], lTrace.mYValues[i], lSpeed);

                if (i < lTrace.mConfidence.size() && lTrace.mConfidence[i].mEnabled) {
                    lTrace.mAnimConfLower[i] = approach(lTrace.mAnimConfLower[i],
                                                        lTrace.mConfidence[i].mLowerBound, lSpeed);
                    lTrace.mAnimConfUpper[i] = approach(lTrace.mAnimConfUpper[i],
                                                        lTrace.mConfidence[i].mUpperBound, lSpeed);
                }

                // Fade in
                lTrace.mVisibility[i] = approach(lTrace.mVisibility[i], 1.0f, lSpeed);
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
    if (mTimeSeries.empty()) return;

    Rectangle lBounds = mTimeSeriesRect;
    float lPad = mTimeSeriesStyle.mPadding;
    Rectangle lPlotRect = { lBounds.x + lPad, lBounds.y + lPad,
                           lBounds.width - 2*lPad, lBounds.height - 2*lPad };

    // Background
    if (mTimeSeriesStyle.mShowBackground) {
        DrawRectangleRec(lBounds, mTimeSeriesStyle.mBackground);
    }

    // Find Y range
    float lMinY = 0.0f, lMaxY = 1.0f;
    if (mTimeSeriesStyle.mAutoScaleY && !mTimeSeries.empty()) {
        lMinY = lMaxY = mTimeSeries[0];
        for (float lV : mTimeSeries) {
            if (lV < lMinY) lMinY = lV;
            if (lV > lMaxY) lMaxY = lV;
        }
        float lRange = lMaxY - lMinY;
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
        int lGridLines = 4;
        for (int i = 0; i <= lGridLines; ++i) {
            float lY = lPlotRect.y + ((float)i / (float)lGridLines) * lPlotRect.height;
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
    size_t lN = mTimeSeries.size();
    if (lN < 2) return;

    std::vector<Vector2> lPoints;
    lPoints.reserve(lN);

    for (size_t i = 0; i < lN; ++i) {
        float lX = lPlotRect.x + ((float)i / (float)(lN - 1)) * lPlotRect.width;
        float lNormY = (mTimeSeries[i] - lMinY) / (lMaxY - lMinY);
        float lY = lPlotRect.y + lPlotRect.height - lNormY * lPlotRect.height;
        lPoints.push_back(Vector2{lX, lY});
    }

    // Fill under curve
    if (mTimeSeriesStyle.mFillUnderCurve && lN >= 2) {
        for (size_t i = 0; i < lN - 1; ++i) {
            Vector2 lP1 = lPoints[i];
            Vector2 lP2 = lPoints[i + 1];
            float lBottom = lPlotRect.y + lPlotRect.height;

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
        int lTitleSize = (int)mTimeSeriesStyle.mFontSize + 2;
        DrawText(mTimeSeriesStyle.mTitle.c_str(),
                (int)(lBounds.x + 10),
                (int)(lBounds.y + 5),
                lTitleSize,
                mTimeSeriesStyle.mTextColor);
    }

    // Y-axis label
    if (!mTimeSeriesStyle.mYAxisLabel.empty()) {
        DrawText(mTimeSeriesStyle.mYAxisLabel.c_str(),
                (int)(lBounds.x - 5),
                (int)(lPlotRect.y + lPlotRect.height * 0.5f),
                (int)mTimeSeriesStyle.mFontSize,
                mTimeSeriesStyle.mTextColor);
    }
}

void RLLogPlot::drawLogPlot() const {
    Rectangle lBounds = mLogPlotRect;
    float lPad = mLogPlotStyle.mPadding;
    Rectangle lPlotRect = { lBounds.x + lPad, lBounds.y + lPad,
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
        int lTitleSize = (int)mLogPlotStyle.mTitleFontSize;
        int lTitleWidth = MeasureText(mLogPlotStyle.mTitle.c_str(), lTitleSize);
        DrawText(mLogPlotStyle.mTitle.c_str(),
                (int)(lBounds.x + lBounds.width * 0.5f - lTitleWidth * 0.5f),
                (int)(lBounds.y + 8),
                lTitleSize,
                mLogPlotStyle.mTextColor);
    }
}

void RLLogPlot::drawLogGrid(Rectangle aPlotRect) const {
    if (!mLogPlotStyle.mShowGrid) return;

    // Major grid lines - powers of 10
    float lLogRangeX = mLogMaxX - mLogMinX;
    float lLogRangeY = mLogMaxY - mLogMinY;

    // X grid
    int lStartDecadeX = (int)floorf(mLogMinX);
    int lEndDecadeX = (int)ceilf(mLogMaxX);
    for (int lDec = lStartDecadeX; lDec <= lEndDecadeX; ++lDec) {
        float lLogX = (float)lDec;
        if (lLogX < mLogMinX || lLogX > mLogMaxX) continue;

        Vector2 lP1 = mapLogPoint(lLogX, mLogMinY, aPlotRect);
        Vector2 lP2 = mapLogPoint(lLogX, mLogMaxY, aPlotRect);
        DrawLineEx(lP1, lP2, 1.5f, mLogPlotStyle.mGridColor);

        // Minor grid
        if (mLogPlotStyle.mShowMinorGrid) {
            for (int lMinor = 2; lMinor <= 9; ++lMinor) {
                float lLogXMinor = lLogX + log10f((float)lMinor);
                if (lLogXMinor < mLogMinX || lLogXMinor > mLogMaxX) continue;

                Vector2 lPM1 = mapLogPoint(lLogXMinor, mLogMinY, aPlotRect);
                Vector2 lPM2 = mapLogPoint(lLogXMinor, mLogMaxY, aPlotRect);
                DrawLineEx(lPM1, lPM2, 0.8f, mLogPlotStyle.mMinorGridColor);
            }
        }
    }

    // Y grid
    int lStartDecadeY = (int)floorf(mLogMinY);
    int lEndDecadeY = (int)ceilf(mLogMaxY);
    for (int lDec = lStartDecadeY; lDec <= lEndDecadeY; ++lDec) {
        float lLogY = (float)lDec;
        if (lLogY < mLogMinY || lLogY > mLogMaxY) continue;

        Vector2 lP1 = mapLogPoint(mLogMinX, lLogY, aPlotRect);
        Vector2 lP2 = mapLogPoint(mLogMaxX, lLogY, aPlotRect);
        DrawLineEx(lP1, lP2, 1.5f, mLogPlotStyle.mGridColor);

        // Minor grid
        if (mLogPlotStyle.mShowMinorGrid) {
            for (int lMinor = 2; lMinor <= 9; ++lMinor) {
                float lLogYMinor = lLogY + log10f((float)lMinor);
                if (lLogYMinor < mLogMinY || lLogYMinor > mLogMaxY) continue;

                Vector2 lPM1 = mapLogPoint(mLogMinX, lLogYMinor, aPlotRect);
                Vector2 lPM2 = mapLogPoint(mLogMaxX, lLogYMinor, aPlotRect);
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
    int lFontSize = (int)mLogPlotStyle.mFontSize;

    // X-axis labels
    int lStartDecadeX = (int)floorf(mLogMinX);
    int lEndDecadeX = (int)ceilf(mLogMaxX);
    for (int lDec = lStartDecadeX; lDec <= lEndDecadeX; ++lDec) {
        float lLogX = (float)lDec;
        if (lLogX < mLogMinX || lLogX > mLogMaxX) continue;

        Vector2 lPos = mapLogPoint(lLogX, mLogMinY, aPlotRect);
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "10^%d", lDec);
        int lWidth = MeasureText(lBuf, lFontSize);
        DrawText(lBuf, (int)(lPos.x - lWidth * 0.5f), (int)(lPos.y + 8),
                lFontSize, mLogPlotStyle.mTextColor);
    }

    // Y-axis labels
    int lStartDecadeY = (int)floorf(mLogMinY);
    int lEndDecadeY = (int)ceilf(mLogMaxY);
    for (int lDec = lStartDecadeY; lDec <= lEndDecadeY; ++lDec) {
        float lLogY = (float)lDec;
        if (lLogY < mLogMinY || lLogY > mLogMaxY) continue;

        Vector2 lPos = mapLogPoint(mLogMinX, lLogY, aPlotRect);
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "10^%d", lDec);
        int lWidth = MeasureText(lBuf, lFontSize);
        DrawText(lBuf, (int)(lPos.x - lWidth - 10), (int)(lPos.y - lFontSize * 0.5f),
                lFontSize, mLogPlotStyle.mTextColor);
    }

    // Axis labels
    if (!mLogPlotStyle.mXAxisLabel.empty()) {
        int lLabelSize = lFontSize + 2;
        int lWidth = MeasureText(mLogPlotStyle.mXAxisLabel.c_str(), lLabelSize);
        DrawText(mLogPlotStyle.mXAxisLabel.c_str(),
                (int)(aPlotRect.x + aPlotRect.width * 0.5f - lWidth * 0.5f),
                (int)(aPlotRect.y + aPlotRect.height + 35),
                lLabelSize,
                mLogPlotStyle.mTextColor);
    }

    if (!mLogPlotStyle.mYAxisLabel.empty()) {
        // Vertical text (FIXME: simplified - draw horizontally for now)
        int lLabelSize = lFontSize + 2;
        DrawText(mLogPlotStyle.mYAxisLabel.c_str(),
                (int)(aPlotRect.x - mLogPlotStyle.mPadding + 5),
                (int)(aPlotRect.y + aPlotRect.height * 0.5f),
                lLabelSize,
                mLogPlotStyle.mTextColor);
    }
}

void RLLogPlot::drawLogTrace(const RLLogPlotTrace& rTrace, Rectangle aPlotRect) const {
    if (rTrace.mXValues.empty() || rTrace.mYValues.empty()) return;

    // Ensure animation data is initialized
    const_cast<RLLogPlot*>(this)->ensureTraceAnimation(const_cast<RLLogPlotTrace&>(rTrace));

    size_t lN = RLCharts::minVal(rTrace.mXValues.size(), rTrace.mYValues.size());
    if (lN == 0) return;

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
            float lLogX = log10f((*pXVals)[i]);
            float lLogY = log10f((*pYVals)[i]);
            lScreenPoints.push_back(mapLogPoint(lLogX, lLogY, aPlotRect));
        }
    }

    if (lScreenPoints.empty()) return;

    // Draw confidence intervals first (so they're behind the line)
    if (rTrace.mStyle.mShowConfidenceIntervals) {
        Color lConfColor = rTrace.mStyle.mConfidenceColor;
        if (lConfColor.a == 0) {
            lConfColor = rTrace.mStyle.mLineColor;
        }
        lConfColor.a = (unsigned char)(lConfColor.a * rTrace.mStyle.mConfidenceAlpha);

        for (size_t i = 0; i < lN && i < rTrace.mConfidence.size(); ++i) {
            if (!rTrace.mConfidence[i].mEnabled) continue;
            if ((*pXVals)[i] <= 0.0f) continue;

            float lLower = rTrace.mConfidence[i].mLowerBound;
            float lUpper = rTrace.mConfidence[i].mUpperBound;

            // Use animated values
            if (mLogPlotStyle.mSmoothAnimate && i < rTrace.mAnimConfLower.size()) {
                lLower = rTrace.mAnimConfLower[i];
                lUpper = rTrace.mAnimConfUpper[i];
            }

            if (lLower <= 0.0f || lUpper <= 0.0f) continue;

            float lLogX = log10f((*pXVals)[i]);
            float lLogLower = log10f(lLower);
            float lLogUpper = log10f(lUpper);

            Vector2 lCenter = mapLogPoint(lLogX, log10f((*pYVals)[i]), aPlotRect);
            Vector2 lLowerPt = mapLogPoint(lLogX, lLogLower, aPlotRect);
            Vector2 lUpperPt = mapLogPoint(lLogX, lLogUpper, aPlotRect);

            float lVis = (i < rTrace.mVisibility.size()) ? rTrace.mVisibility[i] : 1.0f;
            Color lDrawColor = fadeColor(lConfColor, lVis);

            if (rTrace.mStyle.mConfidenceAsBars) {
                // Error bars
                DrawLineEx(lLowerPt, lUpperPt, 2.0f, lDrawColor);
                float lCapW = rTrace.mStyle.mConfidenceBarWidth * 0.5f;
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
                        float lNextLogX = log10f((*pXVals)[i + 1]);
                        Vector2 lNextLowerPt = mapLogPoint(lNextLogX, log10f(lNextLower), aPlotRect);
                        Vector2 lNextUpperPt = mapLogPoint(lNextLogX, log10f(lNextUpper), aPlotRect);

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
        float lVis = (i < rTrace.mVisibility.size()) ? rTrace.mVisibility[i] : 1.0f;
        Color lDrawColor = fadeColor(rTrace.mStyle.mLineColor, lVis);
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
            float lVis = (i < rTrace.mVisibility.size()) ? rTrace.mVisibility[i] : 1.0f;
            Color lDrawColor = fadeColor(lPointColor, lVis);
            DrawCircleV(lScreenPoints[i], rTrace.mStyle.mPointRadius, lDrawColor);

            // Outline for visibility
            Color lOutline = Color{20, 22, 28, (unsigned char)(255 * lVis)};
            DrawCircleLines((int)lScreenPoints[i].x, (int)lScreenPoints[i].y,
                          rTrace.mStyle.mPointRadius, lOutline);
        }
    }
}

