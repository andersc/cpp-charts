// RLGauge.cpp
#include "RLGauge.h"
#include "RLCommon.h"
#include <cmath>
#include <cstdio>


RLGauge::RLGauge(Rectangle bounds, float minValue, float maxValue, const RLGaugeStyle &style)
    : mBounds(bounds), mMinValue(minValue), mMaxValue(maxValue), mValue(minValue), mTargetValue(minValue), mStyle(style)
{
    setBounds(bounds);
}

void RLGauge::setBounds(Rectangle bounds){
    mBounds = bounds;
    mCenter = { mBounds.x + mBounds.width*0.5f, mBounds.y + mBounds.height*0.5f };
    float r = fminf(mBounds.width, mBounds.height)*0.5f;
    mRadius = fmaxf(4.0f, r - 4.0f);
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

float RLGauge::clamp(float t) const { return t < 0 ? 0 : (t>1?1:t); }

float RLGauge::valueToAngle(float v) const{
    float t = (v - mMinValue) / (mMaxValue - mMinValue);
    t = clamp(t);
    return mStyle.mStartAngle + t * (mStyle.mEndAngle - mStyle.mStartAngle);
}

void RLGauge::recomputeGeometry(){
    mTicks.clear();
    if (mStyle.mTickCount <= 0) return;
    mTicks.reserve((size_t)mStyle.mTickCount + 1);

    float innerR = mRadius - mStyle.mThickness;
    float outerR = mRadius;
    float a0 = mStyle.mStartAngle;
    float a1 = mStyle.mEndAngle;
    float step = (a1 - a0) / (float)mStyle.mTickCount;
    for (int i=0;i<=mStyle.mTickCount;i++){
        float ad = a0 + step * (float)i;
        bool major = (i % mStyle.mMajorEvery) == 0;
        float len = major ? mStyle.mMajorTickLen : mStyle.mTickLen;
        float r0 = innerR - len;
        float r1 = innerR - 2.0f; // small gap from ring
        float ar = RLCharts::degToRad(ad);
        float cs = cosf(ar), sn = sinf(ar);
        Vector2 p0{ mCenter.x + cs*r0, mCenter.y + sn*r0 };
        Vector2 p1{ mCenter.x + cs*r1, mCenter.y + sn*r1 };
        mTicks.push_back({ad, p0, p1, major});
    }
}

void RLGauge::update(float dt){
    if (!mStyle.mSmoothAnimate){ mValue = mTargetValue; return; }
    // critically damped like smoothing; simple exponential approach
    float lambda = 10.0f; // speed factor
    float alpha = 1.0f - expf(-lambda * fmaxf(0.0f, dt));
    mValue = mValue + (mTargetValue - mValue)*alpha;
}

void RLGauge::draw() const{
    // background
    if (mStyle.mBackgroundColor.a > 0){
        DrawRectangleRounded(mBounds, 0.15f, 8, mStyle.mBackgroundColor);
    }

    float innerR = mRadius - mStyle.mThickness;
    float outerR = mRadius;

    // base arc
    DrawRing(mCenter, innerR, outerR, mStyle.mStartAngle, mStyle.mEndAngle, 64, mStyle.mBaseArcColor);

    // value arc
    float angV = valueToAngle(mValue);
    DrawRing(mCenter, innerR, outerR, mStyle.mStartAngle, angV, 64, mStyle.mValueArcColor);

    // ticks
    if (mStyle.mShowTicks){
        for (const auto &t : mTicks){
            Color c = t.mMajor ? mStyle.mMajorTickColor : mStyle.mTickColor;
            float th = t.mMajor ? mStyle.mMajorTickThickness : mStyle.mTickThickness;
            DrawLineEx(t.mP0, t.mP1, th, c);
        }
    }

    // needle
    if (mStyle.mShowNeedle){
        float ang = valueToAngle(mValue);
        float ar = RLCharts::degToRad(ang);
        float r = mRadius * mStyle.mNeedleRadiusScale;
        Vector2 tip{ mCenter.x + cosf(ar)*r, mCenter.y + sinf(ar)*r };
        DrawLineEx(mCenter, tip, mStyle.mNeedleWidth, mStyle.mNeedleColor);
        DrawCircleV(mCenter, mStyle.mNeedleWidth*1.2f, mStyle.mCenterColor);
    }

    // value text
    if (mStyle.mShowValueText){
        char buf[32];
        float t = (mValue - mMinValue)/(mMaxValue - mMinValue);
        // map to 0-100 for common gauge feel
        snprintf(buf, sizeof(buf), "%3.0f", t*100.0f);
        const Font &font = (mStyle.mLabelFont.baseSize>0)? mStyle.mLabelFont : GetFontDefault();
        float fontSize = (fminf(mBounds.width, mBounds.height) * 0.20f);
        Vector2 ts = MeasureTextEx(font, buf, fontSize, 0);
        Vector2 pos{ mCenter.x - ts.x*0.5f, mCenter.y - ts.y*0.5f };
        DrawTextEx(font, buf, pos, fontSize, 0, mStyle.mLabelColor);
    }
}
