// RLLinearGauge.cpp
#include "RLLinearGauge.h"
#include "RLCommon.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

// Constants
constexpr float HALF = 0.5f;
constexpr int VALUE_BUFFER_SIZE = 64;
constexpr float MIN_TRACK_LENGTH = 20.0f;

RLLinearGauge::RLLinearGauge(Rectangle aBounds, float aMinValue, float aMaxValue,
                             RLLinearGaugeOrientation aOrientation,
                             const RLLinearGaugeStyle &aStyle)
    : mBounds(aBounds)
    , mMinValue(aMinValue)
    , mMaxValue(aMaxValue)
    , mValue(aMinValue)
    , mTargetValue(aMinValue)
    , mOrientation(aOrientation)
    , mStyle(aStyle)
{
    recomputeGeometry();
}

void RLLinearGauge::setValue(float aValue) {
    mValue = clampValue(aValue);
    mTargetValue = mValue;
}

void RLLinearGauge::setTargetValue(float aValue) {
    mTargetValue = clampValue(aValue);
}

void RLLinearGauge::setRange(float aMinValue, float aMaxValue) {
    mMinValue = aMinValue;
    mMaxValue = (aMaxValue == aMinValue) ? (aMinValue + 1.0f) : aMaxValue;
    mValue = clampValue(mValue);
    mTargetValue = clampValue(mTargetValue);
    recomputeGeometry();
}

void RLLinearGauge::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
    recomputeGeometry();
}

void RLLinearGauge::setOrientation(RLLinearGaugeOrientation aOrientation) {
    mOrientation = aOrientation;
    recomputeGeometry();
}

void RLLinearGauge::setStyle(const RLLinearGaugeStyle &aStyle) {
    mStyle = aStyle;
    recomputeGeometry();
}

void RLLinearGauge::setPointerStyle(RLLinearGaugePointerStyle aStyle) {
    mPointerStyle = aStyle;
}

void RLLinearGauge::setAnimationEnabled(bool aEnabled) {
    mStyle.mSmoothAnimate = aEnabled;
}

void RLLinearGauge::setTicks(int aMajorCount, int aMinorPerMajor) {
    mStyle.mMajorTickCount = std::max(0, aMajorCount);
    mStyle.mMinorTicksPerMajor = std::max(0, aMinorPerMajor);
    recomputeGeometry();
}

void RLLinearGauge::setLabel(const std::string &aTitle) {
    mTitle = aTitle;
    recomputeGeometry();
}

void RLLinearGauge::setUnit(const std::string &aUnit) {
    mUnit = aUnit;
}

void RLLinearGauge::setRanges(const std::vector<RLLinearGaugeRangeBand> &aRanges) {
    mRangeBands = aRanges;
}

void RLLinearGauge::clearRanges() {
    mRangeBands.clear();
}

void RLLinearGauge::setTargetMarker(float aValue) {
    mTargetMarkerValue = clampValue(aValue);
    mShowTargetMarker = true;
    mStyle.mShowTargetMarker = true;
}

void RLLinearGauge::hideTargetMarker() {
    mShowTargetMarker = false;
    mStyle.mShowTargetMarker = false;
}

float RLLinearGauge::clampValue(float aValue) const {
    return std::max(mMinValue, std::min(mMaxValue, aValue));
}

float RLLinearGauge::valueToPosition(float aValue) const {
    float lNorm = (aValue - mMinValue) / (mMaxValue - mMinValue);
    lNorm = std::max(0.0f, std::min(1.0f, lNorm));

    if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
        return mTrackRect.x + (lNorm * mTrackRect.width);
    } else {
        // Vertical: bottom is min, top is max
        return mTrackRect.y + mTrackRect.height - (lNorm * mTrackRect.height);
    }
}

void RLLinearGauge::recomputeGeometry() {
    // Calculate track rectangle based on orientation and padding
    const float lPadding = mStyle.mPadding;
    const float lTickSpace = mStyle.mShowTicks ? (mStyle.mMajorTickLength + mStyle.mTickLabelGap + mStyle.mLabelFontSize) : 0.0f;
    const float lTitleSpace = mStyle.mShowTitle && !mTitle.empty() ? (mStyle.mTitleFontSize + lPadding) : 0.0f;
    const float lValueSpace = mStyle.mShowValueText ? (mStyle.mValueFontSize + lPadding) : 0.0f;

    if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
        const float lTrackY = mBounds.y + lPadding + lTitleSpace;
        const float lTrackHeight = mStyle.mTrackThickness;
        const float lTrackWidth = mBounds.width - (2.0f * lPadding);
        const float lTrackX = mBounds.x + lPadding;

        mTrackRect = {lTrackX, lTrackY, std::max(MIN_TRACK_LENGTH, lTrackWidth), lTrackHeight};
    } else {
        const float lTrackX = mBounds.x + lPadding + lTickSpace;
        const float lTrackWidth = mStyle.mTrackThickness;
        const float lTrackHeight = mBounds.height - (2.0f * lPadding) - lTitleSpace - lValueSpace;
        const float lTrackY = mBounds.y + lPadding + lTitleSpace;

        mTrackRect = {lTrackX, lTrackY, lTrackWidth, std::max(MIN_TRACK_LENGTH, lTrackHeight)};
    }

    // Precompute tick positions
    mTicks.clear();
    if (!mStyle.mShowTicks || mStyle.mMajorTickCount <= 0) {
        return;
    }

    const int lTotalMajorTicks = mStyle.mMajorTickCount + 1; // Include endpoints
    const int lMinorPerMajor = mStyle.mMinorTicksPerMajor;
    const int lTotalTicks = lTotalMajorTicks + (mStyle.mMajorTickCount * lMinorPerMajor);

    mTicks.reserve((size_t)lTotalTicks);

    const float lRange = mMaxValue - mMinValue;
    const float lMajorStep = lRange / (float)mStyle.mMajorTickCount;
    const float lMinorStep = lMajorStep / (float)(lMinorPerMajor + 1);

    for (int lMajorIdx = 0; lMajorIdx <= mStyle.mMajorTickCount; ++lMajorIdx) {
        const float lMajorValue = mMinValue + (lMajorStep * (float)lMajorIdx);

        // Add major tick
        TickGeom lMajorTick;
        lMajorTick.mValue = lMajorValue;
        lMajorTick.mMajor = true;

        const float lPos = valueToPosition(lMajorValue);
        if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
            lMajorTick.mP0 = {lPos, mTrackRect.y + mTrackRect.height};
            lMajorTick.mP1 = {lPos, mTrackRect.y + mTrackRect.height + mStyle.mMajorTickLength};
        } else {
            lMajorTick.mP0 = {mTrackRect.x - mStyle.mMajorTickLength, lPos};
            lMajorTick.mP1 = {mTrackRect.x, lPos};
        }
        mTicks.push_back(lMajorTick);

        // Add minor ticks between this major and the next (except for last major)
        if (lMajorIdx < mStyle.mMajorTickCount) {
            for (int lMinorIdx = 1; lMinorIdx <= lMinorPerMajor; ++lMinorIdx) {
                const float lMinorValue = lMajorValue + (lMinorStep * (float)lMinorIdx);

                TickGeom lMinorTick;
                lMinorTick.mValue = lMinorValue;
                lMinorTick.mMajor = false;

                const float lMinorPos = valueToPosition(lMinorValue);
                if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
                    lMinorTick.mP0 = {lMinorPos, mTrackRect.y + mTrackRect.height};
                    lMinorTick.mP1 = {lMinorPos, mTrackRect.y + mTrackRect.height + mStyle.mMinorTickLength};
                } else {
                    lMinorTick.mP0 = {mTrackRect.x - mStyle.mMinorTickLength, lMinorPos};
                    lMinorTick.mP1 = {mTrackRect.x, lMinorPos};
                }
                mTicks.push_back(lMinorTick);
            }
        }
    }
}

void RLLinearGauge::update(float aDt) {
    if (!mStyle.mSmoothAnimate) {
        mValue = mTargetValue;
        return;
    }

    // Exponential smoothing
    const float lLambda = mStyle.mAnimateSpeed;
    const float lAlpha = 1.0f - expf(-lLambda * std::max(0.0f, aDt));
    mValue = mValue + ((mTargetValue - mValue) * lAlpha);
}

void RLLinearGauge::draw() const {
    drawBackground();
    drawRangeBands();
    drawTrack();
    drawFill();
    drawTicks();
    drawTargetMarker();
    drawPointer();
    drawLabels();
    drawTitle();
    drawValueText();
}

void RLLinearGauge::drawBackground() const {
    if (mStyle.mShowBackground && mStyle.mBackgroundColor.a > 0) {
        DrawRectangleRounded(mBounds, 0.1f, 8, mStyle.mBackgroundColor);
    }
}

void RLLinearGauge::drawRangeBands() const {
    if (!mStyle.mShowRangeBands || mRangeBands.empty()) {
        return;
    }

    for (const auto &lBand : mRangeBands) {
        const float lBandMin = clampValue(lBand.mMin);
        const float lBandMax = clampValue(lBand.mMax);

        if (lBandMin >= lBandMax) {
            continue;
        }

        const float lPosMin = valueToPosition(lBandMin);
        const float lPosMax = valueToPosition(lBandMax);

        Rectangle lBandRect;
        if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
            lBandRect = {lPosMin, mTrackRect.y, lPosMax - lPosMin, mTrackRect.height};
        } else {
            // Vertical: lPosMax is higher (smaller y), lPosMin is lower (larger y)
            lBandRect = {mTrackRect.x, lPosMax, mTrackRect.width, lPosMin - lPosMax};
        }

        Color lBandColor = lBand.mColor;
        lBandColor.a = 180; // Semi-transparent for overlay effect
        DrawRectangleRec(lBandRect, lBandColor);
    }
}

void RLLinearGauge::drawTrack() const {
    // Draw track border
    if (mStyle.mTrackBorderThickness > 0.0f) {
        const Rectangle lBorderRect = {
            mTrackRect.x - mStyle.mTrackBorderThickness,
            mTrackRect.y - mStyle.mTrackBorderThickness,
            mTrackRect.width + (2.0f * mStyle.mTrackBorderThickness),
            mTrackRect.height + (2.0f * mStyle.mTrackBorderThickness)
        };
        DrawRectangleRounded(lBorderRect, mStyle.mCornerRadius / (mStyle.mTrackThickness + 2.0f * mStyle.mTrackBorderThickness), 4, mStyle.mTrackBorderColor);
    }

    // Draw track background (only where no range bands)
    if (mRangeBands.empty()) {
        DrawRectangleRounded(mTrackRect, mStyle.mCornerRadius / mStyle.mTrackThickness, 4, mStyle.mTrackColor);
    }
}

void RLLinearGauge::drawFill() const {
    if (mPointerStyle != RLLinearGaugePointerStyle::FILL_BAR) {
        return;
    }

    const float lValuePos = valueToPosition(mValue);

    Rectangle lFillRect;
    if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
        const float lWidth = lValuePos - mTrackRect.x;
        if (lWidth > 0.0f) {
            lFillRect = {mTrackRect.x, mTrackRect.y, lWidth, mTrackRect.height};
            DrawRectangleRounded(lFillRect, mStyle.mCornerRadius / mStyle.mTrackThickness, 4, mStyle.mFillColor);
        }
    } else {
        const float lHeight = (mTrackRect.y + mTrackRect.height) - lValuePos;
        if (lHeight > 0.0f) {
            lFillRect = {mTrackRect.x, lValuePos, mTrackRect.width, lHeight};
            DrawRectangleRounded(lFillRect, mStyle.mCornerRadius / mStyle.mTrackThickness, 4, mStyle.mFillColor);
        }
    }
}

void RLLinearGauge::drawPointer() const {
    if (mPointerStyle == RLLinearGaugePointerStyle::FILL_BAR) {
        return; // Fill bar drawn separately
    }

    const float lValuePos = valueToPosition(mValue);
    const float lSize = mStyle.mPointerSize;

    if (mPointerStyle == RLLinearGaugePointerStyle::TRIANGLE) {
        Vector2 lTip, lBase1, lBase2;

        if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
            lTip = {lValuePos, mTrackRect.y + mTrackRect.height + lSize};
            lBase1 = {lValuePos - (lSize * HALF), mTrackRect.y + mTrackRect.height};
            lBase2 = {lValuePos + (lSize * HALF), mTrackRect.y + mTrackRect.height};
        } else {
            lTip = {mTrackRect.x - lSize, lValuePos};
            lBase1 = {mTrackRect.x, lValuePos - (lSize * HALF)};
            lBase2 = {mTrackRect.x, lValuePos + (lSize * HALF)};
        }

        DrawTriangle(lBase1, lTip, lBase2, mStyle.mPointerColor);
    } else if (mPointerStyle == RLLinearGaugePointerStyle::LINE_MARKER) {
        Vector2 lP0, lP1;
        const float lThickness = 3.0f;

        if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
            lP0 = {lValuePos, mTrackRect.y - 2.0f};
            lP1 = {lValuePos, mTrackRect.y + mTrackRect.height + lSize};
        } else {
            lP0 = {mTrackRect.x - lSize, lValuePos};
            lP1 = {mTrackRect.x + mTrackRect.width + 2.0f, lValuePos};
        }

        DrawLineEx(lP0, lP1, lThickness, mStyle.mPointerColor);
    }
}

void RLLinearGauge::drawTicks() const {
    if (!mStyle.mShowTicks) {
        return;
    }

    for (const auto &lTick : mTicks) {
        const Color lColor = lTick.mMajor ? mStyle.mMajorTickColor : mStyle.mMinorTickColor;
        const float lThickness = lTick.mMajor ? mStyle.mMajorTickThickness : mStyle.mMinorTickThickness;
        DrawLineEx(lTick.mP0, lTick.mP1, lThickness, lColor);
    }
}

void RLLinearGauge::drawLabels() const {
    if (!mStyle.mShowTickLabels) {
        return;
    }

    const Font &lFont = (mStyle.mLabelFont.baseSize > 0) ? mStyle.mLabelFont : GetFontDefault();
    const float lFontSize = mStyle.mLabelFontSize;

    for (const auto &lTick : mTicks) {
        if (!lTick.mMajor) {
            continue;
        }

        std::array<char, VALUE_BUFFER_SIZE> lBuf{};
        snprintf(lBuf.data(), lBuf.size(), "%.0f", lTick.mValue);

        const Vector2 lTextSize = MeasureTextEx(lFont, lBuf.data(), lFontSize, 0);
        Vector2 lPos;

        if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
            lPos.x = lTick.mP1.x - (lTextSize.x * HALF);
            lPos.y = lTick.mP1.y + mStyle.mTickLabelGap;
        } else {
            lPos.x = lTick.mP0.x - lTextSize.x - mStyle.mTickLabelGap;
            lPos.y = lTick.mP0.y - (lTextSize.y * HALF);
        }

        DrawTextEx(lFont, lBuf.data(), lPos, lFontSize, 0, mStyle.mLabelColor);
    }
}

void RLLinearGauge::drawTargetMarker() const {
    if (!mShowTargetMarker || !mStyle.mShowTargetMarker) {
        return;
    }

    const float lPos = valueToPosition(mTargetMarkerValue);
    const float lLen = mStyle.mTargetMarkerLength;
    const float lThickness = mStyle.mTargetMarkerThickness;

    Vector2 lP0, lP1;
    if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
        lP0 = {lPos, mTrackRect.y - lLen};
        lP1 = {lPos, mTrackRect.y + mTrackRect.height + lLen};
    } else {
        lP0 = {mTrackRect.x - lLen, lPos};
        lP1 = {mTrackRect.x + mTrackRect.width + lLen, lPos};
    }

    DrawLineEx(lP0, lP1, lThickness, mStyle.mTargetMarkerColor);
}

void RLLinearGauge::drawValueText() const {
    if (!mStyle.mShowValueText) {
        return;
    }

    const Font &lFont = (mStyle.mLabelFont.baseSize > 0) ? mStyle.mLabelFont : GetFontDefault();
    const float lFontSize = mStyle.mValueFontSize;

    std::array<char, VALUE_BUFFER_SIZE> lBuf{};
    if (mUnit.empty()) {
        snprintf(lBuf.data(), lBuf.size(), "%.*f", mStyle.mValueDecimals, mValue);
    } else {
        snprintf(lBuf.data(), lBuf.size(), "%.*f %s", mStyle.mValueDecimals, mValue, mUnit.c_str());
    }

    const Vector2 lTextSize = MeasureTextEx(lFont, lBuf.data(), lFontSize, 0);
    Vector2 lPos;

    if (mOrientation == RLLinearGaugeOrientation::HORIZONTAL) {
        // Position value text centered inside the track for horizontal gauges
        lPos.x = mTrackRect.x + (mTrackRect.width * HALF) - (lTextSize.x * HALF);
        lPos.y = mTrackRect.y + (mTrackRect.height * HALF) - (lTextSize.y * HALF);
    } else {
        // Position value text below the track for vertical
        lPos.x = mTrackRect.x + (mTrackRect.width * HALF) - (lTextSize.x * HALF);
        lPos.y = mTrackRect.y + mTrackRect.height + mStyle.mMajorTickLength + mStyle.mTickLabelGap + mStyle.mLabelFontSize + mStyle.mPadding;

        // Clamp within bounds
        lPos.y = std::min(lPos.y, mBounds.y + mBounds.height - lTextSize.y - mStyle.mPadding);
    }

    DrawTextEx(lFont, lBuf.data(), lPos, lFontSize, 0, mStyle.mValueColor);
}

void RLLinearGauge::drawTitle() const {
    if (!mStyle.mShowTitle || mTitle.empty()) {
        return;
    }

    const Font &lFont = (mStyle.mLabelFont.baseSize > 0) ? mStyle.mLabelFont : GetFontDefault();
    const float lFontSize = mStyle.mTitleFontSize;

    const Vector2 lTextSize = MeasureTextEx(lFont, mTitle.c_str(), lFontSize, 0);
    Vector2 lPos;

    // Both horizontal and vertical: center title above the gauge in the reserved title space
    lPos.x = mBounds.x + (mBounds.width * HALF) - (lTextSize.x * HALF);
    lPos.y = mBounds.y + mStyle.mPadding;

    DrawTextEx(lFont, mTitle.c_str(), lPos, lFontSize, 0, mStyle.mTitleColor);
}

