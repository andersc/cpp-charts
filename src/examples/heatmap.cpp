#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "RLHeatMap.h"

// --- Optimization: Fast PRNG (Xorshift) ---
// std::rand() can be slow and low quality. This is much faster.
static uint32_t sRngState = 123456789;

static inline void seed_fast(uint32_t seed) {
    if (seed == 0) seed = 123456789;
    sRngState = seed;
}

static inline uint32_t rand_fast() {
    uint32_t x = sRngState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return sRngState = x;
}

// Returns float in [0, 1]
static inline float lfrand_fast() {
    // 0x7FFFFF is 23 bits of mantissa
    // This generates a float directly from bits, faster than division
    union { uint32_t i; float f; } u;
    u.i = 0x3F800000 | (rand_fast() & 0x7FFFFF);
    return u.f - 1.0f;
}

// Box-Muller transform for normal distribution (mean 0, stddev 1)
static inline float lrandNormal(){
    // Optimization: fast rand
    float lU1 = fmaxf(lfrand_fast(), 1e-6f);
    float lU2 = lfrand_fast();
    // Note: Log and Sqrt are still heavy, but unavoidable for high quality normal dist
    // without using a Ziggurat lookup table.
    return sqrtf(-2.0f * logf(lU1)) * cosf(6.2831853f * lU2);
}

enum class HeatMode { Uniform, MovingEmitters };

struct Emitter { Vector2 mPos; Vector2 mVel; float mSpread; };

static void lInitEmitters(std::vector<Emitter> &rEmitters, int aCount){
    rEmitters.resize((size_t)aCount);
    for (int i=0;i<aCount;++i){
        float lX = lfrand_fast()*2.0f - 1.0f;
        float lY = lfrand_fast()*2.0f - 1.0f;
        float lAng = lfrand_fast() * 6.2831853f;
        float lSpd = 0.15f + lfrand_fast()*0.35f;
        rEmitters[(size_t)i].mPos = Vector2{lX, lY};
        rEmitters[(size_t)i].mVel = Vector2{cosf(lAng)*lSpd, sinf(lAng)*lSpd};
        rEmitters[(size_t)i].mSpread = 0.10f + lfrand_fast()*0.22f;
    }
}

static void lUpdateEmitters(std::vector<Emitter> &rEmitters, float aDt){
    for (size_t i=0;i<rEmitters.size(); ++i){
        Emitter &rE = rEmitters[i];
        float lJ = (lfrand_fast()*2.0f - 1.0f) * 0.25f;
        float lAng = atan2f(rE.mVel.y, rE.mVel.x) + lJ * aDt;
        float lSpd = sqrtf(rE.mVel.x*rE.mVel.x + rE.mVel.y*rE.mVel.y);
        rE.mVel.x = cosf(lAng) * lSpd;
        rE.mVel.y = sinf(lAng) * lSpd;

        rE.mPos.x += rE.mVel.x * aDt;
        rE.mPos.y += rE.mVel.y * aDt;

        if (rE.mPos.x < -1.0f) rE.mPos.x += 2.0f; else if (rE.mPos.x > 1.0f) rE.mPos.x -= 2.0f;
        if (rE.mPos.y < -1.0f) rE.mPos.y += 2.0f; else if (rE.mPos.y > 1.0f) rE.mPos.y -= 2.0f;

        rE.mSpread += (lfrand_fast()*2.0f - 1.0f) * 0.1f * aDt;
        if (rE.mSpread < 0.06f) rE.mSpread = 0.06f; else if (rE.mSpread > 0.35f) rE.mSpread = 0.35f;
    }
}

static void GenPointsUniform(std::vector<Vector2> &rBuf, size_t aCount){
    if (rBuf.size() != aCount) rBuf.resize(aCount);
    // Vector resize is cheap if capacity is sufficient, but checking size explicitly doesn't hurt.

    // Pointer access is slightly faster than iterator/operator[] in debug builds
    Vector2* pData = rBuf.data();
    for (size_t i=0;i<aCount;++i){
        float lX = lfrand_fast()*2.0f - 1.0f;
        float lY = lfrand_fast()*2.0f - 1.0f;
        pData[i].x = lX;
        pData[i].y = lY;
    }
}

static void GenPointsEmitters(std::vector<Vector2> &rBuf, size_t aCount, const std::vector<Emitter> &rEmitters){
    if (rBuf.size() != aCount) rBuf.resize(aCount);
    size_t lECount = rEmitters.size();
    if (lECount == 0){ GenPointsUniform(rBuf, aCount); return; }

    Vector2* pData = rBuf.data();
    for (size_t i=0;i<aCount;++i){
        // Use bitwise mod if lECount was power of 2, but here we just use modulo
        const Emitter &rE = rEmitters[i % lECount];
        float lX = rE.mPos.x + lrandNormal() * rE.mSpread;
        float lY = rE.mPos.y + lrandNormal() * rE.mSpread;

        // --- FIX FOR VISUAL ARTIFACT ---
        // Originally, this code clamped lX/lY to [-1, 1].
        // This caused all "out of bounds" points to stack up exactly on the border pixels,
        // creating solid bright lines (walls) around the heatmap.
        // By removing the clamp, we let them remain "out of bounds".
        // The RLHeatMap class will simply ignore them, which is the correct behavior.

        /* if (lX < -1.0f) lX = -1.0f; else if (lX > 1.0f) lX = 1.0f;
        if (lY < -1.0f) lY = -1.0f; else if (lY > 1.0f) lY = 1.0f;
        */

        pData[i].x = lX;
        pData[i].y = lY;
    }
}

int main(){
    seed_fast((uint32_t)time(nullptr)); // Seed our fast PRNG

    const int lW = 1400;
    const int lH = 820;
    InitWindow(lW, lH, "raylib heat map - RLHeatMap demo");
    SetTargetFPS(120);

    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    // Layout
    float lPad = 34.0f;
    float lTop = 60.0f;
    float lWidth = (lW - lPad*4) / 3.0f;
    float lHeight = lH - lTop - lPad*2;
    Rectangle lA{ lPad, lTop, lWidth, lHeight };
    Rectangle lB{ lA.x + lWidth + lPad, lTop, lWidth, lHeight };
    Rectangle lC{ lB.x + lWidth + lPad, lTop, lWidth, lHeight };

    // Heatmaps
    RLHeatMap lHM_Acc(lA, 160, 90);
    RLHeatMap lHM_Repl(lB, 96, 54);
    RLHeatMap lHM_Decay(lC, 256, 144);

    RLHeatMapStyle lStyle;
    lStyle.mBackground = Color{24,26,32,255};
    lStyle.mShowBorder = true;
    lStyle.mBorderColor = Color{54,58,66,255};
    lStyle.mBorderThickness = 2.0f;
    lHM_Acc.setStyle(lStyle);
    lHM_Repl.setStyle(lStyle);
    lHM_Decay.setStyle(lStyle);

    // Colors
    std::vector<Color> lStops3 = {
        Color{0, 0, 40, 255},
        Color{0, 180, 255, 255},
        Color{255, 60, 0, 255}
    };
    std::vector<Color> lStops4 = {
        Color{0, 0, 40, 255},
        Color{0, 180, 255, 255},
        Color{255, 220, 0, 255},
        Color{255, 60, 0, 255}
    };
    lHM_Acc.setColorStops(lStops3);
    lHM_Repl.setColorStops(lStops4);
    lHM_Decay.setColorStops(lStops4);

    // Modes
    lHM_Acc.setUpdateMode(RLHeatMapUpdateMode::Accumulate);
    lHM_Repl.setUpdateMode(RLHeatMapUpdateMode::Replace);
    lHM_Decay.setUpdateMode(RLHeatMapUpdateMode::Decay);
    lHM_Decay.setDecayHalfLifeSeconds(1.2f);

    std::vector<Vector2> lPoints;
    // Increased default batch size since we are faster now
    size_t lBatch = 120000;

    bool lPause = false;
    HeatMode lMode = HeatMode::MovingEmitters;
    std::vector<Emitter> lEmitters;
    int lEmitterCount = 6;
    lInitEmitters(lEmitters, lEmitterCount);

    while (!WindowShouldClose()){
        float lDt = GetFrameTime();

        // Controls
        if (IsKeyPressed(KEY_SPACE)) lPause = !lPause;
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)){
            if (lBatch > 1000) lBatch -= 1000;
        }
        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)){
            lBatch += 1000;
        }
        if (IsKeyPressed(KEY_ONE)) lHM_Acc.setGrid(96, 54);
        if (IsKeyPressed(KEY_TWO)) lHM_Acc.setGrid(160, 90);
        if (IsKeyPressed(KEY_THREE)) lHM_Acc.setGrid(256, 144);
        if (IsKeyPressed(KEY_U)) lMode = HeatMode::Uniform;
        if (IsKeyPressed(KEY_M)) lMode = HeatMode::MovingEmitters;
        if (IsKeyPressed(KEY_E)) { lEmitterCount += 1; lInitEmitters(lEmitters, lEmitterCount); }
        if (IsKeyPressed(KEY_Q)) { if (lEmitterCount>1){ lEmitterCount -= 1; lInitEmitters(lEmitters, lEmitterCount); } }

        if (!lPause){
            if (lMode == HeatMode::Uniform){
                GenPointsUniform(lPoints, lBatch);
            } else {
                lUpdateEmitters(lEmitters, lDt);
                GenPointsEmitters(lPoints, lBatch, lEmitters);
            }
            lHM_Acc.addPoints(lPoints);
            lHM_Repl.addPoints(lPoints);
            lHM_Decay.addPoints(lPoints);
        }

        lHM_Acc.update(lDt);
        lHM_Repl.update(lDt);
        lHM_Decay.update(lDt);

        BeginDrawing();
        ClearBackground(Color{18,18,22,255});

        lHM_Acc.draw();
        lHM_Repl.draw();
        lHM_Decay.draw();

        DrawTextEx(lFont, "Accumulate (3-color) press 1/2/3", Vector2{lA.x, lA.y-28}, 20, 1.0f, GRAY);
        DrawTextEx(lFont, "Replace per-batch (4-color)", Vector2{lB.x, lB.y-28}, 20, 1.0f, GRAY);
        DrawTextEx(lFont, "Decay (4-color, half-life 1.2s)", Vector2{lC.x, lC.y-28}, 20, 1.0f, GRAY);

        DrawTextEx(lFont, "Space: pause/resume, +/-: batch size, U: Uniform, M: MovingEmitters, E/Q: +/- emitters", Vector2{lPad, (float)(lH-36)}, 20, 1.0f, DARKGRAY);
        DrawTextEx(lFont, TextFormat("Batch: %d points/frame  |  Mode: %s  |  Emitters: %d",
                (int)lBatch, lMode==HeatMode::Uniform?"Uniform":"Moving", lEmitterCount),
                Vector2{lPad, (float)(lH-64)}, 20, 1.0f, DARKGRAY);
        DrawFPS(16,16);
        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}