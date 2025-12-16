#include <vector>
#include <cstdlib>
#include <ctime>
#include "RLBubble.h"

static float frand(){ return (float)rand()/(float)RAND_MAX; }

static std::vector<RLBubblePoint> MakeRandomData(int count, float minSize=0.5f, float maxSize=6.0f){
    std::vector<RLBubblePoint> out;
    out.reserve(count);
    for (int i=0;i<count;i++){
        RLBubblePoint p;
        p.mX = frand();
        p.mY = frand();
        p.mSize = minSize + frand()*(maxSize-minSize);
        // pleasant color palette
        Color cols[6] = {
            Color{  0,190,255,230},
            Color{ 80,220,120,230},
            Color{255,140, 80,230},
            Color{255, 95,120,230},
            Color{170,120,255,230},
            Color{255,220, 80,230}
        };
        p.mColor = cols[i%6];
        out.push_back(p);
    }
    return out;
}

static int RandCount(){ return 8 + (rand() % 8); } // 8..15

int main(){
    srand((unsigned)time(nullptr));
    const int screenW = 1280;
    const int screenH = 720;
    InitWindow(screenW, screenH, "raylib bubble chart - RLBubble demo");
    SetTargetFPS(120);

    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    // Layout
    Rectangle left{ 40, 60, (screenW-120)*0.5f, screenH-120.0f };
    Rectangle right{ left.x + left.width + 40.0f, left.y, left.width, left.height };

    RLBubbleStyle style;
    style.mBackground = Color{24,26,32,255};
    style.mGridLines = 5;
    style.mSizeScale = 22.0f;
    style.mMinRadius = 4.0f;
    style.mOutline = 2.0f;
    style.mOutlineColor = Color{0,0,0,70};
    style.mShowAxes = true;

    // Scatter chart
    RLBubble scatter(left, RLBubbleMode::Scatter, style);
    auto dataA = MakeRandomData(RandCount());
    auto dataB = MakeRandomData(RandCount());
    scatter.setData(dataA);
    // Start animating right away so the count change demo is visible immediately
    scatter.setTargetData(dataB);

    // Gravity chart
    RLBubble gravity(right, RLBubbleMode::Gravity, style);
    auto dataG = MakeRandomData(RandCount(), 2.0f, 12.0f);
    gravity.setData(dataG);
    // Ensure targets are initialized so bubbles are visible without pressing G
    gravity.setTargetData(dataG);

    float t = 0.0f;
    float switchT = 0.0f;
    float switchInterval = 3.0f;
    bool pause = false;

    while (!WindowShouldClose()){
        float dt = GetFrameTime();
        if (!pause) t += dt;

        // periodically animate to new data
        if (!pause){
            switchT += dt;
            if (switchT > switchInterval){
                switchT = 0.0f;
                dataA = dataB;
                dataB = MakeRandomData(RandCount());
                scatter.setTargetData(dataB);

                dataG = MakeRandomData(RandCount(), 1.0f, 12.0f);
                gravity.setTargetData(dataG);
            }
        }

        // update
        if (!pause){
            scatter.update(dt);
            gravity.update(dt);
        }

        BeginDrawing();
        ClearBackground(Color{18,18,22,255});

        scatter.draw();
        gravity.draw();

        DrawTextEx(lFont, "Scatter: x,y,size,color (animates between datasets)", Vector2{(float)left.x, (float)(left.y-28)}, 20, 1.0f, GRAY);
        DrawTextEx(lFont, "Gravity: largest centered, others attract like mass", Vector2{(float)right.x, (float)(right.y-28)}, 20, 1.0f, GRAY);
        DrawTextEx(lFont, "Space: pause/resume  |  G: regenerate both datasets now", Vector2{40, (float)(screenH-36)}, 20, 1.0f, DARKGRAY);
        DrawFPS(16,16);
        EndDrawing();

        // simple controls
        if (IsKeyPressed(KEY_SPACE)) pause = !pause;
        if (IsKeyPressed(KEY_G)){
            dataA = MakeRandomData(RandCount());
            dataB = MakeRandomData(RandCount());
            scatter.setData(dataA);
            scatter.setTargetData(dataB);

            dataG = MakeRandomData(RandCount(), 2.0f, 12.0f);
            gravity.setData(dataG);
            gravity.setTargetData(dataG);
        }
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}
