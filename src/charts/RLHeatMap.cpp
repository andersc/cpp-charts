#include "RLHeatMap.h"
#include "RLCommon.h"
#include <cmath>
#include <algorithm>
#include <cstring>


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
    if (!pPoints || aCount == 0) return;

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
    const float halfW = mCellsX * 0.5f;
    const float halfH = mCellsY * 0.5f;
    const int stride = mCellsX;

    float currentMax = mMaxValue;

    // Note: We cannot easily OpenMP this loop because of race conditions on mCounts[idx]++
    // unless we use atomic adds, which might be slower than serial for dense clusters.
    for (size_t i = 0; i < aCount; ++i){
        const Vector2& p = pPoints[i];

        // Bounds check (assuming input is normalized -1 to 1)
        if (p.x < -1.0f || p.x > 1.0f || p.y < -1.0f || p.y > 1.0f) continue;

        // Coordinate math optimized
        // Y is flipped: input +1 (top) maps to index 0.
        // y_norm = (p.y + 1) * 0.5. y_flip = 1.0 - y_norm.
        // y_idx = y_flip * Height = (1.0 - (0.5*p.y + 0.5)) * H = (0.5 - 0.5*p.y) * H
        // = 0.5*H - p.y*0.5*H

        int ix = (int)(p.x * halfW + halfW);
        int iy = (int)(halfH - p.y * halfH);

        // Fast clamp to ensure we stay in grid memory
        ix = RLCharts::clampIdx(ix, mCellsX);
        iy = RLCharts::clampIdx(iy, mCellsY);

        size_t idx = (size_t)iy * stride + ix;

        float val = mCounts[idx] + 1.0f;
        mCounts[idx] = val;

        // Track max value locally to minimize memory writes
        if (val > currentMax) currentMax = val;
    }

    mMaxValue = currentMax;
    mCountsDirty = true;
}

void RLHeatMap::update(float aDt){
    // 1. Handle Decay
    if (mMode == RLHeatMapUpdateMode::Decay && mDecayHalfLife > 0.0f){
        float lFactor = powf(0.5f, aDt / mDecayHalfLife);
        float lNewMax = 0.0f;
        const size_t count = mCounts.size();

        #ifdef _OPENMP
        #pragma omp parallel for reduction(max:lNewMax)
        #endif
        for (size_t i = 0; i < count; ++i){
            float v = mCounts[i] * lFactor;
            // Threshold to zero
            if (v < 1e-4f) v = 0.0f;
            mCounts[i] = v;
            if (v > lNewMax) lNewMax = v;
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
        Rectangle src = {0, 0, (float)mCellsX, (float)mCellsY};
        Rectangle dst = mBounds;
        Vector2 origin = {0,0};
        DrawTexturePro(mTexture, src, dst, origin, 0.0f, WHITE);
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

    size_t total = (size_t)mCellsX * (size_t)mCellsY;
    mCounts.assign(total, 0.0f);

    // Pixel buffer size (4 bytes per pixel)
    mPixels.assign(total * 4, 0);

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
    int lN = (int)mStops.size();
    if (lN < 2) return; // Should not happen due to check in ensure/setColor

    for (int i = 0; i < 256; ++i){
        float t = i / 255.0f;
        float segF = t * (float)(lN - 1);
        int seg = (int)segF;
        if (seg >= lN - 1) seg = lN - 2;

        float lt = segF - (float)seg;

        const Color& a = mStops[seg];
        const Color& b = mStops[seg + 1];

        // Fast integer lerp
        mLut[i].r = (unsigned char)(a.r + (int)((b.r - a.r) * lt));
        mLut[i].g = (unsigned char)(a.g + (int)((b.g - a.g) * lt));
        mLut[i].b = (unsigned char)(a.b + (int)((b.b - a.b) * lt));
        mLut[i].a = (unsigned char)(a.a + (int)((b.a - a.a) * lt));
    }
    mLutDirty = false;
}

void RLHeatMap::rebuildTextureIfNeeded(){
    if (mTextureValid && mTexture.id != 0) return;

    Image img = {};
    img.data = mPixels.data();
    img.width = mCellsX;
    img.height = mCellsY;
    img.mipmaps = 1;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    mTexture = LoadTextureFromImage(img);

    // CRITICAL FIX: Set texture wrap to clamp.
    // Default is usually Repeat, which causes edge pixels to blend
    // with the opposite side, looking like "wrong calculations".
    SetTextureWrap(mTexture, TEXTURE_WRAP_CLAMP);

    mTextureValid = (mTexture.id != 0);
}

void RLHeatMap::updateTexturePixels(){
    const size_t totalPixels = (size_t)mCellsX * (size_t)mCellsY;

    // Use raw pointers for speed
    const float* pCounts = mCounts.data();

    // Cast to uint32_t to write 4 bytes (RGBA) in one instruction
    // Raylib Color is r,g,b,a in memory (struct), usually packed.
    // WARNING: This assumes Little Endian for exact byte order if constructing integer manually,
    // but here we just copy from Color struct which is safe if we copy the struct.
    // However, writing to uint32_t* is the standard optimization for pixel buffers.
    uint32_t* pPixels32 = (uint32_t*)mPixels.data();

    // Avoid division in the loop
    const float invMax = (mMaxValue > 1e-6f) ? (1.0f / mMaxValue) : 1.0f;
    const float scale255 = 255.0f;

    // Use OpenMP if available for massive speedup on large grids
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (size_t i = 0; i < totalPixels; ++i){
        float v = pCounts[i];

        // Fast index calculation without branching 'clampi' or 'clamp01f' overhead
        // v * invMax is roughly 0..1.
        int idx = (int)(v * invMax * scale255);

        // Branchless clamping to 0..255
        if (idx > 255) idx = 255;
        // if (idx < 0) idx = 0; // v is always >= 0, so this is unnecessary

        Color c = mLut[idx];

        // Reinterpret cast hack to write 4 bytes at once
        // (Color struct is 4 bytes: r,g,b,a)
        pPixels32[i] = *(uint32_t*)&c;
    }

    if (mTextureValid && mTexture.id != 0){
        UpdateTexture(mTexture, mPixels.data());
    }
}