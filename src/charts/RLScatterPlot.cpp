#include "RLScatterPlot.h"
#include "RLCommon.h"
#include <algorithm>
#include <cmath>


RLScatterPlot::RLScatterPlot(Rectangle aBounds, const RLScatterPlotStyle &rStyle)
        : mBounds(aBounds), mStyle(rStyle) {
    mGeomDirty = true;
    mScaleDirty = true;
}

void RLScatterPlot::setBounds(Rectangle aBounds){
    mBounds = aBounds;
    mGeomDirty = true;
    markAllDirty();
}

void RLScatterPlot::setStyle(const RLScatterPlotStyle &rStyle){
    mStyle = rStyle;
    mGeomDirty = true;
    mScaleDirty = true;
    markAllDirty();
}

void RLScatterPlot::setScale(float aMinX, float aMaxX, float aMinY, float aMaxY){
    mStyle.mAutoScale = false;
    mStyle.mMinX = aMinX; mStyle.mMaxX = aMaxX;
    mStyle.mMinY = aMinY; mStyle.mMaxY = aMaxY;
    mScaleDirty = true;
    markAllDirty();
}

void RLScatterPlot::clearSeries(){
    mSeries.clear();
    mScaleDirty = true;
}

size_t RLScatterPlot::addSeries(const RLScatterSeries &rSeries){
    mSeries.push_back(rSeries);
    mSeries.back().mDirty = true;
    mScaleDirty = true;
    return mSeries.size()-1;
}

void RLScatterPlot::setSeries(size_t aIndex, const RLScatterSeries &rSeries){
    if (aIndex >= mSeries.size()) {
        return;
    }
    mSeries[aIndex] = rSeries;
    mSeries[aIndex].mDirty = true;
    mScaleDirty = true;
}

void RLScatterPlot::setSingleSeries(const std::vector<Vector2> &rData, const RLScatterSeriesStyle &aStyle){
    RLScatterSeries s;
    s.mData = rData;
    s.mStyle = aStyle;
    // reset animation state for this series (immediate)
    s.mTargetData = rData;
    if (mSeries.size() == 1){
        setSeries(0, s);
    } else {
        clearSeries();
        addSeries(s);
    }
}

void RLScatterPlot::markAllDirty() const {
    for (auto &s : mSeries) {
        s.mDirty = true;
    }
}

Rectangle RLScatterPlot::plotRect() const{
    if (!mGeomDirty) {
        return mPlotRect;
    }
    const float lPad = RLCharts::maxVal(0.0f, mStyle.mPadding);
    mPlotRect = { mBounds.x + lPad, mBounds.y + lPad,
                  RLCharts::maxVal(1.0f, mBounds.width - 2*lPad), RLCharts::maxVal(1.0f, mBounds.height - 2*lPad) };
    mGeomDirty = false;
    return mPlotRect;
}

void RLScatterPlot::ensureScale() const{
    if (!mScaleDirty) {
        return;
    }
    if (!mStyle.mAutoScale){
        mScaleMinX = mStyle.mMinX; mScaleMaxX = mStyle.mMaxX;
        mScaleMinY = mStyle.mMinY; mScaleMaxY = mStyle.mMaxY;
        mScaleDirty = false;
        return;
    }
    // Auto scale from all series data
    float lMinX = 0.0f, lMaxX = 1.0f, lMinY = 0.0f, lMaxY = 1.0f;
    bool lFirst = true;
    for (const auto &s : mSeries){
        // consider both current and target data to avoid popping during animation
        const std::vector<Vector2> * const lLists[2] = { &s.mData, &s.mTargetData };
        for (int li=0; li<2; ++li){
            const auto &vec = *lLists[li];
            for (const auto &p : vec){
                if (lFirst){
                    lMinX = lMaxX = p.x;
                    lMinY = lMaxY = p.y;
                    lFirst = false;
                } else {
                    lMinX = std::min(p.x, lMinX);
                    lMaxX = std::max(p.x, lMaxX);
                    lMinY = std::min(p.y, lMinY);
                    lMaxY = std::max(p.y, lMaxY);
                }
            }
        }
    }
    if (lFirst){ // no data
        lMinX = 0.0f; lMaxX = 1.0f; lMinY = 0.0f; lMaxY = 1.0f;
    }
    // Avoid degenerate ranges
    if (fabsf(lMaxX - lMinX) < 1e-6f){ lMaxX = lMinX + 1.0f; }
    if (fabsf(lMaxY - lMinY) < 1e-6f){ lMaxY = lMinY + 1.0f; }
    mScaleMinX = lMinX; mScaleMaxX = lMaxX;
    mScaleMinY = lMinY; mScaleMaxY = lMaxY;
    mScaleDirty = false;
}

Vector2 RLScatterPlot::mapPoint(const Vector2 &rPt) const{
    const Rectangle lRect = plotRect();
    ensureScale();
    float lNx = (rPt.x - mScaleMinX) / (mScaleMaxX - mScaleMinX);
    float lNy = (rPt.y - mScaleMinY) / (mScaleMaxY - mScaleMinY);
    // clamp to avoid going outside (optional)
    lNx = RLCharts::clamp01(lNx); lNy = RLCharts::clamp01(lNy);
    Vector2 lOut;
    lOut.x = lRect.x + lNx * lRect.width;
    // y goes top->down, so invert
    lOut.y = lRect.y + (1.0f - lNy) * lRect.height;
    return lOut;
}


void RLScatterPlot::buildCaches() const{
    const Rectangle lRect = plotRect();
    (void)lRect;
    ensureScale();
    // For each series, rebuild mapped points and spline cache if dirty
    for (auto &s : mSeries){
        if (!s.mDirty) {
            continue;
        }
        // Ensure dyn arrays are initialized
        ensureDynInitialized(s);
        // Map to screen space from dynamic positions
        s.mCache.clear();
        s.mCache.reserve(s.mDynPos.size());
        s.mCacheVis.clear();
        s.mCacheVis.reserve(s.mDynPos.size());
        for (size_t i=0; i<s.mDynPos.size(); ++i){
            s.mCache.push_back(mapPoint(s.mDynPos[i]));
            s.mCacheVis.push_back(s.mVis[i]);
        }

        // Build spline polyline if needed (with visibility sampling)
        s.mSpline.clear();
        s.mSplineVis.clear();
        if (s.mStyle.mLineMode == RLScatterLineMode::Spline && s.mCache.size() >= 2){
            const std::vector<Vector2> &lPts = s.mCache;
            // Estimate sampling based on pixel spacing
            const float lTargetPx = RLCharts::maxVal(2.0f, mStyle.mSplinePixels);
            // For endpoints, duplicate end points to compute CR
            const size_t lN = lPts.size();
            s.mSpline.reserve(lN * 8);
            s.mSplineVis.reserve(lN * 8);
            for (size_t i = 0; i+1 < lN; ++i){
                const Vector2 &p0 = (i==0) ? lPts[0] : lPts[i-1];
                const Vector2 &p1 = lPts[i];
                const Vector2 &p2 = lPts[i+1];
                const Vector2 &p3 = (i+2<lN) ? lPts[i+2] : lPts[lN-1];
                const float lSegLen = RLCharts::distance(p1,p2);
                const int lSteps = (int)RLCharts::maxVal(1.0f, floorf(lSegLen / lTargetPx));
                const float lInv = 1.0f / (float)lSteps;
                for (int k=0; k<lSteps; ++k){
                    const float t = (float)k * lInv;
                    s.mSpline.push_back(RLCharts::catmullRom(p0,p1,p2,p3,t));
                    const float lVa = s.mCacheVis[i];
                    const float lVb = s.mCacheVis[i+1];
                    s.mSplineVis.push_back(lVa + (lVb - lVa) * t);
                }
            }
            // Ensure last point appended
            s.mSpline.push_back(s.mCache.back());
            s.mSplineVis.push_back(s.mCacheVis.back());
        }
        s.mDirty = false;
    }
}

void RLScatterPlot::draw() const{
    // Background
    if (mStyle.mShowBackground){
        DrawRectangleRounded(mBounds, 0.06f, 6, mStyle.mBackground);
    }

    const Rectangle lRect = plotRect();

    // Grid / axes
    if (mStyle.mShowGrid){
        const int lN = (mStyle.mGridLines < 0) ? 0 : mStyle.mGridLines;
        for (int i=0;i<=lN;i++){
            const float lT = (lN==0)?0.0f:(float)i/(float)lN;
            const float lX = lRect.x + lT * lRect.width;
            const float lY = lRect.y + lT * lRect.height;
            DrawLineV({lX, lRect.y}, {lX, lRect.y + lRect.height}, mStyle.mGridColor);
            DrawLineV({lRect.x, lY}, {lRect.x + lRect.width, lY}, mStyle.mGridColor);
        }
    }
    if (mStyle.mShowAxes){
        DrawRectangleLinesEx(lRect, 1.0f, mStyle.mAxesColor);
    }

    // Build caches if needed
    buildCaches();

    // Draw series (lines first then points so points are on top). Alpha is modulated by visibility.
    for (const auto &s : mSeries){
        const RLScatterSeriesStyle &lSS = s.mStyle;
        if (lSS.mLineMode != RLScatterLineMode::None){
            if (lSS.mLineMode == RLScatterLineMode::Linear){
                // Draw consecutive segments
                if (s.mCache.size() >= 2){
                    for (size_t i=0;i+1<s.mCache.size();++i){
                        const float lVa = (i < s.mCacheVis.size()) ? s.mCacheVis[i] : 1.0f;
                        const float lVb = (i+1 < s.mCacheVis.size()) ? s.mCacheVis[i+1] : 1.0f;
                        const float lV = RLCharts::minVal(lVa, lVb);
                        if (lV <= 0.001f) {
                            continue;
                        }
                        Color lC = lSS.mLineColor;
                        lC.a = RLCharts::mulAlpha(lC.a, lV);
                        DrawLineEx(s.mCache[i], s.mCache[i+1], RLCharts::maxVal(1.0f, lSS.mLineThickness), lC);
                    }
                }
            } else { // Spline
                if (s.mSpline.size() >= 2){
                    for (size_t i=0;i+1<s.mSpline.size();++i){
                        const float lVa = (i < s.mSplineVis.size()) ? s.mSplineVis[i] : 1.0f;
                        const float lVb = (i+1 < s.mSplineVis.size()) ? s.mSplineVis[i+1] : 1.0f;
                        const float lV = RLCharts::minVal(lVa, lVb);
                        if (lV <= 0.001f) {
                            continue;
                        }
                        Color lC = lSS.mLineColor;
                        lC.a = RLCharts::mulAlpha(lC.a, lV);
                        DrawLineEx(s.mSpline[i], s.mSpline[i+1], RLCharts::maxVal(1.0f, lSS.mLineThickness), lC);
                    }
                }
            }
        }
    }

    for (const auto &s : mSeries){
        const RLScatterSeriesStyle &lSS = s.mStyle;
        if (!lSS.mShowPoints) {
            continue;
        }
        const Color lPc = (lSS.mPointColor.a == 0) ? lSS.mLineColor : lSS.mPointColor;
        const float lRadius = lSS.mPointSizePx > 0.0f ? lSS.mPointSizePx : RLCharts::maxVal(1.0f, lSS.mLineThickness * lSS.mPointScale);
        // Draw all points
        for (size_t i=0; i<s.mCache.size(); ++i){
            const float lV = (i < s.mCacheVis.size()) ? s.mCacheVis[i] : 1.0f;
            if (lV <= 0.001f) {
                continue;
            }
            Color lC = lPc;
            lC.a = RLCharts::mulAlpha(lC.a, lV);
            DrawCircleV(s.mCache[i], lRadius, lC);
        }
    }
}

void RLScatterPlot::ensureDynInitialized(const RLScatterSeries &rSeries) const{
    if (!rSeries.mDynPos.empty()) {
        return;
    }
    const auto &src = rSeries.mData;
    rSeries.mDynPos = src;
    rSeries.mDynTarget = src;
    rSeries.mVis.assign(src.size(), 1.0f);
    rSeries.mVisTarget.assign(src.size(), 1.0f);
}

void RLScatterPlot::setSeriesTargetData(size_t aIndex, const std::vector<Vector2> &rData){
    if (aIndex >= mSeries.size()) {
        return;
    }
    RLScatterSeries &s = mSeries[aIndex];
    s.mTargetData = rData;
    ensureDynInitialized(s);
    const size_t lOld = s.mDynPos.size();
    const size_t lNew = rData.size();
    s.mDynTarget.resize(lNew);
    // Existing pairs
    const size_t lMin = std::min(lOld, lNew);
    for (size_t i=0;i<lMin;++i){
        s.mDynTarget[i] = rData[i];
        s.mVisTarget[i] = 1.0f;
    }
    // New points fade in
    if (lNew > lOld){
        s.mDynPos.resize(lNew);
        s.mVis.resize(lNew);
        s.mVisTarget.resize(lNew);
        for (size_t i=lOld;i<lNew;++i){
            // start at target position so movement is minimal on add; invis then fade in
            s.mDynPos[i] = rData[i];
            s.mDynTarget[i] = rData[i];
            s.mVis[i] = 0.0f;
            s.mVisTarget[i] = 1.0f;
        }
    }
    // Old points beyond new size fade out
    if (lOld > lNew){
        for (size_t i=lNew; i<lOld; ++i){
            s.mVisTarget[i] = 0.0f;
        }
    }
    mScaleDirty = true;
    s.mDirty = true;
}

void RLScatterPlot::setSingleSeriesTargetData(const std::vector<Vector2> &rData){
    if (mSeries.empty()){
        RLScatterSeries s; s.mData = rData; s.mTargetData = rData; addSeries(s);
    }
    setSeriesTargetData(0, rData);
}

void RLScatterPlot::update(float aDt){
    if (aDt <= 0.0f) {
        return;
    }
    const float lMoveT = RLCharts::clamp01(mStyle.mMoveSpeed * aDt);
    const float lFadeT = RLCharts::clamp01(mStyle.mFadeSpeed * aDt);
    for (auto &s : mSeries){
        ensureDynInitialized(s);
        bool lAnyChange = false;
        const size_t n = s.mDynPos.size();
        // If we had points faded to zero and beyond new size, we can shrink containers lazily
        // But keep them until visibility is ~0
        for (size_t i=0;i<n;++i){
            // If i exceeds target array (after shrink), keep target at current to not move
            const Vector2 lTarget = (i < s.mDynTarget.size()) ? s.mDynTarget[i] : s.mDynPos[i];
            Vector2 lP = s.mDynPos[i];
            lP.x += (lTarget.x - lP.x) * lMoveT;
            lP.y += (lTarget.y - lP.y) * lMoveT;
            if (fabsf(lP.x - s.mDynPos[i].x) > 1e-6f || fabsf(lP.y - s.mDynPos[i].y) > 1e-6f) {
                lAnyChange = true;
            }
            s.mDynPos[i] = lP;
            const float lVt = (i < s.mVisTarget.size()) ? s.mVisTarget[i] : 1.0f;
            const float lV = s.mVis[i] + (lVt - s.mVis[i]) * lFadeT;
            if (fabsf(lV - s.mVis[i]) > 1e-6f) {
                lAnyChange = true;
            }
            s.mVis[i] = lV;
        }
        // Remove fully faded trailing items (compact vectors end-to-start)
        // Keep alignment by erasing elements with vis ~0 beyond target size
        if (!s.mVis.empty()){
            size_t i = 0;
            // Compact in-place to avoid allocations
            size_t w = 0;
            const size_t total = s.mDynPos.size();
            for (i=0; i<total; ++i){
                const bool removeCond = (i >= s.mTargetData.size()) && (s.mVis[i] < 0.01f);
                if (!removeCond){
                    if (w != i){
                        s.mDynPos[w] = s.mDynPos[i];
                        if (i < s.mDynTarget.size()){
                            if (w >= s.mDynTarget.size()) {
                                s.mDynTarget.push_back(s.mDynTarget[i]);
                            } else {
                                s.mDynTarget[w] = s.mDynTarget[i];
                            }
                        }
                        s.mVis[w] = s.mVis[i];
                        s.mVisTarget[w] = (i < s.mVisTarget.size()) ? s.mVisTarget[i] : 1.0f;
                    }
                    ++w;
                }
            }
            if (w != total){
                s.mDynPos.resize(w);
                if (s.mDynTarget.size() > w) {
                    s.mDynTarget.resize(w);
                }
                s.mVis.resize(w);
                if (s.mVisTarget.size() > w) {
                    s.mVisTarget.resize(w);
                }
                lAnyChange = true;
            }
        }
        if (lAnyChange){ s.mDirty = true; }
        // Also keep s.mData in sync for immediate replace semantics
        s.mData = s.mDynPos; // so external getters (if any) would see moving state; also scale uses mData
    }
}
