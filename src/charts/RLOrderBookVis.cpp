// RLOrderBookVis.cpp
#include "RLOrderBookVis.h"
#include "RLCommon.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>
#include <cstring>

RLOrderBookVis::RLOrderBookVis(Rectangle aBounds, size_t aHistoryLength, size_t aPriceLevels)
    : mBounds(aBounds)
    , mHistoryLength(aHistoryLength > 0 ? aHistoryLength : 100)
    , mPriceLevels(aPriceLevels > 0 ? aPriceLevels : 50)
{
    // Default bid color stops: dark blue -> cyan -> bright green
    mBidStops = {
        Color{0, 20, 40, 255},
        Color{0, 80, 120, 255},
        Color{0, 180, 120, 255},
        Color{80, 255, 160, 255}
    };

    // Default ask color stops: dark red -> orange -> bright red
    mAskStops = {
        Color{40, 10, 10, 255},
        Color{120, 40, 20, 255},
        Color{200, 80, 40, 255},
        Color{255, 120, 80, 255}
    };

    ensureBuffers();
    rebuildLUT();
}

RLOrderBookVis::~RLOrderBookVis() {
    cleanupTexture();
    cleanupMesh();
}

void RLOrderBookVis::cleanupTexture() {
    if (mTextureValid && mTexture.id != 0) {
        UnloadTexture(mTexture);
        mTexture = Texture2D{};
        mTextureValid = false;
    }
}

void RLOrderBookVis::cleanupMesh() {
    if (mMeshValid) {
        if (mBidMesh.vertexCount > 0) {
            UnloadMesh(mBidMesh);
            mBidMesh = Mesh{};
        }
        if (mAskMesh.vertexCount > 0) {
            UnloadMesh(mAskMesh);
            mAskMesh = Mesh{};
        }
        mMeshValid = false;
    }
}

void RLOrderBookVis::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
}

void RLOrderBookVis::setHistoryLength(size_t aLength) {
    if (aLength == mHistoryLength || aLength == 0) return;
    mHistoryLength = aLength;
    ensureBuffers();
    mTextureDirty = true;
    mMeshDirty = true;
}

void RLOrderBookVis::setPriceLevels(size_t aLevels) {
    if (aLevels == mPriceLevels || aLevels == 0) return;
    mPriceLevels = aLevels;
    ensureBuffers();
    mTextureDirty = true;
    mMeshDirty = true;
}

void RLOrderBookVis::setStyle(const RLOrderBookVisStyle& rStyle) {
    mStyle = rStyle;
}

void RLOrderBookVis::setPriceMode(RLOrderBookPriceMode aMode) {
    mPriceMode = aMode;
}

void RLOrderBookVis::setSpreadTicks(int aTicks) {
    mSpreadTicks = aTicks > 0 ? aTicks : 1;
}

void RLOrderBookVis::setPriceRange(float aMinPrice, float aMaxPrice) {
    if (aMaxPrice > aMinPrice) {
        mExplicitMinPrice = aMinPrice;
        mExplicitMaxPrice = aMaxPrice;
    }
}

void RLOrderBookVis::setBidColorStops(const std::vector<Color>& rStops) {
    if (rStops.size() >= 2) {
        mBidStops = rStops;
        mLutDirty = true;
    }
}

void RLOrderBookVis::setAskColorStops(const std::vector<Color>& rStops) {
    if (rStops.size() >= 2) {
        mAskStops = rStops;
        mLutDirty = true;
    }
}

void RLOrderBookVis::ensureBuffers() {
    size_t lTotal = mHistoryLength * mPriceLevels;

    mBidGrid.assign(lTotal, 0.0f);
    mAskGrid.assign(lTotal, 0.0f);

    // RGBA pixels: 2 * priceLevels height (bids on top, asks on bottom)
    // Actually, we'll interleave: each row has bid and ask at same price
    // Width = historyLength, Height = priceLevels
    mPixels.assign(mHistoryLength * mPriceLevels * 4, 0);

    mHead = 0;
    mSnapshotCount = 0;
    mMaxBidSize = 1.0f;
    mMaxAskSize = 1.0f;
    mCurrentMaxBid = 1.0f;
    mCurrentMaxAsk = 1.0f;

    cleanupTexture();
    cleanupMesh();
    mTextureDirty = true;
    mMeshDirty = true;
}

void RLOrderBookVis::rebuildLUT() {
    // Build bid LUT
    int lBidN = (int)mBidStops.size();
    if (lBidN >= 2) {
        for (int i = 0; i < 256; ++i) {
            float lT = (float)i / 255.0f;
            float lSegF = lT * (float)(lBidN - 1);
            int lSeg = (int)lSegF;
            if (lSeg >= lBidN - 1) lSeg = lBidN - 2;
            float lLocalT = lSegF - (float)lSeg;

            const Color& lA = mBidStops[(size_t)lSeg];
            const Color& lB = mBidStops[(size_t)lSeg + 1];

            mBidLut[i].r = (unsigned char)(lA.r + (int)((lB.r - lA.r) * lLocalT));
            mBidLut[i].g = (unsigned char)(lA.g + (int)((lB.g - lA.g) * lLocalT));
            mBidLut[i].b = (unsigned char)(lA.b + (int)((lB.b - lA.b) * lLocalT));
            mBidLut[i].a = (unsigned char)(lA.a + (int)((lB.a - lA.a) * lLocalT));
        }
    }

    // Build ask LUT
    int lAskN = (int)mAskStops.size();
    if (lAskN >= 2) {
        for (int i = 0; i < 256; ++i) {
            float lT = (float)i / 255.0f;
            float lSegF = lT * (float)(lAskN - 1);
            int lSeg = (int)lSegF;
            if (lSeg >= lAskN - 1) lSeg = lAskN - 2;
            float lLocalT = lSegF - (float)lSeg;

            const Color& lA = mAskStops[(size_t)lSeg];
            const Color& lB = mAskStops[(size_t)lSeg + 1];

            mAskLut[i].r = (unsigned char)(lA.r + (int)((lB.r - lA.r) * lLocalT));
            mAskLut[i].g = (unsigned char)(lA.g + (int)((lB.g - lA.g) * lLocalT));
            mAskLut[i].b = (unsigned char)(lA.b + (int)((lB.b - lA.b) * lLocalT));
            mAskLut[i].a = (unsigned char)(lA.a + (int)((lB.a - lA.a) * lLocalT));
        }
    }

    mLutDirty = false;
    mTextureDirty = true;
}

void RLOrderBookVis::clear() {
    std::fill(mBidGrid.begin(), mBidGrid.end(), 0.0f);
    std::fill(mAskGrid.begin(), mAskGrid.end(), 0.0f);
    mHead = 0;
    mSnapshotCount = 0;
    mMaxBidSize = 1.0f;
    mMaxAskSize = 1.0f;
    mCurrentMaxBid = 1.0f;
    mCurrentMaxAsk = 1.0f;
    mTextureDirty = true;
    mMeshDirty = true;
}

float RLOrderBookVis::priceToNormalized(float aPrice) const {
    float lRange = mCurrentMaxPrice - mCurrentMinPrice;
    if (lRange < 0.0001f) lRange = 0.0001f;
    return (aPrice - mCurrentMinPrice) / lRange;
}

float RLOrderBookVis::normalizedToPrice(float aNorm) const {
    return mCurrentMinPrice + aNorm * (mCurrentMaxPrice - mCurrentMinPrice);
}

int RLOrderBookVis::priceToGridRow(float aPrice) const {
    float lNorm = priceToNormalized(aPrice);
    // Flip: high prices at top (row 0), low prices at bottom
    int lRow = (int)((1.0f - lNorm) * (float)mPriceLevels);
    return RLCharts::clampIdx(lRow, (int)mPriceLevels);
}

size_t RLOrderBookVis::gridIndex(size_t aTimeIdx, size_t aPriceIdx) const {
    return aTimeIdx * mPriceLevels + aPriceIdx;
}

size_t RLOrderBookVis::ringTimeIndex(size_t aOffset) const {
    // Convert display offset (0 = oldest visible) to ring buffer index
    if (mSnapshotCount == 0) return 0;

    size_t lVisibleCount = mSnapshotCount < mHistoryLength ? mSnapshotCount : mHistoryLength;
    if (aOffset >= lVisibleCount) aOffset = lVisibleCount - 1;

    // Oldest visible snapshot
    size_t lOldest = (mHead + mHistoryLength - lVisibleCount) % mHistoryLength;
    return (lOldest + aOffset) % mHistoryLength;
}

void RLOrderBookVis::pushSnapshot(const RLOrderBookSnapshot& rSnapshot) {
    // Update current market state
    if (!rSnapshot.mBids.empty() && !rSnapshot.mAsks.empty()) {
        mCurrentBestBid = rSnapshot.mBids[0].first;
        mCurrentBestAsk = rSnapshot.mAsks[0].first;
        mCurrentMidPrice = (mCurrentBestBid + mCurrentBestAsk) * 0.5f;
        mCurrentSpread = mCurrentBestAsk - mCurrentBestBid;
    }

    // Determine price range based on mode
    float lMinPrice = mCurrentMinPrice;
    float lMaxPrice = mCurrentMaxPrice;

    switch (mPriceMode) {
        case RLOrderBookPriceMode::FullDepth: {
            // Use full range from all bids and asks
            lMinPrice = 1e30f;
            lMaxPrice = -1e30f;
            for (const auto& lBid : rSnapshot.mBids) {
                if (lBid.first < lMinPrice) lMinPrice = lBid.first;
                if (lBid.first > lMaxPrice) lMaxPrice = lBid.first;
            }
            for (const auto& lAsk : rSnapshot.mAsks) {
                if (lAsk.first < lMinPrice) lMinPrice = lAsk.first;
                if (lAsk.first > lMaxPrice) lMaxPrice = lAsk.first;
            }
            if (lMinPrice > lMaxPrice) {
                lMinPrice = mCurrentMidPrice - 1.0f;
                lMaxPrice = mCurrentMidPrice + 1.0f;
            }
            break;
        }
        case RLOrderBookPriceMode::SpreadTicks: {
            // Estimate tick size from spread or use small fraction
            float lTickSize = mCurrentSpread > 0.0001f ? mCurrentSpread : 0.01f;
            float lHalfRange = lTickSize * (float)mSpreadTicks;
            lMinPrice = mCurrentMidPrice - lHalfRange;
            lMaxPrice = mCurrentMidPrice + lHalfRange;
            break;
        }
        case RLOrderBookPriceMode::ExplicitRange: {
            lMinPrice = mExplicitMinPrice;
            lMaxPrice = mExplicitMaxPrice;
            break;
        }
    }

    // Smooth transition to new price range
    mTargetMinPrice = lMinPrice;
    mTargetMaxPrice = lMaxPrice;

    // Clear the column we're about to write
    size_t lColStart = mHead * mPriceLevels;
    for (size_t i = 0; i < mPriceLevels; ++i) {
        mBidGrid[lColStart + i] = 0.0f;
        mAskGrid[lColStart + i] = 0.0f;
    }

    // Track max sizes for scaling
    float lLocalMaxBid = 0.0f;
    float lLocalMaxAsk = 0.0f;

    // Write bids to grid
    for (const auto& lBid : rSnapshot.mBids) {
        float lPrice = lBid.first;
        float lSize = lBid.second;
        if (lPrice < mCurrentMinPrice || lPrice > mCurrentMaxPrice) continue;

        int lRow = priceToGridRow(lPrice);
        size_t lIdx = gridIndex(mHead, (size_t)lRow);
        mBidGrid[lIdx] += lSize;

        if (mBidGrid[lIdx] > lLocalMaxBid) lLocalMaxBid = mBidGrid[lIdx];
    }

    // Write asks to grid
    for (const auto& lAsk : rSnapshot.mAsks) {
        float lPrice = lAsk.first;
        float lSize = lAsk.second;
        if (lPrice < mCurrentMinPrice || lPrice > mCurrentMaxPrice) continue;

        int lRow = priceToGridRow(lPrice);
        size_t lIdx = gridIndex(mHead, (size_t)lRow);
        mAskGrid[lIdx] += lSize;

        if (mAskGrid[lIdx] > lLocalMaxAsk) lLocalMaxAsk = mAskGrid[lIdx];
    }

    // Update max sizes
    if (lLocalMaxBid > mMaxBidSize) mMaxBidSize = lLocalMaxBid;
    if (lLocalMaxAsk > mMaxAskSize) mMaxAskSize = lLocalMaxAsk;

    // Advance ring buffer
    mHead = (mHead + 1) % mHistoryLength;
    if (mSnapshotCount < mHistoryLength) {
        ++mSnapshotCount;
    }

    mTextureDirty = true;
    mMeshDirty = true;
}

void RLOrderBookVis::update(float aDt) {
    // Smooth price range transitions
    float lT = RLCharts::clamp01(mStyle.mScaleSpeed * aDt);
    mCurrentMinPrice = RLCharts::lerpF(mCurrentMinPrice, mTargetMinPrice, lT);
    mCurrentMaxPrice = RLCharts::lerpF(mCurrentMaxPrice, mTargetMaxPrice, lT);

    // Smooth intensity scale transitions
    mCurrentMaxBid = RLCharts::lerpF(mCurrentMaxBid, mMaxBidSize, lT);
    mCurrentMaxAsk = RLCharts::lerpF(mCurrentMaxAsk, mMaxAskSize, lT);

    // Decay max sizes slowly to adapt to changing conditions
    mMaxBidSize *= (1.0f - 0.1f * aDt);
    mMaxAskSize *= (1.0f - 0.1f * aDt);
    if (mMaxBidSize < 1.0f) mMaxBidSize = 1.0f;
    if (mMaxAskSize < 1.0f) mMaxAskSize = 1.0f;

    // Rebuild LUT if needed
    if (mLutDirty) {
        rebuildLUT();
    }

    // Update texture if dirty
    if (mTextureDirty) {
        rebuildTexture();
        updateTexturePixels();
        mTextureDirty = false;
    }

    // Update mesh if dirty (only when 3D is likely to be used)
    if (mMeshDirty) {
        rebuildMesh();
        updateMeshData();
        mMeshDirty = false;
    }
}

void RLOrderBookVis::rebuildTexture() {
    if (mTextureValid && mTexture.id != 0) return; // Already valid

    // Create texture from pixel buffer
    Image lImg = {0};
    lImg.data = mPixels.data();
    lImg.width = (int)mHistoryLength;
    lImg.height = (int)mPriceLevels;
    lImg.mipmaps = 1;
    lImg.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    mTexture = LoadTextureFromImage(lImg);
    SetTextureWrap(mTexture, TEXTURE_WRAP_CLAMP);
    SetTextureFilter(mTexture, TEXTURE_FILTER_BILINEAR);

    mTextureValid = (mTexture.id != 0);
}

void RLOrderBookVis::updateTexturePixels() {
    if (mSnapshotCount == 0) return;

    size_t lVisibleCount = mSnapshotCount < mHistoryLength ? mSnapshotCount : mHistoryLength;

    // Inverse max for scaling
    float lInvMaxBid = (mCurrentMaxBid > 0.001f) ? (1.0f / mCurrentMaxBid) : 1.0f;
    float lInvMaxAsk = (mCurrentMaxAsk > 0.001f) ? (1.0f / mCurrentMaxAsk) : 1.0f;
    float lIntensityMult = mStyle.mIntensityScale * 255.0f;

    // Convert grid to pixels
    // Pixel layout: row 0 = highest price, row (priceLevels-1) = lowest price
    // Column 0 = oldest snapshot, column (visibleCount-1) = newest

    uint32_t* pPixels = (uint32_t*)mPixels.data();

    for (size_t lTimeOffset = 0; lTimeOffset < mHistoryLength; ++lTimeOffset) {
        size_t lRingIdx = ringTimeIndex(lTimeOffset);

        for (size_t lPriceIdx = 0; lPriceIdx < mPriceLevels; ++lPriceIdx) {
            size_t lGridIdx = gridIndex(lRingIdx, lPriceIdx);
            size_t lPixelIdx = lPriceIdx * mHistoryLength + lTimeOffset;

            float lBidVal = mBidGrid[lGridIdx];
            float lAskVal = mAskGrid[lGridIdx];

            Color lColor;

            if (lBidVal > 0.0f && lAskVal > 0.0f) {
                // Both bid and ask at this level - blend colors
                float lBidIntensity = lBidVal * lInvMaxBid * lIntensityMult;
                float lAskIntensity = lAskVal * lInvMaxAsk * lIntensityMult;

                int lBidIdx = RLCharts::clampIdx((int)lBidIntensity, 256);
                int lAskIdx = RLCharts::clampIdx((int)lAskIntensity, 256);

                Color lBidColor = mBidLut[lBidIdx];
                Color lAskColor = mAskLut[lAskIdx];

                // Blend based on relative intensity
                float lTotal = lBidIntensity + lAskIntensity;
                float lBidWeight = lBidIntensity / (lTotal + 0.001f);

                lColor.r = (unsigned char)(lBidColor.r * lBidWeight + lAskColor.r * (1.0f - lBidWeight));
                lColor.g = (unsigned char)(lBidColor.g * lBidWeight + lAskColor.g * (1.0f - lBidWeight));
                lColor.b = (unsigned char)(lBidColor.b * lBidWeight + lAskColor.b * (1.0f - lBidWeight));
                lColor.a = (unsigned char)RLCharts::clamp((lBidColor.a + lAskColor.a) / 2, 0, 255);
            }
            else if (lBidVal > 0.0f) {
                float lIntensity = lBidVal * lInvMaxBid * lIntensityMult;
                int lIdx = RLCharts::clampIdx((int)lIntensity, 256);
                lColor = mBidLut[lIdx];
            }
            else if (lAskVal > 0.0f) {
                float lIntensity = lAskVal * lInvMaxAsk * lIntensityMult;
                int lIdx = RLCharts::clampIdx((int)lIntensity, 256);
                lColor = mAskLut[lIdx];
            }
            else {
                // Empty cell - use background-ish color
                lColor = mStyle.mBackground;
                lColor.a = 255;
            }

            pPixels[lPixelIdx] = *(uint32_t*)&lColor;
        }
    }

    if (mTextureValid && mTexture.id != 0) {
        UpdateTexture(mTexture, mPixels.data());
    }
}

void RLOrderBookVis::rebuildMesh() {
    cleanupMesh();

    // Create heightmap meshes for bids and asks
    // Each mesh is a grid of quads: (historyLength-1) x (priceLevels-1) quads
    // 6 vertices per quad (2 triangles)

    int lQuadsX = (int)mHistoryLength - 1;
    int lQuadsY = (int)mPriceLevels - 1;
    if (lQuadsX < 1 || lQuadsY < 1) return;

    int lVertexCount = lQuadsX * lQuadsY * 6;

    // Allocate mesh data for bid mesh
    mBidMesh.vertexCount = lVertexCount;
    mBidMesh.triangleCount = lQuadsX * lQuadsY * 2;
    mBidMesh.vertices = (float*)MemAlloc(lVertexCount * 3 * sizeof(float));
    mBidMesh.colors = (unsigned char*)MemAlloc(lVertexCount * 4 * sizeof(unsigned char));
    mBidMesh.normals = (float*)MemAlloc(lVertexCount * 3 * sizeof(float));

    // Allocate mesh data for ask mesh
    mAskMesh.vertexCount = lVertexCount;
    mAskMesh.triangleCount = lQuadsX * lQuadsY * 2;
    mAskMesh.vertices = (float*)MemAlloc(lVertexCount * 3 * sizeof(float));
    mAskMesh.colors = (unsigned char*)MemAlloc(lVertexCount * 4 * sizeof(unsigned char));
    mAskMesh.normals = (float*)MemAlloc(lVertexCount * 3 * sizeof(float));

    // Initialize normals to up vector
    for (int i = 0; i < lVertexCount; ++i) {
        mBidMesh.normals[i * 3 + 0] = 0.0f;
        mBidMesh.normals[i * 3 + 1] = 1.0f;
        mBidMesh.normals[i * 3 + 2] = 0.0f;

        mAskMesh.normals[i * 3 + 0] = 0.0f;
        mAskMesh.normals[i * 3 + 1] = 1.0f;
        mAskMesh.normals[i * 3 + 2] = 0.0f;
    }

    UploadMesh(&mBidMesh, true); // dynamic = true for updates
    UploadMesh(&mAskMesh, true);

    mMeshValid = true;
}

void RLOrderBookVis::updateMeshData() {
    if (!mMeshValid) return;

    int lQuadsX = (int)mHistoryLength - 1;
    int lQuadsY = (int)mPriceLevels - 1;
    if (lQuadsX < 1 || lQuadsY < 1) return;

    float lCellSize = mStyle.m3DCellSize;
    float lHeightScale = mStyle.mHeightScale;

    float lInvMaxBid = (mCurrentMaxBid > 0.001f) ? (1.0f / mCurrentMaxBid) : 1.0f;
    float lInvMaxAsk = (mCurrentMaxAsk > 0.001f) ? (1.0f / mCurrentMaxAsk) : 1.0f;

    // Helper to get height at a grid point
    auto getBidHeight = [&](size_t aTimeIdx, size_t aPriceIdx) -> float {
        size_t lRingIdx = ringTimeIndex(aTimeIdx);
        size_t lIdx = gridIndex(lRingIdx, aPriceIdx);
        return mBidGrid[lIdx] * lInvMaxBid * lHeightScale;
    };

    auto getAskHeight = [&](size_t aTimeIdx, size_t aPriceIdx) -> float {
        size_t lRingIdx = ringTimeIndex(aTimeIdx);
        size_t lIdx = gridIndex(lRingIdx, aPriceIdx);
        return mAskGrid[lIdx] * lInvMaxAsk * lHeightScale;
    };

    int lVertIdx = 0;

    for (int lQy = 0; lQy < lQuadsY; ++lQy) {
        for (int lQx = 0; lQx < lQuadsX; ++lQx) {
            // Quad corners (time, price)
            size_t lT0 = (size_t)lQx;
            size_t lT1 = (size_t)(lQx + 1);
            size_t lP0 = (size_t)lQy;
            size_t lP1 = (size_t)(lQy + 1);

            // World positions
            float lX0 = (float)lQx * lCellSize;
            float lX1 = (float)(lQx + 1) * lCellSize;
            float lZ0 = (float)lQy * lCellSize;
            float lZ1 = (float)(lQy + 1) * lCellSize;

            // Bid heights
            float lBh00 = getBidHeight(lT0, lP0);
            float lBh10 = getBidHeight(lT1, lP0);
            float lBh01 = getBidHeight(lT0, lP1);
            float lBh11 = getBidHeight(lT1, lP1);

            // Ask heights
            float lAh00 = getAskHeight(lT0, lP0);
            float lAh10 = getAskHeight(lT1, lP0);
            float lAh01 = getAskHeight(lT0, lP1);
            float lAh11 = getAskHeight(lT1, lP1);

            // Colors based on height
            auto bidColor = [&](float aH) -> Color {
                int lIdx = RLCharts::clampIdx((int)(aH / lHeightScale * 255.0f), 256);
                return mBidLut[lIdx];
            };
            auto askColor = [&](float aH) -> Color {
                int lIdx = RLCharts::clampIdx((int)(aH / lHeightScale * 255.0f), 256);
                return mAskLut[lIdx];
            };

            // Triangle 1: (0,0), (1,0), (0,1)
            // Triangle 2: (1,0), (1,1), (0,1)

            // Bid mesh vertices
            float* pBidVerts = mBidMesh.vertices + lVertIdx * 3;
            unsigned char* pBidColors = mBidMesh.colors + lVertIdx * 4;

            // Tri 1
            pBidVerts[0] = lX0; pBidVerts[1] = lBh00; pBidVerts[2] = lZ0;
            pBidVerts[3] = lX1; pBidVerts[4] = lBh10; pBidVerts[5] = lZ0;
            pBidVerts[6] = lX0; pBidVerts[7] = lBh01; pBidVerts[8] = lZ1;
            // Tri 2
            pBidVerts[9] = lX1; pBidVerts[10] = lBh10; pBidVerts[11] = lZ0;
            pBidVerts[12] = lX1; pBidVerts[13] = lBh11; pBidVerts[14] = lZ1;
            pBidVerts[15] = lX0; pBidVerts[16] = lBh01; pBidVerts[17] = lZ1;

            Color lBc00 = bidColor(lBh00);
            Color lBc10 = bidColor(lBh10);
            Color lBc01 = bidColor(lBh01);
            Color lBc11 = bidColor(lBh11);

            pBidColors[0] = lBc00.r; pBidColors[1] = lBc00.g; pBidColors[2] = lBc00.b; pBidColors[3] = lBc00.a;
            pBidColors[4] = lBc10.r; pBidColors[5] = lBc10.g; pBidColors[6] = lBc10.b; pBidColors[7] = lBc10.a;
            pBidColors[8] = lBc01.r; pBidColors[9] = lBc01.g; pBidColors[10] = lBc01.b; pBidColors[11] = lBc01.a;
            pBidColors[12] = lBc10.r; pBidColors[13] = lBc10.g; pBidColors[14] = lBc10.b; pBidColors[15] = lBc10.a;
            pBidColors[16] = lBc11.r; pBidColors[17] = lBc11.g; pBidColors[18] = lBc11.b; pBidColors[19] = lBc11.a;
            pBidColors[20] = lBc01.r; pBidColors[21] = lBc01.g; pBidColors[22] = lBc01.b; pBidColors[23] = lBc01.a;

            // Ask mesh vertices (offset in Z to separate from bids)
            float* pAskVerts = mAskMesh.vertices + lVertIdx * 3;
            unsigned char* pAskColors = mAskMesh.colors + lVertIdx * 4;

            pAskVerts[0] = lX0; pAskVerts[1] = lAh00; pAskVerts[2] = lZ0;
            pAskVerts[3] = lX1; pAskVerts[4] = lAh10; pAskVerts[5] = lZ0;
            pAskVerts[6] = lX0; pAskVerts[7] = lAh01; pAskVerts[8] = lZ1;
            pAskVerts[9] = lX1; pAskVerts[10] = lAh10; pAskVerts[11] = lZ0;
            pAskVerts[12] = lX1; pAskVerts[13] = lAh11; pAskVerts[14] = lZ1;
            pAskVerts[15] = lX0; pAskVerts[16] = lAh01; pAskVerts[17] = lZ1;

            Color lAc00 = askColor(lAh00);
            Color lAc10 = askColor(lAh10);
            Color lAc01 = askColor(lAh01);
            Color lAc11 = askColor(lAh11);

            pAskColors[0] = lAc00.r; pAskColors[1] = lAc00.g; pAskColors[2] = lAc00.b; pAskColors[3] = lAc00.a;
            pAskColors[4] = lAc10.r; pAskColors[5] = lAc10.g; pAskColors[6] = lAc10.b; pAskColors[7] = lAc10.a;
            pAskColors[8] = lAc01.r; pAskColors[9] = lAc01.g; pAskColors[10] = lAc01.b; pAskColors[11] = lAc01.a;
            pAskColors[12] = lAc10.r; pAskColors[13] = lAc10.g; pAskColors[14] = lAc10.b; pAskColors[15] = lAc10.a;
            pAskColors[16] = lAc11.r; pAskColors[17] = lAc11.g; pAskColors[18] = lAc11.b; pAskColors[19] = lAc11.a;
            pAskColors[20] = lAc01.r; pAskColors[21] = lAc01.g; pAskColors[22] = lAc01.b; pAskColors[23] = lAc01.a;

            lVertIdx += 6;
        }
    }

    // Update GPU buffers
    UpdateMeshBuffer(mBidMesh, 0, mBidMesh.vertices, mBidMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(mBidMesh, 3, mBidMesh.colors, mBidMesh.vertexCount * 4 * sizeof(unsigned char), 0);

    UpdateMeshBuffer(mAskMesh, 0, mAskMesh.vertices, mAskMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(mAskMesh, 3, mAskMesh.colors, mAskMesh.vertexCount * 4 * sizeof(unsigned char), 0);
}

Rectangle RLOrderBookVis::getPlotArea() const {
    float lPad = mStyle.mPadding;
    return Rectangle{
        mBounds.x + lPad,
        mBounds.y + lPad,
        mBounds.width - 2.0f * lPad,
        mBounds.height - 2.0f * lPad
    };
}

void RLOrderBookVis::drawBackground() const {
    DrawRectangleRec(mBounds, mStyle.mBackground);

    if (mStyle.mShowBorder) {
        DrawRectangleLinesEx(mBounds, mStyle.mBorderThickness, mStyle.mBorderColor);
    }
}

void RLOrderBookVis::drawGrid2D() const {
    if (!mStyle.mShowGrid) return;

    Rectangle lPlot = getPlotArea();

    // Vertical lines (time axis)
    for (int i = 0; i <= mStyle.mGridLinesX; ++i) {
        float lX = lPlot.x + lPlot.width * (float)i / (float)mStyle.mGridLinesX;
        DrawLineV(
            Vector2{lX, lPlot.y},
            Vector2{lX, lPlot.y + lPlot.height},
            mStyle.mGridColor
        );
    }

    // Horizontal lines (price axis)
    for (int i = 0; i <= mStyle.mGridLinesY; ++i) {
        float lY = lPlot.y + lPlot.height * (float)i / (float)mStyle.mGridLinesY;
        DrawLineV(
            Vector2{lPlot.x, lY},
            Vector2{lPlot.x + lPlot.width, lY},
            mStyle.mGridColor
        );
    }
}

void RLOrderBookVis::drawMidLine2D() const {
    if (!mStyle.mShowMidLine && !mStyle.mShowSpreadArea) return;

    Rectangle lPlot = getPlotArea();

    // Normalize mid price to plot Y
    float lMidNorm = priceToNormalized(mCurrentMidPrice);
    float lMidY = lPlot.y + (1.0f - lMidNorm) * lPlot.height;

    if (mStyle.mShowSpreadArea) {
        float lBidNorm = priceToNormalized(mCurrentBestBid);
        float lAskNorm = priceToNormalized(mCurrentBestAsk);

        float lBidY = lPlot.y + (1.0f - lBidNorm) * lPlot.height;
        float lAskY = lPlot.y + (1.0f - lAskNorm) * lPlot.height;

        float lSpreadHeight = lBidY - lAskY;
        if (lSpreadHeight > 0) {
            DrawRectangle(
                (int)lPlot.x, (int)lAskY,
                (int)lPlot.width, (int)lSpreadHeight,
                mStyle.mSpreadAreaColor
            );
        }
    }

    if (mStyle.mShowMidLine) {
        DrawLineEx(
            Vector2{lPlot.x, lMidY},
            Vector2{lPlot.x + lPlot.width, lMidY},
            mStyle.mMidLineThickness,
            mStyle.mMidLineColor
        );
    }
}

void RLOrderBookVis::drawHeatmap2D() const {
    if (!mTextureValid || mTexture.id == 0) return;
    if (mSnapshotCount == 0) return;

    Rectangle lPlot = getPlotArea();

    // Source rectangle (full texture)
    Rectangle lSrc = {0, 0, (float)mHistoryLength, (float)mPriceLevels};

    // Destination rectangle (plot area)
    Rectangle lDst = lPlot;

    DrawTexturePro(mTexture, lSrc, lDst, Vector2{0, 0}, 0.0f, WHITE);
}

void RLOrderBookVis::draw2D() const {
    drawBackground();
    drawGrid2D();
    drawHeatmap2D();
    drawMidLine2D();
}

void RLOrderBookVis::draw3D(const Camera3D& rCamera) const {
    if (!mMeshValid) return;
    if (mSnapshotCount == 0) return;

    BeginMode3D(rCamera);

    // Center the mesh
    float lOffsetX = -(float)mHistoryLength * mStyle.m3DCellSize * 0.5f;
    float lOffsetZ = -(float)mPriceLevels * mStyle.m3DCellSize * 0.5f;

    Matrix lTransform = MatrixTranslate(lOffsetX, 0.0f, lOffsetZ);

    // Draw floor grid
    if (mStyle.mShow3DGrid) {
        float lGridSize = (float)mHistoryLength * mStyle.m3DCellSize;
        float lGridSizeZ = (float)mPriceLevels * mStyle.m3DCellSize;
        int lGridDivs = 10;

        for (int i = 0; i <= lGridDivs; ++i) {
            float lT = (float)i / (float)lGridDivs;
            // Time axis lines
            DrawLine3D(
                Vector3{lOffsetX + lT * lGridSize, 0.0f, lOffsetZ},
                Vector3{lOffsetX + lT * lGridSize, 0.0f, lOffsetZ + lGridSizeZ},
                mStyle.m3DGridColor
            );
            // Price axis lines
            DrawLine3D(
                Vector3{lOffsetX, 0.0f, lOffsetZ + lT * lGridSizeZ},
                Vector3{lOffsetX + lGridSize, 0.0f, lOffsetZ + lT * lGridSizeZ},
                mStyle.m3DGridColor
            );
        }
    }

    // Draw bid and ask meshes
    // Use a simple material with vertex colors
    Material lMat = LoadMaterialDefault();
    lMat.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

    DrawMesh(mBidMesh, lMat, lTransform);
    DrawMesh(mAskMesh, lMat, lTransform);

    EndMode3D();
}

