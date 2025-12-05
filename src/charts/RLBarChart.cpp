// RLBarChart.cpp
#include "RLBarChart.h"
#include "RLCommon.h"
#include <cmath>
#include <cstdlib>


RLBarChart::RLBarChart(Rectangle aBounds, RLBarOrientation aOrientation, const RLBarChartStyle &rStyle)
    : mBounds(aBounds), mOrientation(aOrientation), mStyle(rStyle)
{
    mScaleMin = mStyle.mAutoScale ? 0.0f : mStyle.mMinValue;
    mScaleMax = mStyle.mAutoScale ? 1.0f : fmaxf(mStyle.mMaxValue, mStyle.mMinValue + 1.0f);
    mScaleMaxTarget = mScaleMax;
}

void RLBarChart::setBounds(Rectangle aBounds){ mBounds = aBounds; }

void RLBarChart::setOrientation(RLBarOrientation aOrientation){ mOrientation = aOrientation; }

void RLBarChart::setStyle(const RLBarChartStyle &rStyle){ mStyle = rStyle; }

void RLBarChart::ensureSize(size_t aCount){
    // Only ever grow immediately. Shrinking is animated and trimmed later in Update().
    if (aCount > mBars.size()) mBars.resize(aCount);
}

void RLBarChart::setData(const std::vector<RLBarData> &rData){
    mTargetCount = rData.size();
    // Hard set current data; this is immediate, no appear/disappear animation.
    mBars.clear();
    mBars.resize(mTargetCount);
    for (size_t i=0;i<mTargetCount;++i){
        const RLBarData &lD = rData[i];
        BarDyn &b = mBars[i];
        b.mValue = lD.value;
        b.mTarget = lD.value;
        b.mColor = lD.color;
        b.mColorTarget = lD.color;
        b.mVisAlpha = 1.0f;
        b.mVisTarget = 1.0f;
        b.mShowBorder = lD.showBorder;
        b.mBorderColor = lD.borderColor;
        b.mLabel = lD.label;
    }
    if (mStyle.mAutoScale){
        mScaleMin = 0.0f;
        float lMax = 1.0f;
        for (size_t i=0;i<rData.size();++i) if (rData[i].value > lMax) lMax = rData[i].value;
        mScaleMax = fmaxf(lMax, 1.0f);
        mScaleMaxTarget = mScaleMax;
    } else {
        mScaleMin = mStyle.mMinValue;
        mScaleMax = fmaxf(mStyle.mMaxValue, mStyle.mMinValue + 1.0f);
        mScaleMaxTarget = mScaleMax;
    }
}

void RLBarChart::recomputeScaleTargetsFromData(const std::vector<RLBarData> &data){
    if (!mStyle.mAutoScale) return;
    float lMax = computeAutoMaxFromTargets();
    // also include new incoming data
    for (size_t i=0;i<data.size();++i) if (data[i].value > lMax) lMax = data[i].value;
    mScaleMin = 0.0f;
    mScaleMaxTarget = fmaxf(lMax, 1.0f);
}

float RLBarChart::computeAutoMaxFromTargets() const{
    float lMax = 1.0f;
    for (const auto &b : mBars){ if (b.mTarget > lMax) lMax = b.mTarget; }
    return lMax;
}

void RLBarChart::setTargetData(const std::vector<RLBarData> &rData){
    size_t lOldSize = mBars.size();
    mTargetCount = rData.size();
    ensureSize(mTargetCount);
    for (size_t i=0;i<mTargetCount;++i){
        const RLBarData &lD = rData[i];
        BarDyn &lB = mBars[i];
        lB.mTarget = lD.value;
        lB.mColorTarget = lD.color;
        // If this is a newly added bar, initialize as invisible and fade in
        if (i >= lOldSize){
            lB.mValue = 0.0f; // will grow to target by usual animation
            lB.mColor = { lD.color.r, lD.color.g, lD.color.b, 0 };
            lB.mVisAlpha = 0.0f;
            lB.mVisTarget = 1.0f;
        } else {
            lB.mVisTarget = 1.0f;
        }
        lB.mShowBorder = lD.showBorder;
        lB.mBorderColor = lD.borderColor;
        lB.mLabel = lD.label;
    }
    // Any extra existing bars beyond target count should fade out and be removed later
    for (size_t i=mTargetCount; i<lOldSize; ++i){
        BarDyn &lB = mBars[i];
        lB.mVisTarget = 0.0f;
        // keep current target/value so they animate out without popping
    }
    recomputeScaleTargetsFromData(rData);
}

void RLBarChart::setScale(float aMinValue, float aMaxValue){
    mStyle.mAutoScale = false;
    mStyle.mMinValue = aMinValue;
    mStyle.mMaxValue = aMaxValue;
    mScaleMin = aMinValue;
    mScaleMax = fmaxf(aMaxValue, aMinValue + 1.0f);
    mScaleMaxTarget = mScaleMax;
}

float RLBarChart::lerp(float a, float b, float t) const { return a + (b-a)*t; }

Color RLBarChart::lerp(const Color &a, const Color &b, float t) const {
    return RLCharts::lerpColor(a, b, t);
}

void RLBarChart::update(float aDt){
    if (!mStyle.mSmoothAnimate){
        for (auto &lB : mBars){ lB.mValue = lB.mTarget; lB.mColor = lB.mColorTarget; lB.mVisAlpha = lB.mVisTarget; }
        mScaleMax = mScaleMaxTarget;
        // On hard set, trim any fully hidden bars
        if (mBars.size() > mTargetCount) mBars.resize(mTargetCount);
        return;
    }
    float lLambda = mStyle.mAnimateSpeed; // how fast it converges
    float lAlpha = 1.0f - expf(-lLambda * fmaxf(0.0f, aDt));
    for (auto &rB : mBars){
        rB.mValue = lerp(rB.mValue, rB.mTarget, lAlpha);
        rB.mColor = lerp(rB.mColor, rB.mColorTarget, lAlpha);
        rB.mVisAlpha = lerp(rB.mVisAlpha, rB.mVisTarget, lAlpha);
    }
    mScaleMax = lerp(mScaleMax, mScaleMaxTarget, lAlpha);

    // Remove bars that have faded out and are beyond target range (tail)
    if (mBars.size() > mTargetCount){
        // Trim from the end while bars are fully hidden
        while (mBars.size() > mTargetCount){
            const BarDyn &lB = mBars.back();
            if (lB.mVisTarget <= 0.0f && lB.mVisAlpha < 0.01f){
                mBars.pop_back();
            } else {
                break;
            }
        }
    }
}

void RLBarChart::draw() const{
    // background
    if (mStyle.mShowBackground){
        DrawRectangleRounded(mBounds, 0.08f, 6, mStyle.mBackground);
    }

    const float lPad = mStyle.mPadding;
    Rectangle lInner{ mBounds.x + lPad, mBounds.y + lPad, mBounds.width - 2.0f*lPad, mBounds.height - 2.0f*lPad };

    // grid
    if (mStyle.mShowGrid && mStyle.mGridLines > 0){
        if (mOrientation == RLBarOrientation::VERTICAL){
            for (int i=1;i<=mStyle.mGridLines;i++){
                float t = (float)i / (float)(mStyle.mGridLines+1);
                float y = lInner.y + lInner.height * (1.0f - t);
                DrawLineV({lInner.x, y}, {lInner.x + lInner.width, y}, mStyle.mGridColor);
            }
        }else{
            for (int i=1;i<=mStyle.mGridLines;i++){
                float t = (float)i / (float)(mStyle.mGridLines+1);
                float x = lInner.x + lInner.width * t;
                DrawLineV({x, lInner.y}, {x, lInner.y + lInner.height}, mStyle.mGridColor);
            }
        }
    }

    int lCountAll = (int)mBars.size();
    if (lCountAll <= 0) return;

    const float lCorner = mStyle.mCornerRadius;
    const float lSpacing = mStyle.mSpacing;
    const float lMin = mScaleMin;
    const float lMax = mScaleMax;
    const Font &lFont = (mStyle.mLabelFont.baseSize>0)? mStyle.mLabelFont : GetFontDefault();
    const auto lFontSize = (float)mStyle.mLabelFontSize;

    if (mOrientation == RLBarOrientation::VERTICAL){
        // Compute dynamic weights based on visibility to redistribute space
        float lSumW = 0.0f;
        for (int i=0;i<lCountAll;i++){ lSumW += RLCharts::clamp01(mBars[i].mVisAlpha); }
        if (lSumW <= 0.0001f) return;
        float totalSpacing = lSpacing * fmaxf(0.0f, lSumW - 1.0f);
        float unit = (lInner.width - totalSpacing) / lSumW;
        float x = lInner.x;
        for (int i=0;i<lCountAll;i++){
            const BarDyn &b = mBars[i];
            float s = RLCharts::clamp01(b.mVisAlpha);
            if (s <= 0.0001f) continue;
            float barW = unit * s;
            float t = (b.mValue - lMin) / (lMax - lMin);
            t = RLCharts::clamp01(t);
            float h = lInner.height * (t * s);
            Rectangle r{ x, lInner.y + (lInner.height - h), barW, h };

            // bar fill
            if (r.height > 0.5f){
                Color c = b.mColor; c.a = (unsigned char)((int)c.a * s);
                float corner = (r.height < lCorner*2.0f)? (r.height*0.5f/barW) : (lCorner/barW);
                DrawRectangleRounded(r, corner, 6, c);
            }
            // border
            if (b.mShowBorder && r.height > 1.0f){
                // raylib 5.5 DrawRectangleRoundedLines has no thickness parameter; approximate single-pixel outline
                Color bc = b.mBorderColor; bc.a = (unsigned char)((int)bc.a * s);
                DrawRectangleRoundedLines(r, lCorner/barW, 6, bc);
            }
            // label
            if (mStyle.mShowLabels && !b.mLabel.empty() && r.height > 2.0f){
                Vector2 ts = MeasureTextEx(lFont, b.mLabel.c_str(), lFontSize, 0);
                if (ts.y + 6.0f <= r.height && ts.x + 6.0f <= r.width){
                    Color txt = mStyle.mAutoLabelColor ? ((RLCharts::colorLuma(b.mColor) < 120.0f)? WHITE : BLACK) : mStyle.mLabelColor;
                    txt.a = (unsigned char)((int)txt.a * s);
                    Vector2 pos{ r.x + (r.width - ts.x)*0.5f, r.y + (r.height - ts.y)*0.5f };
                    DrawTextEx(lFont, b.mLabel.c_str(), pos, lFontSize, 0, txt);
                }
            }
            // Add spacing only if there are remaining visible bars
            x += barW;
            // Peek ahead to see if a next visible bar exists
            for (int j=i+1;j<lCountAll;j++){ if (mBars[j].mVisAlpha > 0.0001f){ x += lSpacing; break; } }
        }
    } else { // Horizontal
        float lSumW = 0.0f;
        for (int i=0;i<lCountAll;i++){ lSumW += RLCharts::clamp01(mBars[i].mVisAlpha); }
        if (lSumW <= 0.0001f) return;
        float totalSpacing = lSpacing * fmaxf(0.0f, lSumW - 1.0f);
        float unit = (lInner.height - totalSpacing) / lSumW;
        float y = lInner.y;
        for (int i=0;i<lCountAll;i++){
            const BarDyn &b = mBars[i];
            float s = RLCharts::clamp01(b.mVisAlpha);
            if (s <= 0.0001f) continue;
            float lBarH = unit * s;
            float t = (b.mValue - lMin) / (lMax - lMin);
            t = RLCharts::clamp01(t);
            float w = lInner.width * (t * s);
            Rectangle r{ lInner.x, y, w, lBarH };

            if (r.width > 0.5f){
                Color c = b.mColor; c.a = (unsigned char)((int)c.a * s);
                float lCorner = (r.width < lCorner*2.0f)? (r.width*0.5f/lBarH) : (lCorner/lBarH);
                DrawRectangleRounded(r, lCorner, 6, c);
            }
            if (b.mShowBorder && r.width > 1.0f){
                Color bc = b.mBorderColor; bc.a = (unsigned char)((int)bc.a * s);
                DrawRectangleRoundedLines(r, lCorner/lBarH, 6, bc);
            }
            if (mStyle.mShowLabels && !b.mLabel.empty() && r.width > 2.0f){
                Vector2 ts = MeasureTextEx(lFont, b.mLabel.c_str(), lFontSize, 0);
                if (ts.x + 6.0f <= r.width && ts.y + 6.0f <= r.height){
                    Color txt = mStyle.mAutoLabelColor ? ((RLCharts::colorLuma(b.mColor) < 120.0f)? WHITE : BLACK) : mStyle.mLabelColor;
                    txt.a = (unsigned char)((int)txt.a * s);
                    Vector2 pos{ r.x + (r.width - ts.x)*0.5f, r.y + (r.height - ts.y)*0.5f };
                    DrawTextEx(lFont, b.mLabel.c_str(), pos, lFontSize, 0, txt);
                }
            }
            y += lBarH;
            for (int j=i+1;j<lCountAll;j++) {
                if (mBars[j].mVisAlpha > 0.0001f) {
                    y += lSpacing;
                    break;
                }
            }
        }
    }
}
