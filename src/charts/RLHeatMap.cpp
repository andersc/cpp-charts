#include "RLHeatMap.h"
#include "RLCommon.h"
#include <cmath>
#include <algorithm>


RLHeatMap::RLHeatMap(Rectangle aBounds, int aCellsX, int aCellsY)
    : mBounds(aBounds)
{
    if (aCellsX < 1) aCellsX = 1;
    if (aCellsY < 1) aCellsY = 1;

    // Default stops
    mStops = {
        Color{0, 0, 40, 255},
        Color{0, 180, 255, 255},
        Color{255, 60, 0, 255}
    };

    ensureGrid(aCellsX, aCellsY);
}

RLHeatMap::~RLHeatMap(){
    if (mTextureValid && mTexture.id != 0){
        UnloadTexture(mTexture);
        mTexture = Texture2D{};
        mTextureValid = false;
    }
}

void RLHeatMap::setBounds(Rectangle aBounds){ mBounds = aBounds; }

void RLHeatMap::setGrid(int aCellsX, int aCellsY){ ensureGrid(aCellsX, aCellsY); }

void RLHeatMap::setUpdateMode(RLHeatMapUpdateMode aMode){ mMode = aMode; }

void RLHeatMap::setDecayHalfLifeSeconds(float aSeconds){ mDecayHalfLife = aSeconds; }

void RLHeatMap::setStyle(const RLHeatMapStyle &rStyle){ mStyle = rStyle; }

void RLHeatMap::setColorStops(const std::vector<Color> &rStops){
    if (rStops.size() < 2) return;
    mStops = rStops;
    mLutDirty = true;
}

void RLHeatMap::clear(){
    std::fill(mCounts.begin(), mCounts.end(), 0.0f);
    mMaxValue = 1.0f;
    mCountsDirty = true;
}

void RLHeatMap::addPoints(const Vector2 *pPoints, size_t aCount){
    if (pPoints == nullptr || aCount == 0) return;

    if (mMode == RLHeatMapUpdateMode::Replace){
        // Replace mode: clear first, then add.
        // Note: If you call addPoints multiple times per frame in Replace mode,
        // this will wipe previous calls. Usually Replace implies "Set Data".
        std::fill(mCounts.begin(), mCounts.end(), 0.0f);
        mMaxValue = 1.0f;
    }

    // Optimization: Pre-calculate scaling factors to avoid (p + 1) * 0.5 inside loop
    // Original: (x + 1) * 0.5 * Width
    // Optimized: x * (0.5 * Width) + (0.5 * Width)
    const float lHalfW = (float)mCellsX * 0.5f;
    const float lHalfH = (float)mCellsY * 0.5f;
    const int lStride = mCellsX;

    float lCurrentMax = mMaxValue;

    // Note: We cannot easily OpenMP this loop because of race conditions on mCounts[idx]++
    // unless we use atomic adds, which might be slower than serial for dense clusters.
    for (size_t i = 0; i < aCount; ++i){
        const Vector2& rP = pPoints[i];

        // Bounds check (assuming input is normalized -1 to 1)
        if (rP.x < -1.0f || rP.x > 1.0f || rP.y < -1.0f || rP.y > 1.0f) continue;

        // Coordinate math optimized
        // Y is flipped: input +1 (top) maps to index 0.
        // y_norm = (p.y + 1) * 0.5. y_flip = 1.0 - y_norm.
        // y_idx = y_flip * Height = (1.0 - (0.5*p.y + 0.5)) * H = (0.5 - 0.5*p.y) * H
        // = 0.5*H - p.y*0.5*H

        int lIx = (int)(rP.x * lHalfW + lHalfW);
        int lIy = (int)(lHalfH - rP.y * lHalfH);

        // Fast clamp to ensure we stay in grid memory
        lIx = RLCharts::clampIdx(lIx, mCellsX);
        lIy = RLCharts::clampIdx(lIy, mCellsY);

        size_t lIdx = (size_t)lIy * lStride + lIx;

        float lVal = mCounts[lIdx] + 1.0f;
        mCounts[lIdx] = lVal;

        // Track max value locally to minimize memory writes
        if (lVal > lCurrentMax) lCurrentMax = lVal;
    }

    mMaxValue = lCurrentMax;
    mCountsDirty = true;
}

void RLHeatMap::update(float aDt){
    // 1. Handle Decay
    if (mMode == RLHeatMapUpdateMode::Decay && mDecayHalfLife > 0.0f){
        float lDecayFactor = powf(0.5f, aDt / mDecayHalfLife);
        float lNewMax = 0.0f;
        const size_t lCount = mCounts.size();

        #ifdef _OPENMP
        #pragma omp parallel for reduction(max:lNewMax)
        #endif
        for (size_t i = 0; i < lCount; ++i){
            float lV = mCounts[i] * lDecayFactor;
            // Threshold to zero
            if (lV < 1e-4f) lV = 0.0f;
            mCounts[i] = lV;
            if (lV > lNewMax) lNewMax = lV;
        }
        mMaxValue = std::max(lNewMax, 1.0f);
        mCountsDirty = true;
    }

    // 2. Handle Texture Updates
    if (mLutDirty) rebuildLUT();

    if (mCountsDirty){
        rebuildTextureIfNeeded();
        updateTexturePixels();
        mCountsDirty = false;
    }
}

void RLHeatMap::draw() const{
    if (mStyle.mShowBackground){
        DrawRectangleRec(mBounds, mStyle.mBackground);
    }

    if (mTextureValid && mTexture.id != 0){
        Rectangle lSrc = {0, 0, (float)mCellsX, (float)mCellsY};
        Rectangle lDst = mBounds;
        Vector2 lOrigin = {0, 0};
        DrawTexturePro(mTexture, lSrc, lDst, lOrigin, 0.0f, WHITE);
    }

    if (mStyle.mShowBorder){
        DrawRectangleLinesEx(mBounds, mStyle.mBorderThickness, mStyle.mBorderColor);
    }
}

void RLHeatMap::ensureGrid(int aCellsX, int aCellsY){
    if (aCellsX < 1) aCellsX = 1;
    if (aCellsY < 1) aCellsY = 1;

    if (aCellsX == mCellsX && aCellsY == mCellsY && !mCounts.empty()) return;

    mCellsX = aCellsX;
    mCellsY = aCellsY;

    size_t lTotal = (size_t)mCellsX * (size_t)mCellsY;
    mCounts.assign(lTotal, 0.0f);

    // Pixel buffer size (4 bytes per pixel)
    mPixels.assign(lTotal * 4, 0);

    mMaxValue = 1.0f;
    mCountsDirty = true;

    // Force texture recreation
    if (mTextureValid && mTexture.id != 0){
        UnloadTexture(mTexture);
        mTexture = Texture2D{};
        mTextureValid = false;
    }
}

void RLHeatMap::rebuildLUT(){
    size_t lN = mStops.size();
    if (lN < 2) return; // Should not happen due to check in ensure/setColor

    for (int i = 0; i < 256; ++i){
        float lT = (float)i / 255.0f;
        float lSegF = lT * (float)(lN - 1);
        auto lSeg = (size_t)lSegF;
        if (lSeg >= lN - 1) lSeg = lN - 2;

        float lLerp = lSegF - (float)lSeg;

        const Color& rA = mStops[lSeg];
        const Color& rB = mStops[lSeg + 1];

        // Fast lerp: compute as float, then cast to unsigned char
        mLut[i].r = (unsigned char)((float)rA.r + ((float)rB.r - (float)rA.r) * lLerp);
        mLut[i].g = (unsigned char)((float)rA.g + ((float)rB.g - (float)rA.g) * lLerp);
        mLut[i].b = (unsigned char)((float)rA.b + ((float)rB.b - (float)rA.b) * lLerp);
        mLut[i].a = (unsigned char)((float)rA.a + ((float)rB.a - (float)rA.a) * lLerp);
    }
    mLutDirty = false;
}

void RLHeatMap::rebuildTextureIfNeeded(){
    if (mTextureValid && mTexture.id != 0) return;

    Image lImg = {};
    lImg.data = mPixels.data();
    lImg.width = mCellsX;
    lImg.height = mCellsY;
    lImg.mipmaps = 1;
    lImg.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    mTexture = LoadTextureFromImage(lImg);

    // CRITICAL FIX: Set texture wrap to clamp.
    // Default is usually Repeat, which causes edge pixels to blend
    // with the opposite side, looking like "wrong calculations".
    SetTextureWrap(mTexture, TEXTURE_WRAP_CLAMP);

    mTextureValid = (mTexture.id != 0);
}

void RLHeatMap::updateTexturePixels(){
    const size_t lTotalPixels = (size_t)mCellsX * (size_t)mCellsY;

    // Use raw pointers for speed
    const float* pCounts = mCounts.data();

    // Cast to uint32_t to write 4 bytes (RGBA) in one instruction
    // Raylib Color is r,g,b,a in memory (struct), usually packed.
    // WARNING: This assumes Little Endian for exact byte order if constructing integer manually,
    // but here we just copy from Color struct which is safe if we copy the struct.
    // However, writing to uint32_t* is the standard optimization for pixel buffers.
    auto pPixels32 = (uint32_t*)mPixels.data();

    // Avoid division in the loop
    const float lInvMax = (mMaxValue > 1e-6f) ? (1.0f / mMaxValue) : 1.0f;
    const float SCALE_255 = 255.0f;

    // Use OpenMP if available for massive speedup on large grids
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (size_t i = 0; i < lTotalPixels; ++i){
        float lV = pCounts[i];

        // Fast index calculation without branching 'clampi' or 'clamp01f' overhead
        // v * invMax is roughly 0..1.
        int lIdx = (int)(lV * lInvMax * SCALE_255);

        // Branchless clamping to 0..255
        if (lIdx > 255) lIdx = 255;
        // if (lIdx < 0) lIdx = 0; // v is always >= 0, so this is unnecessary

        Color lC = mLut[lIdx];

        // Reinterpret cast hack to write 4 bytes at once
        // (Color struct is 4 bytes: r,g,b,a)
        pPixels32[i] = *(uint32_t*)&lC;
    }

    if (mTextureValid && mTexture.id != 0){
        UpdateTexture(mTexture, mPixels.data());
    }
}