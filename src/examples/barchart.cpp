#include "raylib.h"
#include "RLBarChart.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

static float lFrand(){ return (float)rand()/(float)RAND_MAX; }

static Color lPalette(int i){
    Color lCols[8] = {
        Color{  0,190,255,230},
        Color{ 80,220,120,230},
        Color{255,140, 80,230},
        Color{255, 95,120,230},
        Color{170,120,255,230},
        Color{255,220, 80,230},
        Color{ 80,210,200,230},
        Color{210,120,200,230}
    };
    return lCols[i%8];
}

//If the text is too large to fit the bar, it will be invisible.
static std::vector<RLBarData> lMakeBars(int aCount, float aMin=10.0f, float aMax=100.0f, bool aBorders=false, bool aLabels=true){
    std::vector<RLBarData> lOut;
    lOut.reserve(aCount);
    for (int i=0;i<aCount;i++){
        RLBarData lB;
        lB.value = aMin + lFrand()*(aMax - aMin);
        // Randomize color per bar (pick a random palette entry)
        lB.color = lPalette(rand());
        lB.showBorder = aBorders;
        lB.borderColor = Color{0,0,0,100};
        if (aLabels){
            lB.label = std::to_string((int)lB.value);
        }
        lOut.push_back(lB);
    }
    return lOut;
}

int main(){
    srand((unsigned)time(nullptr));
    const int lScreenW = 1280;
    const int lScreenH = 720;
    InitWindow(lScreenW, lScreenH, "raylib bar chart - RLBarChart demo");
    SetTargetFPS(120);

    // Layout
    Rectangle lTopLeft{ 40, 60, (lScreenW-120)*0.5f, (lScreenH-120)*0.45f };
    Rectangle lTopRight{ lTopLeft.x + lTopLeft.width + 40.0f, lTopLeft.y, lTopLeft.width, lTopLeft.height };
    Rectangle lBottomLeft{ lTopLeft.x, lTopLeft.y + lTopLeft.height + 40.0f, lTopLeft.width, lTopLeft.height };
    Rectangle lBottomRight{ lTopRight.x, lTopRight.y + lTopRight.height + 40.0f, lTopRight.width, lTopRight.height };

    RLBarChartStyle lStyle;
    lStyle.mBackground = Color{24,26,32,255};
    lStyle.mShowGrid = true;
    lStyle.mGridLines = 4;
    lStyle.mSpacing = 12.0f;
    lStyle.mCornerRadius = 8.0f;
    lStyle.lLabelFontSize = 16;

    // Vertical with labels (random number of bars between 5 and 15)
    RLBarChart lVertical(lTopLeft, RLBarOrientation::VERTICAL, lStyle);
    auto randCount = [](){ return 5 + (rand() % 11); }; // 5..15
    int lVCount = randCount();
    auto lVDataA = lMakeBars(lVCount, 10, 120, true, true);
    auto lVDataB = lMakeBars(lVCount, 10, 120, true, true);
    lVertical.setData(lVDataA);

    // Horizontal with labels off
    RLBarChartStyle lNoLabelStyle = lStyle; lNoLabelStyle.mShowLabels = false; lNoLabelStyle.mShowGrid = false;
    RLBarChart lHorizontal(lTopRight, RLBarOrientation::HORIZONTAL, lNoLabelStyle);
    auto lHDataA = lMakeBars(8, 5, 100, false, false);
    auto lHDataB = lMakeBars(8, 5, 100, false, false);
    lHorizontal.setData(lHDataA);

    // Vertical compact
    RLBarChartStyle lCompact = lStyle; lCompact.mSpacing = 6.0f; lCompact.mPadding = 10.0f; lCompact.lLabelFontSize = 14;
    RLBarChart lVerticalCompact(lBottomLeft, RLBarOrientation::VERTICAL, lCompact);
    auto lVCDataA = lMakeBars(12, 0, 80, false, true);
    auto lVCDataB = lMakeBars(12, 0, 80, false, true);
    lVerticalCompact.setData(lVCDataA);

    // Horizontal with borders and autoscale off (fixed scale 0..150)
    RLBarChartStyle lFixed = lStyle; lFixed.mAutoScale = false; lFixed.lMinValue = 0.0f; lFixed.lMaxValue = 150.0f;
    RLBarChart lHorizontalFixed(lBottomRight, RLBarOrientation::HORIZONTAL, lFixed);
    auto lHFDataA = lMakeBars(6, 0, 150, true, true);
    auto lHFDataB = lMakeBars(6, 0, 150, true, true);
    lHorizontalFixed.setData(lHFDataA);

    float lSwitchT = 0.0f;
    float lSwitchInterval = 2.5f;
    bool lPause = false;

    while (!WindowShouldClose()){
        float lDt = GetFrameTime();
        if (!lPause) lSwitchT += lDt;
        if (!lPause && lSwitchT > lSwitchInterval){
            lSwitchT = 0.0f;
            // Randomize vertical chart bar count between 5 and 15
            lVCount = randCount();
            lVDataB = lMakeBars(lVCount, 10, 120, true, true);
            lVertical.setTargetData(lVDataB);
            lHDataB = lMakeBars(8, 5, 100, false, false);
            lHorizontal.setTargetData(lHDataB);
            lVCDataB = lMakeBars(12, 0, 80, false, true);
            lVerticalCompact.setTargetData(lVCDataB);
            lHFDataB = lMakeBars(6, 0, 150, true, true);
            lHorizontalFixed.setTargetData(lHFDataB);
        }

        if (!lPause){
            lVertical.update(lDt);
            lHorizontal.update(lDt);
            lVerticalCompact.update(lDt);
            lHorizontalFixed.update(lDt);
        }

        BeginDrawing();
        ClearBackground(Color{18,18,22,255});

        lVertical.draw();
        lHorizontal.draw();
        lVerticalCompact.draw();
        lHorizontalFixed.draw();

        DrawText("Vertical (labels, borders, autoscale) random 5 to 15 bars", (int)lTopLeft.x, (int)(lTopLeft.y-26), 20, GRAY);
        DrawText("Horizontal (no labels, clean)", (int)lTopRight.x, (int)(lTopRight.y-26), 20, GRAY);
        DrawText("Vertical compact", (int)lBottomLeft.x, (int)(lBottomLeft.y-26), 20, GRAY);
        DrawText("Horizontal fixed scale 0..150 (borders)", (int)lBottomRight.x, (int)(lBottomRight.y-26), 20, GRAY);
        DrawText("Space: pause/resume  |  R: randomize now", 40, lScreenH-36, 20, DARKGRAY);
        DrawFPS(16,16);
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) lPause = !lPause;
        if (IsKeyPressed(KEY_R)){
            // Randomize vertical chart: count (5..15), values, and colors with smooth transition
            lVCount = randCount();
            lVDataA = lMakeBars(lVCount, 10, 120, true, true);
            lVertical.setTargetData(lVDataA);
            lHDataA = lMakeBars(8, 5, 100, false, false);
            lHorizontal.setData(lHDataA);
            lVCDataA = lMakeBars(12, 0, 80, false, true);
            lVerticalCompact.setData(lVCDataA);
            lHFDataA = lMakeBars(6, 0, 150, true, true);
            lHorizontalFixed.setData(lHFDataA);
        }
    }

    CloseWindow();
    return 0;
}
