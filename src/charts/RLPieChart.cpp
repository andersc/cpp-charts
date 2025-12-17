#include "RLPieChart.h"
#include "RLCommon.h"
#include <algorithm>
#include <cmath>


RLPieChart::RLPieChart(Rectangle aBounds, const RLPieChartStyle &aStyle)
        : mBounds(aBounds), mStyle(aStyle) {
}

void RLPieChart::setBounds(Rectangle aBounds){
    mBounds = aBounds;
    mGeomDirty = true;
}

void RLPieChart::setStyle(const RLPieChartStyle &rStyle){
    mStyle = rStyle;
}

void RLPieChart::setHollowFactor(float aFactor){
    mHollowFactor = std::clamp(aFactor, 0.0f, 1.0f);
}

void RLPieChart::ensureSize(size_t aCount){
    if (mSlices.size() < aCount){
        const size_t lOld = mSlices.size();
        mSlices.resize(aCount);
        for (size_t i=lOld;i<aCount;i++){
            mSlices[i].mValue = 0.0f;
            mSlices[i].mTarget = 0.0f;
            mSlices[i].mStart = mSlices[i].mEnd = 0.0f;
            mSlices[i].mStartTarget = mSlices[i].mEndTarget = 0.0f;
            mSlices[i].mVis = 0.0f;
            mSlices[i].mVisTarget = 0.0f;
        }
    }
}

void RLPieChart::recomputeTargetsFromData(const std::vector<RLPieSliceData> &rData){
    // Determine target count and ensure vector size to keep existing for fade out
    mTargetCount = rData.size();
    const size_t lNewCount = mTargetCount > mSlices.size() ? mTargetCount : mSlices.size();
    ensureSize(lNewCount);

    // Assign targets and color targets
    for (size_t i=0; i<lNewCount; ++i){
        SliceDyn &lS = mSlices[i];
        if (i < mTargetCount){
            lS.mTarget = rData[i].mValue;
            lS.mColorTarget = rData[i].mColor;
            lS.mLabel = rData[i].mLabel;
            // If this was a new slice (currently invisible), start from zero angle and fade in
            if (lS.mVisTarget <= 0.0f && lS.mVis <= 0.0f && lS.mValue <= 0.0f){
                lS.mStart = lS.mEnd; // keep current
                lS.mStartTarget = lS.mEndTarget; // will be set below
                lS.mVis = 0.0f;
            }
            lS.mVisTarget = 1.0f;
        } else {
            // Removed slice: target value 0 and fade out
            lS.mTarget = 0.0f;
            lS.mVisTarget = 0.0f;
        }
    }

    // Compute angle targets from target values (only slices with target or fading out get angles)
    float lSum = 0.0f;
    for (size_t i=0;i<lNewCount;i++) {
        lSum += (mSlices[i].mTarget > 0.0f ? mSlices[i].mTarget : 0.0f);
    }
    // If sum is zero, distribute evenly among currently visible ones to avoid NaNs
    float lAngle = -90.0f; // start from top (12 o'clock)
    if (lSum <= 1e-6f){
        size_t lVisible = 0;
        for (size_t i=0;i<lNewCount;i++) {
            if (mSlices[i].mVisTarget > 0.0f || mSlices[i].mVis > 0.0f) {
                lVisible++;
            }
        }
        const float lStep = lVisible > 0 ? 360.0f / (float)lVisible : 0.0f;
        for (size_t i=0;i<lNewCount;i++){
            SliceDyn &lS = mSlices[i];
            if (lS.mVisTarget > 0.0f || lS.mVis > 0.0f){
                lS.mStartTarget = lAngle;
                lS.mEndTarget = lAngle + lStep;
                lAngle += lStep;
            } else {
                lS.mStartTarget = lS.mEndTarget = lAngle;
            }
        }
        return;
    }

    for (size_t i=0;i<lNewCount;i++){
        SliceDyn &lS = mSlices[i];
        const float lFrac = (lS.mTarget > 0.0f ? (lS.mTarget / lSum) : 0.0f);
        const float lSpan = 360.0f * lFrac;
        lS.mStartTarget = lAngle;
        lS.mEndTarget = lAngle + lSpan;
        lAngle += lSpan;
    }

    // Initialize new slices to zero span at their target start for smooth grow
    for (size_t i=0;i<lNewCount;i++){
        SliceDyn &lS = mSlices[i];
        const bool lWasInvisible = (lS.mVis <= 0.0f) && (lS.mValue <= 0.0f);
        if (i < mTargetCount && lWasInvisible && lS.mVisTarget > 0.0f){
            lS.mStart = lS.mStartTarget;
            lS.mEnd = lS.mStartTarget;
        }
    }
}

void RLPieChart::setData(const std::vector<RLPieSliceData> &rData){
    // Immediate: set as both current and target
    recomputeTargetsFromData(rData);
    for (size_t i=0; i<mSlices.size(); ++i){
        SliceDyn &lS = mSlices[i];
        if (i < rData.size()){
            lS.mValue = lS.mTarget;
            lS.mColor = rData[i].mColor;
            lS.mVis = 1.0f;
        } else {
            lS.mValue = 0.0f;
            lS.mVis = 0.0f;
        }
        lS.mStart = lS.mStartTarget;
        lS.mEnd = lS.mEndTarget;
    }
}

void RLPieChart::setTargetData(const std::vector<RLPieSliceData> &rData){
    recomputeTargetsFromData(rData);
}


void RLPieChart::ensureGeometry() const{
    if (!mGeomDirty) {
        return;
    }
    const float lPad = mStyle.mPadding;
    float lW = mBounds.width - 2.0f * lPad;
    float lH = mBounds.height - 2.0f * lPad;
    lW = std::max(lW, 1.0f);
    lH = std::max(lH, 1.0f);
    const float lR = std::min(lW, lH) * 0.5f;
    mOuterRadius = lR;
    mCenter = Vector2{ mBounds.x + lPad + lW * 0.5f, mBounds.y + lPad + lH * 0.5f };
    mGeomDirty = false;
}

void RLPieChart::update(float aDt){
    const float lAngleK = mStyle.mSmoothAnimate ? (mStyle.mAngleSpeed * aDt) : 1.0f;
    const float lFadeK = mStyle.mSmoothAnimate ? (mStyle.mFadeSpeed * aDt) : 1.0f;
    const float lColorK = mStyle.mSmoothAnimate ? (mStyle.mColorSpeed * aDt) : 1.0f;

    for (auto &lS : mSlices){
        lS.mStart = RLCharts::approach(lS.mStart, lS.mStartTarget, lAngleK);
        lS.mEnd = RLCharts::approach(lS.mEnd, lS.mEndTarget, lAngleK);
        lS.mVis = RLCharts::approach(lS.mVis, lS.mVisTarget, lFadeK);
        lS.mValue = RLCharts::approach(lS.mValue, lS.mTarget, lAngleK);
        lS.mColor = RLCharts::lerpColor(lS.mColor, lS.mColorTarget, RLCharts::clamp01(lColorK));
    }
}

void RLPieChart::draw() const{
    ensureGeometry();
    if (mStyle.mShowBackground){
        DrawRectangleV(Vector2{mBounds.x, mBounds.y}, Vector2{mBounds.width, mBounds.height}, mStyle.mBackground);
    }

    const float lInner = mOuterRadius * RLCharts::clamp01(mHollowFactor);

    for (const auto &lS : mSlices){
        constexpr int lSegments = 72;
        if (lS.mVis <= 0.001f) {
            continue;
        }
        const float lStart = lS.mStart;
        const float lEnd = lS.mEnd;
        if (lEnd <= lStart) {
            continue;
        }
        const Color lC = lS.mColor;
        // apply visibility to alpha
        const float lAlpha = RLCharts::clamp01(lS.mVis);
        const auto lA = static_cast<unsigned char>(std::lround(static_cast<float>(lC.a) * lAlpha));
        const Color lCol{ lC.r, lC.g, lC.b, lA };

        if (lInner <= 0.5f){
            // Solid sector
            DrawCircleSector(mCenter, mOuterRadius, lStart, lEnd, lSegments, lCol);
        } else if (lInner >= mOuterRadius - 0.5f){
            // Fully hollow -> effectively invisible (skip)
            continue;
        } else {
            DrawRing(mCenter, lInner, mOuterRadius, lStart, lEnd, lSegments, lCol);
        }
    }
}
