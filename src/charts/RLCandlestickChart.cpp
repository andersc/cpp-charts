// RLCandlestickChart.cpp
#include "RLCandlestickChart.h"
#include "RLCommon.h"
#include <cmath>
#include <algorithm>


RLCandlestickChart::RLCandlestickChart(Rectangle aBounds, int aValuesPerCandle, int aVisibleCandles, const RLCandleStyle &aStyle)
    : mBounds(aBounds), mStyle(aStyle), mValuesPerCandle(aValuesPerCandle), mVisibleCandles(aVisibleCandles)
{
    mScaleMin = aStyle.lMinPrice;
    mScaleMax = aStyle.lMaxPrice;
    mScaleTargetMax = mScaleMax;
}

void RLCandlestickChart::setBounds(Rectangle aBounds){ mBounds = aBounds; }
void RLCandlestickChart::setValuesPerCandle(int aValuesPerCandle){ mValuesPerCandle = (aValuesPerCandle<=0)?1:aValuesPerCandle; }
void RLCandlestickChart::setVisibleCandles(int aVisibleCandles){ mVisibleCandles = (aVisibleCandles<=1)?1:aVisibleCandles; ensureWindow(); }
void RLCandlestickChart::setStyle(const RLCandleStyle &rStyle){ mStyle = rStyle; }
void RLCandlestickChart::setExplicitScale(float aMinPrice, float aMaxPrice){
    mStyle.mAutoScale = false;
    mScaleMin = aMinPrice;
    mScaleMax = aMaxPrice > aMinPrice ? aMaxPrice : (aMinPrice + 1.0f);
    mScaleTargetMax = mScaleMax;
}

std::string RLCandlestickChart::dayKeyFromDate(const std::string &aDate){
    // Expect formats like YYYY-MM-DD HH:MM:SS or YYYY-MM-DD
    size_t lPos = aDate.find(' ');
    if (lPos == std::string::npos) return aDate;
    return aDate.substr(0, lPos);
}

void RLCandlestickChart::addSample(const CandleInput &rSample){
    std::string lIncomingDay = dayKeyFromDate(rSample.aDate);
    if (mHasWorking && lIncomingDay != mWorking.mDayKey){
        // Day changed: finalize current candle early to align separator exactly at boundary
        finalizeWorkingCandle();
    }

    if (!mHasWorking){
        // Starting a new candle
        // For single-value candles (mValuesPerCandle==1), use the last close as open
        if (mValuesPerCandle == 1 && mHasLastClose) {
            mWorking.mOpen = mLastClose;
        } else {
            mWorking.mOpen = rSample.aOpen;
        }
        mWorking.mHigh = rSample.aHigh;
        mWorking.mLow = rSample.aLow;
        mWorking.mClose = rSample.aClose;
        mWorking.mVolume = rSample.aVolume;
        mWorking.mDate = rSample.aDate;
        mWorking.mDayKey = lIncomingDay;
        mWorking.mDaySeparator = false; // finalized candle will decide
        mWorkingCount = 1;
        mHasWorking = true;
    } else {
        // aggregate
        if (rSample.aHigh > mWorking.mHigh) mWorking.mHigh = rSample.aHigh;
        if (rSample.aLow < mWorking.mLow) mWorking.mLow = rSample.aLow;
        mWorking.mClose = rSample.aClose;
        mWorking.mVolume += rSample.aVolume;
        mWorking.mDate = rSample.aDate; // keep last timestamp
        mWorkingCount += 1;
    }

    if (mWorkingCount >= mValuesPerCandle){
        finalizeWorkingCandle();
    }
}

void RLCandlestickChart::finalizeWorkingCandle(){
    // Determine if day changed compared to last finalized candle
    bool lNewDay = false;
    std::string lKey = mWorking.mDayKey;
    if (mCandles.empty()){
        lNewDay = true;
    } else {
        const CandleDyn &lLast = mCandles.back();
        lNewDay = (lLast.mDayKey != lKey);
    }
    CandleDyn lFinal = mWorking;
    lFinal.mDaySeparator = lNewDay;

    // Store the close price for next candle (important for single-value candles)
    mLastClose = mWorking.mClose;
    mHasLastClose = true;

    // Trigger slide for every new candle to ensure smooth "scroll left"
    mIncoming = lFinal;
    mHasIncoming = true;
    mIsSliding = true;
    mSlideProgress = 0.0f;

    // Reset working state
    mHasWorking = false;
    mWorkingCount = 0;

    ensureWindow();
}

void RLCandlestickChart::ensureWindow(){
    // Keep at most mVisibleCandles in deque.
    while ((int)mCandles.size() > mVisibleCandles){
        // If sliding, pop when slide finishes in update.
        // If not sliding (e.g., init), just drop oldest.
        if (!mIsSliding){
            mCandles.pop_front();
        } else {
            break;
        }
    }
}

Rectangle RLCandlestickChart::priceArea() const{
    float lPad = mStyle.mPadding;
    float lVolumeH = mBounds.height * mStyle.mVolumeAreaRatio;
    Rectangle lR{ mBounds.x + lPad, mBounds.y + lPad, mBounds.width - 2.0f*lPad, mBounds.height - 2.0f*lPad - lVolumeH };
    return lR;
}

Rectangle RLCandlestickChart::volumeArea() const{
    float lPad = mStyle.mPadding;
    float lVolumeH = mBounds.height * mStyle.mVolumeAreaRatio;
    Rectangle lR{ mBounds.x + lPad, mBounds.y + mBounds.height - lPad - lVolumeH, mBounds.width - 2.0f*lPad, lVolumeH };
    return lR;
}

float RLCandlestickChart::extractPriceMax() const{
    float lMax = mStyle.mAutoScale ? 0.0f : mStyle.lMaxPrice;
    for (const auto & lC : mCandles){
        float lVal = mStyle.mIncludeWicksInScale ? lC.mHigh : (lC.mOpen>lC.mClose?lC.mOpen:lC.mClose);
        if (lVal > lMax) lMax = lVal;
    }
    if (mHasWorking){
        float lVal = mStyle.mIncludeWicksInScale ? mWorking.mHigh : (mWorking.mOpen>mWorking.mClose?mWorking.mOpen:mWorking.mClose);
        if (lVal > lMax) lMax = lVal;
    }
    if (lMax <= 0.0f) lMax = 1.0f;
    return lMax;
}

void RLCandlestickChart::update(float aDt){
    // Update scaling
    if (mStyle.mAutoScale){
        float lTarget = extractPriceMax();
        mScaleTargetMax = lTarget;
        const float lT = RLCharts::clamp01(mStyle.mFadeSpeed * aDt);
        mScaleMax = RLCharts::lerpF(mScaleMax, mScaleTargetMax, lT);
        // Compute min as min visible low for better framing
        float lMin = 1e30f;
        for (auto & mCandle : mCandles) {
            if (mCandle.mLow < lMin) lMin = mCandle.mLow;
        }
        if (mHasWorking && mWorking.mLow < lMin) lMin = mWorking.mLow;
        if (lMin == 1e30f) lMin = 0.0f;
        mScaleMin = lMin;
        if (mScaleMax <= mScaleMin) mScaleMax = mScaleMin + 1.0f;
    }

    // Update slide progress
    if (mIsSliding){
        float lStep = mStyle.mSlideSpeed * aDt; // measured in widths
        mSlideProgress += lStep;
        if (mSlideProgress >= 1.0f){
            mSlideProgress = 1.0f;
            mIsSliding = false;
            // Append the incoming candle now that slide finished
            if (mHasIncoming){
                mCandles.push_back(mIncoming);
                mHasIncoming = false;
            }
            // Remove oldest if window exceeded
            while ((int)mCandles.size() > mVisibleCandles) mCandles.pop_front();
        }
    }
}

void RLCandlestickChart::draw() const{
    // Background
    DrawRectangleRec(mBounds, mStyle.mBackground);

    Rectangle lPriceR = priceArea();
    Rectangle lVolR = volumeArea();

    // Grid (horizontal)
    if (mStyle.mGridLines > 0){
        for (int i=0;i<=mStyle.mGridLines;++i){
            float lY = lPriceR.y + (lPriceR.height * (float)i / (float)(mStyle.mGridLines));
            DrawLine((int)lPriceR.x, (int)lY, (int)(lPriceR.x + lPriceR.width), (int)lY, mStyle.mGridColor);
        }
    }

    int lVisible = mVisibleCandles;
    float lSpacing = mStyle.mCandleSpacing;
    float lBodyWidth = (lPriceR.width - lSpacing * (lVisible - 1)) / (float)lVisible;
    if (lBodyWidth < mStyle.mBodyMinWidth) lBodyWidth = mStyle.mBodyMinWidth;
    float lUnit = (lBodyWidth + lSpacing);

    // Helpers
    float lPriceRange = (mScaleMax - mScaleMin);
    if (lPriceRange <= 0.0001f) lPriceRange = 1.0f;
    auto priceToY = [&](float aPrice){
        float lNorm = (aPrice - mScaleMin) / lPriceRange; // 0 bottom .. 1 top
        lNorm = 1.0f - RLCharts::clamp01(lNorm);
        return lPriceR.y + lNorm * lPriceR.height;
    };

    // Calculate Max Volume for scaling
    float lMaxVol = 1.0f;
    for (const auto& c : mCandles) { if (c.mVolume > lMaxVol) lMaxVol = c.mVolume; }
    if (mHasWorking && mWorking.mVolume > lMaxVol) lMaxVol = mWorking.mVolume;
    if (mHasIncoming && mIncoming.mVolume > lMaxVol) lMaxVol = mIncoming.mVolume;

    // Helper to draw a single candle at absolute X
    auto drawSingleCandle = [&](const CandleDyn& aC, float aX, bool aIsWorking) {
        if (aX + lBodyWidth < lPriceR.x - 2.0f) return;
        if (aX > lPriceR.x + lPriceR.width + 2.0f) return;

        bool lUp = aC.mClose >= aC.mOpen;
        Color lBodyColor = lUp ? mStyle.mUpBody : mStyle.mDownBody;
        Color lWickColor = lUp ? mStyle.mUpWick : mStyle.mDownWick;

        // Apply full opacity for all candles (working and finalized)
        unsigned char lA = 255;

        lBodyColor.a = lA; lWickColor.a = lA;

        // Wick
        float lYH = priceToY(aC.mHigh);
        float lYL = priceToY(aC.mLow);
        DrawLineEx({ aX + lBodyWidth*0.5f, lYH }, { aX + lBodyWidth*0.5f, lYL }, mStyle.mWickThickness, lWickColor);

        // Body
        float lYO = priceToY(aC.mOpen);
        float lYC = priceToY(aC.mClose);
        float lYTop = lYO < lYC ? lYO : lYC;
        float lH = std::fabs(lYC - lYO);
        if (lH < 1.0f) { lH = 1.0f; lYTop -= 0.5f; }
        Rectangle lBody{ aX, lYTop, lBodyWidth, lH };
        DrawRectangleRec(lBody, lBodyColor);

        // Day separator
        if (aC.mDaySeparator){
            Color lSep = mStyle.mSeparator; lSep.a = lA;
            DrawLineV({ aX - lSpacing*0.5f, lPriceR.y }, { aX - lSpacing*0.5f, lPriceR.y + lPriceR.height }, lSep);
        }

        // Volume
        float lVN = aC.mVolume / lMaxVol;
        float lVH = lVolR.height * RLCharts::clamp01(lVN);
        float lVY = lVolR.y + lVolR.height - lVH;
        Color lVolColor = lUp ? mStyle.mVolumeUp : mStyle.mVolumeDown;
        lVolColor.a = lA;
        DrawRectangle((int)aX, (int)lVY, (int)lBodyWidth, (int)lVH, lVolColor);
    };

    // --- RENDER LOGIC ---
    // We anchor the coordinate system to the Right Edge.
    // The "Rightmost Slot" (Slot 0) is reserved for the Working candle.
    // The "Next Slot" (Slot -1) is for the last Finalized candle (or Incoming).
    // All items slide left by (SlideProgress * Unit) when sliding.

    float lRightEdge = lPriceR.x + lPriceR.width;
    float lSlot0_X = lRightEdge - lBodyWidth; // Base X for the Working Slot

    // Global render offset due to sliding
    float lSlideOffset = 0.0f;
    if (mIsSliding) {
        lSlideOffset = -mSlideProgress * lUnit;
    }

    // 1. Draw Working Candle (if any)
    // If sliding, it is entering from right (Slot +1 -> Slot 0).
    // If not sliding, it is stable at Slot 0.
    if (mHasWorking) {
        float lWorkingX = lSlot0_X;
        if (mIsSliding) {
            // It visually moves from (Slot 0 + Unit) to (Slot 0)
            // Combined with lSlideOffset which is (-Progress * Unit), we need logic:
            // Target Pos = Slot 0. Start Pos = Slot +1.
            // Pos = Slot0 + Unit * (1.0 - Progress).
            // Let's rely on global offset:
            // lWorkingX + lSlideOffset = (Slot0) - Slide.
            // We want (Slot0 + Unit) - Slide.
            lWorkingX += lUnit;
        }
        drawSingleCandle(mWorking, lWorkingX + lSlideOffset, true);
    }

    // 2. Draw Incoming Candle (only if sliding)
    // It is moving from Slot 0 (Working pos) to Slot -1 (Finalized pos).
    // At Progress=0: Pos = Slot0.
    // At Progress=1: Pos = Slot0 - Unit.
    // Formula: Slot0 + SlideOffset matches this.
    if (mIsSliding && mHasIncoming) {
        drawSingleCandle(mIncoming, lSlot0_X + lSlideOffset, false);
    }

    // 3. Draw History Candles (mCandles)
    // If sliding: The newest history candle was at Slot -1, moving to Slot -2.
    //             Start drawing at (Slot0 - Unit) + SlideOffset.
    // If NOT sliding: The newest history candle IS at Slot -1 (stable).
    //             Start drawing at (Slot0 - Unit).

    float lHistoryStartX = lSlot0_X - lUnit + lSlideOffset;

    // Iterate backwards so we draw from right to left
    int lCount = (int)mCandles.size();
    for (int i = lCount - 1; i >= 0; --i) {
        // Calculate offset from the newest history candle
        int lDist = (lCount - 1) - i; // 0 for newest
        float lX = lHistoryStartX - (lDist * lUnit);

        drawSingleCandle(mCandles[i], lX, false);
    }
}