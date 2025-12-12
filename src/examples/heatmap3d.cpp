// heatmap3d.cpp
// 3D Scientific Plot Visualization Demo
// Demonstrates surface and scatter modes with axis box, floor grid, and transparent back walls
#include "RLHeatMap3D.h"
#include <vector>
#include <cmath>
#include <cstdio>

// Demo mode enumeration
enum class DemoMode {
    SurfaceStatic,
    SurfaceLive,
    ScatterStatic,
    ScatterLive
};

// Generate Gaussian hill dataset
static void generateGaussianHill(std::vector<float>& rValues, int aWidth, int aHeight) {
    rValues.resize((size_t)(aWidth * aHeight));

    float lCenterX = (float)aWidth * 0.5f;
    float lCenterY = (float)aHeight * 0.5f;
    float lSigma = (float)aWidth * 0.25f;

    for (int lY = 0; lY < aHeight; ++lY) {
        for (int lX = 0; lX < aWidth; ++lX) {
            float lDx = (float)lX - lCenterX;
            float lDy = (float)lY - lCenterY;
            float lDist2 = lDx * lDx + lDy * lDy;
            float lValue = expf(-lDist2 / (2.0f * lSigma * lSigma));
            rValues[(size_t)(lY * aWidth + lX)] = lValue;
        }
    }
}

// Generate saddle surface (z = x^2 - y^2)
static void generateSaddle(std::vector<float>& rValues, int aWidth, int aHeight) {
    rValues.resize((size_t)(aWidth * aHeight));

    for (int lY = 0; lY < aHeight; ++lY) {
        for (int lX = 0; lX < aWidth; ++lX) {
            float lNx = ((float)lX / (float)(aWidth - 1)) * 2.0f - 1.0f;  // -1 to 1
            float lNy = ((float)lY / (float)(aHeight - 1)) * 2.0f - 1.0f; // -1 to 1
            float lValue = (lNx * lNx - lNy * lNy + 1.0f) * 0.5f; // Normalize to ~0-1
            rValues[(size_t)(lY * aWidth + lX)] = lValue;
        }
    }
}

// Generate animated sine wave pattern
static void generateSineWaves(std::vector<float>& rValues, int aWidth, int aHeight, float aTime) {
    rValues.resize((size_t)(aWidth * aHeight));

    for (int lY = 0; lY < aHeight; ++lY) {
        for (int lX = 0; lX < aWidth; ++lX) {
            float lNx = (float)lX / (float)aWidth;
            float lNy = (float)lY / (float)aHeight;

            // Multiple overlapping sine waves
            float lWave1 = sinf(lNx * 4.0f * PI + aTime * 2.0f) * 0.25f;
            float lWave2 = sinf(lNy * 3.0f * PI + aTime * 1.5f) * 0.25f;
            float lWave3 = sinf((lNx + lNy) * 5.0f * PI + aTime * 3.0f) * 0.15f;
            float lWave4 = cosf(sqrtf(lNx * lNx + lNy * lNy) * 8.0f * PI - aTime * 4.0f) * 0.15f;

            float lValue = 0.5f + lWave1 + lWave2 + lWave3 + lWave4;
            rValues[(size_t)(lY * aWidth + lX)] = lValue;
        }
    }
}

// Generate ripple pattern
static void generateRipple(std::vector<float>& rValues, int aWidth, int aHeight, float aTime) {
    rValues.resize((size_t)(aWidth * aHeight));

    float lCx = 0.5f + 0.2f * sinf(aTime * 0.7f);
    float lCy = 0.5f + 0.2f * cosf(aTime * 0.5f);

    for (int lY = 0; lY < aHeight; ++lY) {
        for (int lX = 0; lX < aWidth; ++lX) {
            float lNx = (float)lX / (float)(aWidth - 1);
            float lNy = (float)lY / (float)(aHeight - 1);

            float lDx = lNx - lCx;
            float lDy = lNy - lCy;
            float lDist = sqrtf(lDx * lDx + lDy * lDy);

            float lValue = 0.5f + 0.4f * sinf(lDist * 15.0f - aTime * 5.0f) * expf(-lDist * 2.0f);
            rValues[(size_t)(lY * aWidth + lX)] = lValue;
        }
    }
}

int main() {
    // Window setup
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLHeatMap3D - Scientific 3D Plot Demo");
    SetTargetFPS(60);

    // Camera setup for orbit control
    Camera3D lCamera = {0};
    lCamera.position = Vector3{2.0f, 1.5f, 2.0f};
    lCamera.target = Vector3{0.0f, 0.4f, 0.0f};
    lCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    lCamera.fovy = 45.0f;
    lCamera.projection = CAMERA_PERSPECTIVE;

    // Orbit camera state
    float lCameraDistance = 3.0f;
    float lCameraYaw = 0.8f;
    float lCameraPitch = 0.5f;

    // Mouse state
    Vector2 lLastMousePos = GetMousePosition();

    // Create 3D heat map
    const int GRID_WIDTH = 40;
    const int GRID_HEIGHT = 40;

    RLHeatMap3D lHeatMap(GRID_WIDTH, GRID_HEIGHT);

    // Configure style for scientific visualization
    RLHeatMap3DStyle lStyle;
    lStyle.mMode = RLHeatMap3DMode::Surface;
    lStyle.mSmoothingSpeed = 4.0f;
    lStyle.mShowWireframe = true;
    lStyle.mWireframeColor = Color{60, 60, 70, 180};
    lStyle.mSurfaceOpacity = 0.9f;
    lStyle.mShowAxisBox = true;
    lStyle.mAxisColor = Color{140, 140, 150, 255};
    lStyle.mGridColor = Color{70, 70, 80, 150};
    lStyle.mBackWallColor = Color{50, 55, 65, 60};
    lStyle.mGridDivisions = 10;
    lStyle.mShowFloorGrid = true;
    lStyle.mFloorGridColor = Color{60, 65, 75, 100};
    lStyle.mShowTicks = true;
    lStyle.mTickCount = 5;
    lStyle.mTickColor = Color{160, 160, 170, 255};
    lStyle.mPointSize = 0.02f;
    lHeatMap.setStyle(lStyle);

    // Set custom palette: blue -> cyan -> green -> yellow -> red
    lHeatMap.setPalette(
        Color{30, 60, 180, 255},     // Blue (low)
        Color{0, 180, 200, 255},     // Cyan
        Color{100, 220, 100, 255},   // Green
        Color{255, 180, 50, 255}     // Orange/Yellow (high)
    );

    // Data buffers
    std::vector<float> lValues;

    // Initial data
    generateGaussianHill(lValues, GRID_WIDTH, GRID_HEIGHT);
    lHeatMap.setValues(lValues.data(), (int)lValues.size());

    // Demo state
    DemoMode lMode = DemoMode::SurfaceStatic;
    float lTime = 0.0f;
    int lDatasetIndex = 0; // 0 = Gaussian, 1 = Saddle

    // Load font for UI
    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Handle keyboard input
        if (IsKeyPressed(KEY_SPACE)) {
            // Cycle through modes
            int lModeInt = (int)lMode;
            lModeInt = (lModeInt + 1) % 4;
            lMode = (DemoMode)lModeInt;

            // Update render mode
            if (lMode == DemoMode::SurfaceStatic || lMode == DemoMode::SurfaceLive) {
                lHeatMap.setMode(RLHeatMap3DMode::Surface);
            } else {
                lHeatMap.setMode(RLHeatMap3DMode::Scatter);
            }
        }

        if (IsKeyPressed(KEY_W)) {
            // Toggle wireframe
            lStyle.mShowWireframe = !lStyle.mShowWireframe;
            lHeatMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_G)) {
            // Toggle floor grid
            lStyle.mShowFloorGrid = !lStyle.mShowFloorGrid;
            lHeatMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_B)) {
            // Toggle axis box
            lStyle.mShowAxisBox = !lStyle.mShowAxisBox;
            lHeatMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_D)) {
            // Cycle static datasets
            lDatasetIndex = (lDatasetIndex + 1) % 2;
        }

        if (IsKeyPressed(KEY_R)) {
            // Reset camera
            lCameraDistance = 3.0f;
            lCameraYaw = 0.8f;
            lCameraPitch = 0.5f;
        }

        // Update data based on mode
        if (lMode == DemoMode::SurfaceStatic || lMode == DemoMode::ScatterStatic) {
            // Static dataset with slow breathing
            float lPulse = 1.0f + 0.05f * sinf(lTime * 0.5f);
            if (lDatasetIndex == 0) {
                generateGaussianHill(lValues, GRID_WIDTH, GRID_HEIGHT);
            } else {
                generateSaddle(lValues, GRID_WIDTH, GRID_HEIGHT);
            }
            // Apply pulse
            for (size_t i = 0; i < lValues.size(); ++i) {
                lValues[i] *= lPulse;
            }
            lHeatMap.setValues(lValues.data(), (int)lValues.size());
        } else {
            // Live animated data
            if (lMode == DemoMode::SurfaceLive) {
                generateSineWaves(lValues, GRID_WIDTH, GRID_HEIGHT, lTime);
            } else {
                generateRipple(lValues, GRID_WIDTH, GRID_HEIGHT, lTime);
            }
            lHeatMap.setValues(lValues.data(), (int)lValues.size());
        }

        // Mouse controls for camera orbit
        Vector2 lMousePos = GetMousePosition();
        Vector2 lMouseDelta = Vector2{lMousePos.x - lLastMousePos.x, lMousePos.y - lLastMousePos.y};

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            lCameraYaw -= lMouseDelta.x * 0.005f;
            lCameraPitch -= lMouseDelta.y * 0.005f;

            // Clamp pitch
            if (lCameraPitch < 0.1f) lCameraPitch = 0.1f;
            if (lCameraPitch > 1.4f) lCameraPitch = 1.4f;
        }

        // Mouse wheel for zoom
        float lWheel = GetMouseWheelMove();
        if (lWheel != 0.0f) {
            lCameraDistance -= lWheel * 0.2f;
            if (lCameraDistance < 1.5f) lCameraDistance = 1.5f;
            if (lCameraDistance > 8.0f) lCameraDistance = 8.0f;
        }

        lLastMousePos = lMousePos;

        // Update camera position based on orbit parameters
        lCamera.position.x = sinf(lCameraYaw) * cosf(lCameraPitch) * lCameraDistance;
        lCamera.position.y = sinf(lCameraPitch) * lCameraDistance;
        lCamera.position.z = cosf(lCameraYaw) * cosf(lCameraPitch) * lCameraDistance;
        lCamera.target = Vector3{0.0f, 0.4f, 0.0f};

        // Update heat map animation
        lHeatMap.update(lDt);

        // Render
        BeginDrawing();
        ClearBackground(Color{25, 28, 35, 255});

        BeginMode3D(lCamera);

        // Draw the 3D heat map with axis box and all decorations
        lHeatMap.draw(Vector3{0.0f, 0.0f, 0.0f}, 1.0f, lCamera);

        EndMode3D();

        // Draw UI
        const char* lModeNames[] = {
            "Mode: SURFACE (Static)",
            "Mode: SURFACE (Animated)",
            "Mode: SCATTER (Static)",
            "Mode: SCATTER (Animated)"
        };
        DrawTextEx(lFont, lModeNames[(int)lMode], Vector2{20, 20}, 20, 1, WHITE);

        const char* lDatasetNames[] = {"Dataset: Gaussian Hill", "Dataset: Saddle Surface"};
        if (lMode == DemoMode::SurfaceStatic || lMode == DemoMode::ScatterStatic) {
            DrawTextEx(lFont, lDatasetNames[lDatasetIndex], Vector2{20, 45}, 16, 1, Color{180, 180, 190, 255});
        } else {
            const char* lAnimNames[] = {"", "", "Animation: Sine Waves", "Animation: Ripple"};
            DrawTextEx(lFont, lAnimNames[(int)lMode], Vector2{20, 45}, 16, 1, Color{180, 180, 190, 255});
        }

        DrawTextEx(lFont, "Controls:", Vector2{20, 80}, 16, 1, LIGHTGRAY);
        DrawTextEx(lFont, "  Mouse Drag: Rotate view", Vector2{20, 100}, 14, 1, GRAY);
        DrawTextEx(lFont, "  Mouse Wheel: Zoom", Vector2{20, 118}, 14, 1, GRAY);
        DrawTextEx(lFont, "  SPACE: Cycle modes", Vector2{20, 136}, 14, 1, GRAY);
        DrawTextEx(lFont, "  W: Toggle wireframe", Vector2{20, 154}, 14, 1, GRAY);
        DrawTextEx(lFont, "  G: Toggle floor grid", Vector2{20, 172}, 14, 1, GRAY);
        DrawTextEx(lFont, "  B: Toggle axis box", Vector2{20, 190}, 14, 1, GRAY);
        DrawTextEx(lFont, "  D: Cycle datasets (static mode)", Vector2{20, 208}, 14, 1, GRAY);
        DrawTextEx(lFont, "  R: Reset camera", Vector2{20, 226}, 14, 1, GRAY);

        // Status indicators
        int lStatusY = SCREEN_HEIGHT - 80;
        char lStatusBuf[64];

        snprintf(lStatusBuf, sizeof(lStatusBuf), "Wireframe: %s", lStyle.mShowWireframe ? "ON" : "OFF");
        DrawTextEx(lFont, lStatusBuf, Vector2{20, (float)lStatusY}, 14, 1, lStyle.mShowWireframe ? GREEN : GRAY);

        snprintf(lStatusBuf, sizeof(lStatusBuf), "Floor Grid: %s", lStyle.mShowFloorGrid ? "ON" : "OFF");
        DrawTextEx(lFont, lStatusBuf, Vector2{20, (float)(lStatusY + 18)}, 14, 1, lStyle.mShowFloorGrid ? GREEN : GRAY);

        snprintf(lStatusBuf, sizeof(lStatusBuf), "Axis Box: %s", lStyle.mShowAxisBox ? "ON" : "OFF");
        DrawTextEx(lFont, lStatusBuf, Vector2{20, (float)(lStatusY + 36)}, 14, 1, lStyle.mShowAxisBox ? GREEN : GRAY);

        // Value range
        snprintf(lStatusBuf, sizeof(lStatusBuf), "Z Range: %.2f - %.2f", lHeatMap.getMinValue(), lHeatMap.getMaxValue());
        DrawTextEx(lFont, lStatusBuf, Vector2{SCREEN_WIDTH - 180.0f, 20}, 14, 1, GRAY);

        DrawFPS(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 30);

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();

    return 0;
}

