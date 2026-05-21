/**
 * pixelbuffer.cpp — Render a chart and access the raw pixel buffer.
 *
 * Demonstrates how to render a chart using the raylib 6.0 Memory platform
 * and retrieve the raw RGBA pixel data for sending over a network, piping
 * to another process, or integrating into a custom rendering pipeline.
 *
 * Build requirements:
 *   raylib must be compiled with -DPLATFORM=Memory
 *   See headless/CMakeLists.txt for a ready-made build configuration.
 *
 * Usage:
 *   ./pixelbuffer_charts [output.rgba]
 *
 * The output is raw RGBA pixel data (4 bytes per pixel, top-to-bottom,
 * left-to-right). Dimensions are printed to stdout for the receiver.
 */

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "RLBarChart.h"
#include "RLPieChart.h"

static std::vector<RLBarData> makeBarData() {
    std::vector<RLBarData> lBars;
    float lValues[] = {72.0f, 91.0f, 58.0f, 85.0f, 67.0f};
    Color lColors[] = {
        Color{0, 190, 255, 230}, Color{80, 220, 120, 230},
        Color{255, 140, 80, 230}, Color{255, 95, 120, 230},
        Color{170, 120, 255, 230}
    };
    const char* lLabels[] = {"Jan", "Feb", "Mar", "Apr", "May"};
    for (int32_t i = 0; i < 5; ++i) {
        RLBarData lBar;
        lBar.value = lValues[i];
        lBar.color = lColors[i];
        lBar.label = lLabels[i];
        lBars.push_back(lBar);
    }
    return lBars;
}

int main(int argc, char* argv[]) {
    const char* lOutputPath = (argc > 1) ? argv[1] : "chart_output.rgba";
    const int32_t lWidth = 800;
    const int32_t lHeight = 600;

    InitWindow(lWidth, lHeight, "pixelbuffer");

    // Render a bar chart
    Rectangle lBounds{20.0f, 20.0f, (float)lWidth - 40.0f, (float)lHeight - 40.0f};
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

    BeginDrawing();
    ClearBackground(lStyle.mBackground);
    lChart.draw();
    EndDrawing();

    // Access the raw pixel buffer
    Image lImage = LoadImageFromScreen();
    uint8_t* lPixels = (uint8_t*)lImage.data;
    int32_t lDataSize = lImage.width * lImage.height * 4; // RGBA = 4 bytes per pixel

    // Write raw RGBA data to file (or could send over network)
    FILE* lFile = fopen(lOutputPath, "wb");
    if (lFile) {
        fwrite(lPixels, 1, (size_t)lDataSize, lFile);
        fclose(lFile);
        printf("Pixel buffer written: %s\n", lOutputPath);
        printf("  Format: RGBA (4 bytes/pixel)\n");
        printf("  Dimensions: %dx%d\n", lImage.width, lImage.height);
        printf("  Buffer size: %d bytes\n", lDataSize);
    } else {
        fprintf(stderr, "Error: could not open %s for writing\n", lOutputPath);
        UnloadImage(lImage);
        CloseWindow();
        return 1;
    }

    // Demonstrate reading back specific pixel values
    int32_t lCenterX = lImage.width / 2;
    int32_t lCenterY = lImage.height / 2;
    int32_t lOffset = (lCenterY * lImage.width + lCenterX) * 4;
    printf("  Center pixel (%d,%d): R=%d G=%d B=%d A=%d\n",
           lCenterX, lCenterY,
           lPixels[lOffset], lPixels[lOffset + 1],
           lPixels[lOffset + 2], lPixels[lOffset + 3]);

    UnloadImage(lImage);
    CloseWindow();
    return 0;
}
