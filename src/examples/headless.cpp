/**
 * headless.cpp — Headless chart rendering (no GPU / no display).
 *
 * Uses the raylib 6.0 Memory platform backend to render charts directly
 * into a framebuffer, then exports the result as a PNG image.
 *
 * Build requirements:
 *   raylib must be compiled with -DPLATFORM=Memory
 *   See headless/CMakeLists.txt for a ready-made build configuration.
 *
 * Usage:
 *   ./headless_charts [output.png]
 */

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "RLBarChart.h"
#include "RLPieChart.h"
#include "RLTimeSeries.h"
#include "RLTheme.h"

static std::vector<RLBarData> makeBarData() {
    std::vector<RLBarData> lBars;
    float lValues[] = {72.0f, 91.0f, 58.0f, 85.0f, 67.0f, 94.0f};
    Color lColors[] = {
        Color{0, 190, 255, 230}, Color{80, 220, 120, 230},
        Color{255, 140, 80, 230}, Color{255, 95, 120, 230},
        Color{170, 120, 255, 230}, Color{255, 220, 80, 230}
    };
    const char* lLabels[] = {"Q1", "Q2", "Q3", "Q4", "Q5", "Q6"};
    for (int i = 0; i < 6; ++i) {
        RLBarData lBar;
        lBar.value = lValues[i];
        lBar.color = lColors[i];
        lBar.label = lLabels[i];
        lBars.push_back(lBar);
    }
    return lBars;
}

int main(int argc, char* argv[]) {
    const char* lOutputPath = (argc > 1) ? argv[1] : "chart_output.png";
    const int lWidth = 1280;
    const int lHeight = 720;

    InitWindow(lWidth, lHeight, "headless");

    // Create a bar chart
    Rectangle lBounds{40.0f, 40.0f, (float)lWidth - 80.0f, (float)lHeight - 80.0f};
    RLBarChartStyle lStyle;
    lStyle.mBackground = Color{30, 30, 46, 255};
    lStyle.mGridColor = Color{80, 80, 100, 120};
    lStyle.mGridDashed = true;
    lStyle.mLabelColor = WHITE;
    lStyle.mShowGrid = true;
    lStyle.mGridLines = 5;
    lStyle.mShowLabels = true;
    RLBarChart lChart(lBounds, RLBarOrientation::VERTICAL, lStyle);
    lChart.setData(makeBarData());

    // Render one frame
    BeginDrawing();
    ClearBackground(lStyle.mBackground);
    lChart.draw();
    DrawText("Headless Chart Export (raylib 6.0 Memory Backend)", 40, 10, 20, WHITE);
    EndDrawing();

    // Capture framebuffer and export
    Image lImage = LoadImageFromScreen();
    ExportImage(lImage, lOutputPath);
    UnloadImage(lImage);

    printf("Chart exported to: %s (%dx%d)\n", lOutputPath, lWidth, lHeight);

    CloseWindow();
    return 0;
}
