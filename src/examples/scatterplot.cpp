#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "RLScatterPlot.h"

static float frand(){ return (float)rand()/(float)RAND_MAX; }

static std::vector<Vector2> MakeRandomPoints(int aCount, float aMinX=0.0f, float aMaxX=1.0f, float aMinY=0.0f, float aMaxY=1.0f){
    std::vector<Vector2> lOut;
    lOut.reserve(aCount);
    for (int i=0;i<aCount;i++){
        float lX = aMinX + frand()*(aMaxX-aMinX);
        float lY = aMinY + frand()*(aMaxY-aMinY);
        lOut.push_back({lX,lY});
    }
    return lOut;
}

static std::vector<Vector2> MakeSineWave(int aCount, float aAmp=1.0f, float aFreq=2.0f){
    std::vector<Vector2> lOut;
    lOut.reserve(aCount);
    for (int i=0;i<aCount;i++){
        float lT = (float)i / (float)(aCount-1);
        float lX = lT;
        float lY = 0.5f + 0.45f * aAmp * sinf(lT * aFreq * 6.2831853f);
        lOut.push_back({lX,lY});
    }
    return lOut;
}

int main(){
    srand((unsigned)time(nullptr));
    const int lW = 1280;
    const int lH = 720;
    InitWindow(lW, lH, "raylib scatter plot - RLScatterPlot demo");
    SetTargetFPS(120);

    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    Rectangle lLeft{40,60,(lW-120)*0.5f, lH-120.0f};
    Rectangle lRight{ lLeft.x + lLeft.width + 40.0f, lLeft.y, lLeft.width, lLeft.height };

    RLScatterPlotStyle lStyle;
    lStyle.mBackground = Color{24,26,32,255};
    lStyle.mShowGrid = true;
    lStyle.mGridLines = 5;
    lStyle.mPadding = 12.0f;
    lStyle.mSplinePixels = 5.0f;

    // Left: single-series with linear lines
    RLScatterPlot lSingle(lLeft, lStyle);
    RLScatterSeriesStyle lSingleStyle;
    lSingleStyle.mLineColor = Color{ 0,190,255,230 };
    lSingleStyle.mLineThickness = 2.0f;
    lSingleStyle.mLineMode = RLScatterLineMode::Linear;
    lSingleStyle.mPointScale = 1.6f;
    lSingle.setSingleSeries(MakeSineWave(120), lSingleStyle);

    // Right: multi-series with mixed styles
    RLScatterPlot lMulti(lRight, lStyle);

    // Series A: spline, thick, cyan
    RLScatterSeries lA;
    lA.mData = MakeSineWave(80, 1.0f, 1.25f);
    lA.mStyle.mLineColor = Color{ 0, 200, 255, 255 };
    lA.mStyle.mLineThickness = 3.0f;
    lA.mStyle.mLineMode = RLScatterLineMode::Spline;
    lA.mStyle.mShowPoints = true;
    lA.mStyle.mPointScale = 1.4f;
    lMulti.addSeries(lA);

    // Series B: linear, orange, larger points
    RLScatterSeries lB;
    lB.mData = MakeSineWave(50, 0.6f, 2.0f);
    lB.mStyle.mLineColor = Color{ 255, 160, 90, 255 };
    lB.mStyle.mLineThickness = 2.0f;
    lB.mStyle.mLineMode = RLScatterLineMode::Linear;
    lB.mStyle.mShowPoints = true;
    lB.mStyle.mPointSizePx = 4.0f;
    lMulti.addSeries(lB);

    // Series C: scatter-only (no lines), magenta points
    RLScatterSeries lC;
    lC.mData = MakeRandomPoints(120);
    lC.mStyle.mLineMode = RLScatterLineMode::None;
    lC.mStyle.mShowPoints = true;
    lC.mStyle.mPointColor = Color{ 200, 120, 255, 230 };
    lC.mStyle.mPointSizePx = 3.0f;
    lMulti.addSeries(lC);
    int lCountC = 120;

    // Large dataset for performance (toggle)
    bool lShowLarge = false;
    RLScatterPlot lPerf(lLeft, lStyle);
    std::vector<Vector2> lBigA = MakeRandomPoints(15000);
    std::vector<Vector2> lBigB = MakeRandomPoints(15000);
    RLScatterSeriesStyle lBigStyle;
    lBigStyle.mLineMode = RLScatterLineMode::None; // pure scatter for speed
    lBigStyle.mPointSizePx = 2.0f;
    lPerf.setSingleSeries(lBigA, lBigStyle);

    // Animation timers
    float lSwitch = 0.0f;
    float lInterval = 2.5f;
    while (!WindowShouldClose()){
        float lDt = GetFrameTime();
        lSwitch += lDt;
        // Periodically change targets to demonstrate smooth animation/fade
        if (lSwitch > lInterval){
            lSwitch = 0.0f;
            // Single series: change amplitude/frequency slightly
            int lN = 100 + (rand()%60);
            float lAmp = 0.8f + 0.4f*frand();
            float lFreq = 1.0f + 1.2f*frand();
            lSingle.setSingleSeriesTargetData(MakeSineWave(lN, lAmp, lFreq));

            // Multi-series A and B: small reshuffle counts
            RLScatterSeries lTmpA = lA; // keep style
            lTmpA.mData = MakeSineWave(60 + (rand()%40), 1.0f, 1.0f + frand());
            lMulti.setSeriesTargetData(0, lTmpA.mData);

            RLScatterSeries lTmpB = lB;
            lTmpB.mData = MakeSineWave(30 + (rand()%50), 0.5f + 0.5f*frand(), 1.5f + frand());
            lMulti.setSeriesTargetData(1, lTmpB.mData);

            // Series C: random points with varying count to showcase add/remove fade
            int lCnt = 80 + (rand()%80);
            lCountC = lCnt;
            lMulti.setSeriesTargetData(2, MakeRandomPoints(lCnt));
        }

        // Update animation
        lSingle.update(lDt);
        lMulti.update(lDt);
        if (lShowLarge) lPerf.update(lDt);
        BeginDrawing();
        ClearBackground(Color{18,18,22,255});

        if (!lShowLarge){
            lSingle.draw();
            lMulti.draw();
            DrawTextEx(lFont, "Single-series (left): linear line + points", Vector2{(float)lLeft.x, (float)(lLeft.y-28)}, 20, 1.0f, GRAY);
            DrawTextEx(lFont, "Multi-series (right): spline vs linear vs scatter-only (animated)", Vector2{(float)lRight.x, (float)(lRight.y-28)}, 20, 1.0f, GRAY);
        } else {
            lPerf.draw();
            DrawTextEx(lFont, "Large dataset: 15k points (scatter-only)", Vector2{(float)lLeft.x, (float)(lLeft.y-28)}, 20, 1.0f, GRAY);
        }

        DrawTextEx(lFont, "Space: toggle large dataset view  |  A: add pts to C  |  R: remove pts from C", Vector2{40, (float)(lH-36)}, 20, 1.0f, DARKGRAY);
        DrawFPS(16,16);
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)){
            lShowLarge = !lShowLarge;
            // Swap perf data to stress cache invalidation
            if (lShowLarge){ lPerf.setSingleSeries(lBigB, lBigStyle); }
            else { lPerf.setSingleSeries(lBigA, lBigStyle); }
        }

        // Interactive add/remove on series C to demonstrate fade in/out
        if (IsKeyPressed(KEY_A)){
            // grow target by 10
            lCountC += 10;
            lMulti.setSeriesTargetData(2, MakeRandomPoints(lCountC));
        }
        if (IsKeyPressed(KEY_R)){
            lCountC = lCountC > 10 ? (lCountC - 10) : lCountC;
            lMulti.setSeriesTargetData(2, MakeRandomPoints(lCountC));
        }
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}
