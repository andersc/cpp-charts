#include "raylib.h"
#include "RLPieChart.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

static float lFrand(){ return (float)rand()/(float)RAND_MAX; }

static Color lPalette(int i){
    Color lCols[10] = {
        Color{  0,190,255,230},
        Color{ 80,220,120,230},
        Color{255,140, 80,230},
        Color{255, 95,120,230},
        Color{170,120,255,230},
        Color{255,220, 80,230},
        Color{ 80,210,200,230},
        Color{210,120,200,230},
        Color{120,220,160,230},
        Color{250,170, 60,230}
    };
    return lCols[i%10];
}

static std::vector<RLPieSliceData> lMakeSlices(int aCount){
    std::vector<RLPieSliceData> lOut;
    lOut.reserve(aCount);
    for (int i=0;i<aCount;i++){
        RLPieSliceData lS;
        lS.mValue = 5.0f + lFrand() * 30.0f;
        lS.mColor = lPalette(i);
        lS.mLabel = std::to_string((int)lS.mValue);
        lOut.push_back(lS);
    }
    return lOut;
}

int main(){
    srand((unsigned)time(nullptr));
    const int lScreenW = 1280;
    const int lScreenH = 720;
    InitWindow(lScreenW, lScreenH, "raylib pie chart - RLPieChart demo");
    SetTargetFPS(120);

    Rectangle lLeft{ 40, 80, (lScreenW-120)*0.33f, (lScreenH-140) };
    Rectangle lMid{ lLeft.x + lLeft.width + 40.0f, lLeft.y, lLeft.width, lLeft.height };
    Rectangle lRight{ lMid.x + lMid.width + 40.0f, lLeft.y, lLeft.width, lLeft.height };

    RLPieChartStyle lStyle; lStyle.mBackground = Color{24,26,32,255}; lStyle.mPadding = 16.0f; lStyle.mAngleSpeed = 8.0f; lStyle.mFadeSpeed = 8.0f;

    // Left: values change over time (solid pie)
    RLPieChart lPieA(lLeft, lStyle);
    auto lAData = lMakeSlices(5);
    lPieA.setData(lAData);
    lPieA.setHollowFactor(0.0f);

    // Middle: add/remove slices with fade and rebalance (donut)
    RLPieChart lPieB(lMid, lStyle);
    auto lBData = lMakeSlices(3);
    lPieB.setData(lBData);
    lPieB.setHollowFactor(0.5f);

    // Right: demonstrate hollow factors changing (ring)
    RLPieChart lPieC(lRight, lStyle);
    auto lCData = lMakeSlices(6);
    lPieC.setData(lCData);
    lPieC.setHollowFactor(0.75f);

    float lTimer = 0.0f;
    float lInterval = 2.2f;
    bool lPause = false;
    int lMode = 0; // for right pie hollow factor cycling

    while (!WindowShouldClose()){
        float lDt = GetFrameTime();
        if (!lPause) lTimer += lDt;
        if (!lPause && lTimer > lInterval){
            lTimer = 0.0f;
            // Left: tweak values (same count)
            for (size_t i=0;i<lAData.size();++i){
                lAData[i].mValue = 5.0f + lFrand()*35.0f;
                lAData[i].mLabel = std::to_string((int)lAData[i].mValue);
            }
            lPieA.setTargetData(lAData);

            // Middle: randomly add or remove a slice
            if ((rand() % 2)==0 && lBData.size() < 9){
                // add
                RLPieSliceData lS; lS.mValue = 5.0f + lFrand()*35.0f; lS.mColor = lPalette((int)lBData.size()); lS.mLabel = std::to_string((int)lS.mValue);
                lBData.push_back(lS);
            } else if (lBData.size() > 1){
                // remove random
                size_t lIdx = (size_t)(rand() % (int)lBData.size());
                lBData.erase(lBData.begin() + (long)lIdx);
            }
            lPieB.setTargetData(lBData);

            // Right: change hollow factor and values sometimes
            lMode = (lMode + 1) % 3; // 0 -> 0.0, 1 -> 0.5, 2 -> 0.9
            float lHF = (lMode==0?0.0f:(lMode==1?0.5f:0.9f));
            lPieC.setHollowFactor(lHF);
            // Also randomize values to show still visible behavior
            for (auto &s : lCData){ s.mValue = 5.0f + lFrand()*35.0f; s.mLabel = std::to_string((int)s.mValue); }
            lPieC.setTargetData(lCData);
        }

        if (!lPause){
            lPieA.update(lDt);
            lPieB.update(lDt);
            lPieC.update(lDt);
        }

        BeginDrawing();
        ClearBackground(Color{18,18,22,255});

        lPieA.draw();
        lPieB.draw();
        lPieC.draw();

        DrawText("Values change over time (solid)", (int)lLeft.x, (int)(lLeft.y-28), 20, GRAY);
        DrawText("Add/remove slices (fade) with donut style", (int)lMid.x, (int)(lMid.y-28), 20, GRAY);
        DrawText("Hollow factor demo: 0.0 -> 0.5 -> 0.9", (int)lRight.x, (int)(lRight.y-28), 20, GRAY);
        DrawText("Space: pause/resume | R: randomize now", 40, lScreenH-36, 20, DARKGRAY);
        DrawFPS(16,16);
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) lPause = !lPause;
        if (IsKeyPressed(KEY_R)){
            lAData = lMakeSlices(5);
            lPieA.setTargetData(lAData);
            lBData = lMakeSlices(3 + rand()%5);
            lPieB.setTargetData(lBData);
            lCData = lMakeSlices(6);
            lPieC.setTargetData(lCData);
        }
    }

    CloseWindow();
    return 0;
}
