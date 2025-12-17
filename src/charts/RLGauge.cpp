// RLGauge.cpp
#include "RLGauge.h"
#include "RLCommon.h"
#include <array>
#include <cmath>
#include <cstdio>

// Constants for RLGauge
constexpr float CENTER_DOT_SCALE = 1.2f;
constexpr float FONT_SIZE_SCALE = 0.20f;
constexpr float TEXT_Y_OFFSET = 0.4f;
constexpr float HALF = 0.5f;
constexpr int VALUE_BUFFER_SIZE = 32;


RLGauge::RLGauge(Rectangle bounds, float minValue, float maxValue, const RLGaugeStyle &style)
    : mBounds(bounds), mMinValue(minValue), mMaxValue(maxValue), mValue(minValue), mTargetValue(minValue), mStyle(style)
{
    setBounds(bounds);
}

void RLGauge::setBounds(Rectangle bounds){
    mBounds = bounds;
    mCenter = { mBounds.x + (mBounds.width * HALF), mBounds.y + (mBounds.height * HALF) };
    float lRadius = fminf(mBounds.width, mBounds.height) * HALF;
    mRadius = fmaxf(4.0f, lRadius - 4.0f);
    recomputeGeometry();
}

void RLGauge::setRange(float minValue, float maxValue){
    mMinValue = minValue;
    mMaxValue = (maxValue==minValue)?(minValue+1.0f):maxValue;
    mValue = fminf(mMaxValue, fmaxf(mMinValue, mValue));
    mTargetValue = fminf(mMaxValue, fmaxf(mMinValue, mTargetValue));
}

void RLGauge::setStyle(const RLGaugeStyle &rStyle){
    mStyle = rStyle;
    recomputeGeometry();
}

void RLGauge::setValue(float value){
    mValue = fminf(mMaxValue, fmaxf(mMinValue, value));
    mTargetValue = mValue;
}

void RLGauge::setTargetValue(float value){
    mTargetValue = fminf(mMaxValue, fmaxf(mMinValue, value));
}

float RLGauge::clamp(float aVal) { return aVal < 0 ? 0 : (aVal > 1 ? 1 : aVal); }

float RLGauge::valueToAngle(float aValue) const{
    float lNorm = (aValue - mMinValue) / (mMaxValue - mMinValue);
    lNorm = clamp(lNorm);
    return mStyle.mStartAngle + (lNorm * (mStyle.mEndAngle - mStyle.mStartAngle));
}

void RLGauge::recomputeGeometry(){
    mTicks.clear();
    if (mStyle.mTickCount <= 0) return;
    mTicks.reserve((size_t)mStyle.mTickCount + 1);

    const float lInnerR = mRadius - mStyle.mThickness;
    const float lStartAngle = mStyle.mStartAngle;
    const float lEndAngle = mStyle.mEndAngle;
    const float lStep = (lEndAngle - lStartAngle) / (float)mStyle.mTickCount;
    for (int lIdx = 0; lIdx <= mStyle.mTickCount; lIdx++){
        const float lAngleDeg = lStartAngle + (lStep * (float)lIdx);
        const bool lMajor = (lIdx % mStyle.mMajorEvery) == 0;
        const float lLen = lMajor ? mStyle.mMajorTickLen : mStyle.mTickLen;
        const float lRadiusStart = lInnerR - lLen;
        const float lRadiusEnd = lInnerR - 2.0f; // small gap from ring
        const float lAngleRad = RLCharts::degToRad(lAngleDeg);
        const float lCos = cosf(lAngleRad);
        const float lSin = sinf(lAngleRad);
        Vector2 lP0{ mCenter.x + (lCos * lRadiusStart), mCenter.y + (lSin * lRadiusStart) };
        Vector2 lP1{ mCenter.x + (lCos * lRadiusEnd), mCenter.y + (lSin * lRadiusEnd) };
        mTicks.push_back({lAngleDeg, lP0, lP1, lMajor});
    }
}

void RLGauge::update(float aDeltaTime){
    if (!mStyle.mSmoothAnimate){ mValue = mTargetValue; return; }
    // critically damped like smoothing; simple exponential approach
    constexpr float LAMBDA = 10.0f; // speed factor
    const float lAlpha = 1.0f - expf(-LAMBDA * fmaxf(0.0f, aDeltaTime));
    mValue = mValue + ((mTargetValue - mValue) * lAlpha);
}

void RLGauge::draw() const{
    // background
    if (mStyle.mBackgroundColor.a > 0){
        DrawRectangleRounded(mBounds, 0.15f, 8, mStyle.mBackgroundColor);
    }

    const float lInnerR = mRadius - mStyle.mThickness;
    const float lOuterR = mRadius;

    // base arc
    DrawRing(mCenter, lInnerR, lOuterR, mStyle.mStartAngle, mStyle.mEndAngle, 64, mStyle.mBaseArcColor);

    // value arc
    const float lAngValue = valueToAngle(mValue);
    DrawRing(mCenter, lInnerR, lOuterR, mStyle.mStartAngle, lAngValue, 64, mStyle.mValueArcColor);

    // ticks
    if (mStyle.mShowTicks){
        for (const auto &lTick : mTicks){
            const Color lColor = lTick.mMajor ? mStyle.mMajorTickColor : mStyle.mTickColor;
            const float lThickness = lTick.mMajor ? mStyle.mMajorTickThickness : mStyle.mTickThickness;
            DrawLineEx(lTick.mP0, lTick.mP1, lThickness, lColor);
        }
    }

    // needle
    if (mStyle.mShowNeedle){
        float lAng = valueToAngle(mValue);
        float lAngRad = RLCharts::degToRad(lAng);
        float lNeedleRadius = mRadius * mStyle.mNeedleRadiusScale;
        Vector2 lTip{ mCenter.x + (cosf(lAngRad) * lNeedleRadius), mCenter.y + (sinf(lAngRad) * lNeedleRadius) };
        DrawLineEx(mCenter, lTip, mStyle.mNeedleWidth, mStyle.mNeedleColor);
        DrawCircleV(mCenter, mStyle.mNeedleWidth * CENTER_DOT_SCALE, mStyle.mCenterColor);
    }

    // value text
    if (mStyle.mShowValueText){
        std::array<char, VALUE_BUFFER_SIZE> lBuf{};
        const float lNormValue = (mValue - mMinValue)/(mMaxValue - mMinValue);
        // map to 0-100 for common gauge feel
        snprintf(lBuf.data(), lBuf.size(), "%.0f", lNormValue * 100.0f);
        const Font &lFont = (mStyle.mLabelFont.baseSize>0)? mStyle.mLabelFont : GetFontDefault();
        float lFontSize = (fminf(mBounds.width, mBounds.height) * FONT_SIZE_SCALE);
        const Vector2 lTextSize = MeasureTextEx(lFont, lBuf.data(), lFontSize, 0);
        // Position text at bottom center of gauge (below center, inside the arc)
        const float lTextInnerR = mRadius - mStyle.mThickness;
        const float lTextY = mCenter.y + (lTextInnerR * TEXT_Y_OFFSET);  // Position below center
        // Center text horizontally and vertically
        const Vector2 lPos{ mCenter.x - (lTextSize.x * HALF), lTextY - (lTextSize.y * HALF) };
        DrawTextEx(lFont, lBuf.data(), lPos, lFontSize, 0, mStyle.mLabelColor);
    }
}
