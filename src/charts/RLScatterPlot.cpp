#include "RLScatterPlot.h"
#include "RLCommon.h"
#include <cmath>


RLScatterPlot::RLScatterPlot(Rectangle aBounds, const RLScatterPlotStyle &aStyle)
        : mBounds(aBounds), mStyle(aStyle) {
    mGeomDirty = true;
    mScaleDirty = true;
}

void RLScatterPlot::setBounds(Rectangle aBounds){
    mBounds = aBounds;
    mGeomDirty = true;
    markAllDirty();
}

void RLScatterPlot::setStyle(const RLScatterPlotStyle &aStyle){
    mStyle = aStyle;
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

size_t RLScatterPlot::addSeries(const RLScatterSeries &aSeries){
    mSeries.push_back(aSeries);
    mSeries.back().mDirty = true;
    mScaleDirty = true;
    return mSeries.size()-1;
}

void RLScatterPlot::setSeries(size_t aIndex, const RLScatterSeries &aSeries){
    if (aIndex >= mSeries.size()) return;
    mSeries[aIndex] = aSeries;
    mSeries[aIndex].mDirty = true;
    mScaleDirty = true;
}

void RLScatterPlot::setSingleSeries(const std::vector<Vector2> &aData, const RLScatterSeriesStyle &aStyle){
    RLScatterSeries s;
    s.mData = aData;
    s.mStyle = aStyle;
    // reset animation state for this series (immediate)
    s.mTargetData = aData;
    if (mSeries.size() == 1){
        setSeries(0, s);
    } else {
        clearSeries();
        addSeries(s);
    }
}

void RLScatterPlot::markAllDirty() const {
    for (auto &s : mSeries) s.mDirty = true;
}

Rectangle RLScatterPlot::plotRect() const{
    if (!mGeomDirty) return mPlotRect;
    float lPad = RLCharts::maxVal(0.0f, mStyle.mPadding);
    mPlotRect = { mBounds.x + lPad, mBounds.y + lPad,
                  RLCharts::maxVal(1.0f, mBounds.width - 2*lPad), RLCharts::maxVal(1.0f, mBounds.height - 2*lPad) };
    mGeomDirty = false;
    return mPlotRect;
}

void RLScatterPlot::ensureScale() const{
    if (!mScaleDirty) return;
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
        const std::vector<Vector2> *lists[2] = { &s.mData, &s.mTargetData };
        for (int li=0; li<2; ++li){
            const auto &vec = *lists[li];
            for (const auto &p : vec){
                if (lFirst){ lMinX = lMaxX = p.x; lMinY = lMaxY = p.y; lFirst = false; }
                else {
                    if (p.x < lMinX) lMinX = p.x; if (p.x > lMaxX) lMaxX = p.x;
                    if (p.y < lMinY) lMinY = p.y; if (p.y > lMaxY) lMaxY = p.y;
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

Vector2 RLScatterPlot::mapPoint(const Vector2 &aPt) const{
    Rectangle lRect = plotRect();
    ensureScale();
    float lNx = (aPt.x - mScaleMinX) / (mScaleMaxX - mScaleMinX);
    float lNy = (aPt.y - mScaleMinY) / (mScaleMaxY - mScaleMinY);
    // clamp to avoid going outside (optional)
    lNx = clamp01(lNx); lNy = clamp01(lNy);
    Vector2 lOut;
    lOut.x = lRect.x + lNx * lRect.width;
    // y goes top->down, so invert
    lOut.y = lRect.y + (1.0f - lNy) * lRect.height;
    return lOut;
}

Vector2 RLScatterPlot::catmullRom(const Vector2 &aP0, const Vector2 &aP1, const Vector2 &aP2, const Vector2 &aP3, float aT){
    float lT2 = aT*aT; float lT3 = lT2*aT;
    float lX = 0.5f * ((2.0f*aP1.x) + (-aP0.x + aP2.x)*aT + (2*aP0.x - 5*aP1.x + 4*aP2.x - aP3.x)*lT2 + (-aP0.x + 3*aP1.x - 3*aP2.x + aP3.x)*lT3);
    float lY = 0.5f * ((2.0f*aP1.y) + (-aP0.y + aP2.y)*aT + (2*aP0.y - 5*aP1.y + 4*aP2.y - aP3.y)*lT2 + (-aP0.y + 3*aP1.y - 3*aP2.y + aP3.y)*lT3);
    return { lX, lY };
}

float RLScatterPlot::dist(const Vector2 &a, const Vector2 &b){
    float lDx = a.x - b.x; float lDy = a.y - b.y; return sqrtf(lDx*lDx + lDy*lDy);
}

void RLScatterPlot::buildCaches() const{
    Rectangle lRect = plotRect();
    (void)lRect;
    ensureScale();
    // For each series, rebuild mapped points and spline cache if dirty
    for (auto &s : mSeries){
        if (!s.mDirty) continue;
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
            float lTargetPx = RLCharts::maxVal(2.0f, mStyle.mSplinePixels);
            // For endpoints, duplicate end points to compute CR
            size_t lN = lPts.size();
            s.mSpline.reserve(lN * 8);
            s.mSplineVis.reserve(lN * 8);
            for (size_t i = 0; i+1 < lN; ++i){
                const Vector2 &p0 = (i==0) ? lPts[0] : lPts[i-1];
                const Vector2 &p1 = lPts[i];
                const Vector2 &p2 = lPts[i+1];
                const Vector2 &p3 = (i+2<lN) ? lPts[i+2] : lPts[lN-1];
                float lSegLen = dist(p1,p2);
                int lSteps = (int)RLCharts::maxVal(1.0f, floorf(lSegLen / lTargetPx));
                float lInv = 1.0f / (float)lSteps;
                for (int k=0; k<lSteps; ++k){
                    float t = k * lInv;
                    s.mSpline.push_back(catmullRom(p0,p1,p2,p3,t));
                    float lVa = s.mCacheVis[i];
                    float lVb = s.mCacheVis[i+1];
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

    Rectangle lRect = plotRect();

    // Grid / axes
    if (mStyle.mShowGrid){
        int lN = (mStyle.mGridLines < 0) ? 0 : mStyle.mGridLines;
        for (int i=0;i<=lN;i++){
            float lT = (lN==0)?0.0f:(float)i/(float)lN;
            float lX = lRect.x + lT * lRect.width;
            float lY = lRect.y + lT * lRect.height;
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
                        float lVa = (i < s.mCacheVis.size()) ? s.mCacheVis[i] : 1.0f;
                        float lVb = (i+1 < s.mCacheVis.size()) ? s.mCacheVis[i+1] : 1.0f;
                        float lV = RLCharts::minVal(lVa, lVb);
                        if (lV <= 0.001f) continue;
                        Color lC = lSS.mLineColor;
                        lC.a = mulAlpha(lC.a, lV);
                        DrawLineEx(s.mCache[i], s.mCache[i+1], RLCharts::maxVal(1.0f, lSS.mLineThickness), lC);
                    }
                }
            } else { // Spline
                if (s.mSpline.size() >= 2){
                    for (size_t i=0;i+1<s.mSpline.size();++i){
                        float lVa = (i < s.mSplineVis.size()) ? s.mSplineVis[i] : 1.0f;
                        float lVb = (i+1 < s.mSplineVis.size()) ? s.mSplineVis[i+1] : 1.0f;
                        float lV = RLCharts::minVal(lVa, lVb);
                        if (lV <= 0.001f) continue;
                        Color lC = lSS.mLineColor;
                        lC.a = mulAlpha(lC.a, lV);
                        DrawLineEx(s.mSpline[i], s.mSpline[i+1], RLCharts::maxVal(1.0f, lSS.mLineThickness), lC);
                    }
                }
            }
        }
    }

    for (const auto &s : mSeries){
        const RLScatterSeriesStyle &lSS = s.mStyle;
        if (!lSS.mShowPoints) continue;
        Color lPc = (lSS.mPointColor.a == 0) ? lSS.mLineColor : lSS.mPointColor;
        float lRadius = lSS.mPointSizePx > 0.0f ? lSS.mPointSizePx : RLCharts::maxVal(1.0f, lSS.mLineThickness * lSS.mPointScale);
        // Draw all points
        for (size_t i=0; i<s.mCache.size(); ++i){
            float lV = (i < s.mCacheVis.size()) ? s.mCacheVis[i] : 1.0f;
            if (lV <= 0.001f) continue;
            Color lC = lPc; lC.a = mulAlpha(lC.a, lV);
            DrawCircleV(s.mCache[i], lRadius, lC);
        }
    }
}

void RLScatterPlot::ensureDynInitialized(const RLScatterSeries &rSeries) const{
    if (!rSeries.mDynPos.empty()) return;
    const auto &src = rSeries.mData;
    rSeries.mDynPos = src;
    rSeries.mDynTarget = src;
    rSeries.mVis.assign(src.size(), 1.0f);
    rSeries.mVisTarget.assign(src.size(), 1.0f);
}

void RLScatterPlot::setSeriesTargetData(size_t aIndex, const std::vector<Vector2> &aData){
    if (aIndex >= mSeries.size()) return;
    RLScatterSeries &s = mSeries[aIndex];
    s.mTargetData = aData;
    ensureDynInitialized(s);
    size_t lOld = s.mDynPos.size();
    size_t lNew = aData.size();
    s.mDynTarget.resize(lNew);
    // Existing pairs
    size_t lMin = (lOld < lNew) ? lOld : lNew;
    for (size_t i=0;i<lMin;++i){
        s.mDynTarget[i] = aData[i];
        s.mVisTarget[i] = 1.0f;
    }
    // New points fade in
    if (lNew > lOld){
        s.mDynPos.resize(lNew);
        s.mVis.resize(lNew);
        s.mVisTarget.resize(lNew);
        for (size_t i=lOld;i<lNew;++i){
            // start at target position so movement is minimal on add; invis then fade in
            s.mDynPos[i] = aData[i];
            s.mDynTarget[i] = aData[i];
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

void RLScatterPlot::setSingleSeriesTargetData(const std::vector<Vector2> &aData){
    if (mSeries.empty()){
        RLScatterSeries s; s.mData = aData; s.mTargetData = aData; addSeries(s);
    }
    setSeriesTargetData(0, aData);
}

void RLScatterPlot::update(float aDt){
    if (aDt <= 0.0f) return;
    float lMoveT = clamp01(mStyle.lMoveSpeed * aDt);
    float lFadeT = clamp01(mStyle.lFadeSpeed * aDt);
    for (auto &s : mSeries){
        ensureDynInitialized(s);
        bool lAnyChange = false;
        size_t n = s.mDynPos.size();
        // If we had points faded to zero and beyond new size, we can shrink containers lazily
        // But keep them until visibility is ~0
        for (size_t i=0;i<n;++i){
            // If i exceeds target array (after shrink), keep target at current to not move
            Vector2 lTarget = (i < s.mDynTarget.size()) ? s.mDynTarget[i] : s.mDynPos[i];
            Vector2 lP = s.mDynPos[i];
            lP.x += (lTarget.x - lP.x) * lMoveT;
            lP.y += (lTarget.y - lP.y) * lMoveT;
            if (fabsf(lP.x - s.mDynPos[i].x) > 1e-6f || fabsf(lP.y - s.mDynPos[i].y) > 1e-6f) lAnyChange = true;
            s.mDynPos[i] = lP;
            float lVt = (i < s.mVisTarget.size()) ? s.mVisTarget[i] : 1.0f;
            float lV = s.mVis[i] + (lVt - s.mVis[i]) * lFadeT;
            if (fabsf(lV - s.mVis[i]) > 1e-6f) lAnyChange = true;
            s.mVis[i] = lV;
        }
        // Remove fully faded trailing items (compact vectors end-to-start)
        // Keep alignment by erasing elements with vis ~0 beyond target size
        if (!s.mVis.empty()){
            size_t i = 0;
            // Compact in-place to avoid allocations
            size_t w = 0;
            size_t total = s.mDynPos.size();
            for (i=0; i<total; ++i){
                bool removeCond = (i >= s.mTargetData.size()) && (s.mVis[i] < 0.01f);
                if (!removeCond){
                    if (w != i){
                        s.mDynPos[w] = s.mDynPos[i];
                        if (i < s.mDynTarget.size()){
                            if (w >= s.mDynTarget.size()) s.mDynTarget.push_back(s.mDynTarget[i]);
                            else s.mDynTarget[w] = s.mDynTarget[i];
                        }
                        s.mVis[w] = s.mVis[i];
                        s.mVisTarget[w] = (i < s.mVisTarget.size()) ? s.mVisTarget[i] : 1.0f;
                    }
                    ++w;
                }
            }
            if (w != total){
                s.mDynPos.resize(w);
                if (s.mDynTarget.size() > w) s.mDynTarget.resize(w);
                s.mVis.resize(w);
                if (s.mVisTarget.size() > w) s.mVisTarget.resize(w);
                lAnyChange = true;
            }
        }
        if (lAnyChange){ s.mDirty = true; }
        // Also keep s.mData in sync for immediate replace semantics
        s.mData = s.mDynPos; // so external getters (if any) would see moving state; also scale uses mData
    }
}
