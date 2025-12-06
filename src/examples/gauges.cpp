#include <cmath>
#include "RLGauge.h"

int main(){
    const int screenW = 1280;
    const int screenH = 720;
    InitWindow(screenW, screenH, "raylib gauges - RLGauge demo");
    SetTargetFPS(120);

    // Load custom font
    Font lBaseFont = LoadFontEx("base.ttf", 24, nullptr, 250);

    // Simple styles
    RLGaugeStyle styleDefault;
    styleDefault.mBackgroundColor = {20,22,28,255};
    styleDefault.mBaseArcColor = {50,55,65,255};
    styleDefault.mValueArcColor = {0,190,255,255};
    styleDefault.mNeedleColor = {255,80,80,255};
    styleDefault.mLabelColor = {235,235,245,255};
    styleDefault.mLabelFont = lBaseFont;

    RLGaugeStyle styleGreen = styleDefault;
    styleGreen.mValueArcColor = {80, 220, 120, 255};
    styleGreen.mNeedleColor = {80, 220, 120, 255};

    // Gauges
    RLGauge rpm({60, 60, 320, 320}, 0.0f, 8000.0f, styleDefault);
    RLGauge speed({screenW-380.0f, 60, 320, 320}, 0.0f, 240.0f, styleGreen);
    RLGauge temp({screenW*0.5f-160.0f, screenH-360.0f, 320, 320}, 40.0f, 120.0f, styleDefault);

    float t = 0.0f;
    while (!WindowShouldClose()){
        float dt = GetFrameTime();
        t += dt;

        // drive values
        float rpmVal = (cosf(t*0.9f)*0.5f+0.5f) * 8000.0f;
        float spdVal = (sinf(t*0.6f+1.7f)*0.5f+0.5f) * 240.0f;
        float tmpVal = 80.0f + 20.0f*sinf(t*0.7f+2.3f);

        rpm.setTargetValue(rpmVal);
        speed.setTargetValue(spdVal);
        temp.setTargetValue(tmpVal);

        rpm.update(dt);
        speed.update(dt);
        temp.update(dt);

        BeginDrawing();
        ClearBackground({18,18,22,255});

        rpm.draw();
        speed.draw();
        temp.draw();

        DrawText("RLGauge demo", 20, screenH-40, 20, GRAY);
        DrawFPS(20, 20);
        EndDrawing();
    }
    UnloadFont(lBaseFont);
    CloseWindow();
    return 0;
}
