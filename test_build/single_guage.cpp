// single_gauge.cpp
// Simple single gauge demo
#include <cmath>
#include "RLGauge.h"

int main() {
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Single Gauge Demo - RLGauge");
    SetTargetFPS(60);

    // Create gauge style
    RLGaugeStyle lStyle;
    lStyle.mBackgroundColor = {20, 22, 28, 255};
    lStyle.mBaseArcColor = {50, 55, 65, 255};
    lStyle.mValueArcColor = {0, 190, 255, 255};
    lStyle.mNeedleColor = {255, 80, 80, 255};
    lStyle.mLabelColor = {235, 235, 245, 255};

    // Create a single gauge in the center of the screen
    float lGaugeSize = 400.0f;
    Rectangle lBounds = {
        (float)(SCREEN_WIDTH / 2) - lGaugeSize / 2.0f,
        (float)(SCREEN_HEIGHT / 2) - lGaugeSize / 2.0f,
        lGaugeSize,
        lGaugeSize
    };
    RLGauge lGauge(lBounds, 0.0f, 100.0f, lStyle);

    float lTime = 0.0f;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Animate gauge value using sine wave
        float lValue = (sinf(lTime * 0.8f) * 0.5f + 0.5f) * 100.0f;
        lGauge.setTargetValue(lValue);
        lGauge.update(lDt);

        BeginDrawing();
        ClearBackground({18, 18, 22, 255});

        lGauge.draw();

        DrawText("Single Gauge Demo", 20, SCREEN_HEIGHT - 40, 20, GRAY);
        DrawFPS(20, 20);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
