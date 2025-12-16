// heatmap3d.cpp
// 3D Scientific Plot Visualization Demo
// Demonstrates surface and scatter modes with axis box, floor grid, transparent back walls,
// live streaming data, and partial region updates
#include "RLHeatMap3D.h"
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdint>

// Fast PRNG for live data simulation
static uint32_t gRngState = 123456789;

static inline uint32_t randFast() {
    uint32_t lX = gRngState;
    lX ^= lX << 13;
    lX ^= lX >> 17;
    lX ^= lX << 5;
    return gRngState = lX;
}

static inline float frandFast() {
    return (float)(randFast() & 0xFFFFFF) / (float)0xFFFFFF;
}

static inline float frandRange(float aMin, float aMax) {
    return aMin + frandFast() * (aMax - aMin);
}

// Demo mode enumeration - expanded with new modes
enum class DemoMode {
    SurfaceStatic,      // Static surface with breathing effect
    SurfaceLive,        // Animated sine waves surface
    SurfaceStreaming,   // Live streaming random data (simulates sensor feed)
    SurfacePartial,     // Partial region updates demonstration
    ScatterStatic,      // Static scatter points
    ScatterLive,        // Animated scatter points
    ModeCount
};

// Render style enumeration for W key cycling
enum class RenderStyle {
    Scatter,            // Scatter point mode
    Surface,            // Surface mesh without wireframe
    SurfaceWireframe,   // Surface mesh with wireframe overlay
    StyleCount
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
            float lNx = ((float)lX / (float)(aWidth - 1)) * 2.0f - 1.0f;
            float lNy = ((float)lY / (float)(aHeight - 1)) * 2.0f - 1.0f;
            float lValue = (lNx * lNx - lNy * lNy + 1.0f) * 0.5f;
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

            float lWave1 = sinf(lNx * 4.0f * PI + aTime * 2.0f) * 0.25f;
            float lWave2 = sinf(lNy * 3.0f * PI + aTime * 1.5f) * 0.25f;
            float lWave3 = sinf((lNx + lNy) * 5.0f * PI + aTime * 3.0f) * 0.15f;
            float lWave4 = cosf(sqrtf(lNx * lNx + lNy * lNy) * 8.0f * PI - aTime * 4.0f) * 0.15f;

            float lValue = 0.5f + lWave1 + lWave2 + lWave3 + lWave4;
            rValues[(size_t)(lY * aWidth + lX)] = lValue;
        }
    }
}

// Generate ripple pattern for scatter mode
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

// Streaming data state - simulates live sensor feed with smoothing
struct StreamingState {
    std::vector<float> mCurrentValues;
    std::vector<float> mTargetValues;
    float mUpdateTimer = 0.0f;
    float mUpdateInterval = 0.05f; // 20 Hz update rate

    void init(int aWidth, int aHeight) {
        size_t lSize = (size_t)(aWidth * aHeight);
        mCurrentValues.resize(lSize, 0.5f);
        mTargetValues.resize(lSize, 0.5f);

        // Initialize with some structure
        for (int lY = 0; lY < aHeight; ++lY) {
            for (int lX = 0; lX < aWidth; ++lX) {
                float lNx = (float)lX / (float)(aWidth - 1);
                float lNy = (float)lY / (float)(aHeight - 1);
                float lBase = 0.3f + 0.2f * sinf(lNx * PI) * sinf(lNy * PI);
                mCurrentValues[(size_t)(lY * aWidth + lX)] = lBase;
                mTargetValues[(size_t)(lY * aWidth + lX)] = lBase;
            }
        }
    }

    void update(float aDt, int aWidth, int aHeight) {
        mUpdateTimer += aDt;

        // Generate new target values periodically (simulates incoming data)
        if (mUpdateTimer >= mUpdateInterval) {
            mUpdateTimer = 0.0f;

            for (int lY = 0; lY < aHeight; ++lY) {
                for (int lX = 0; lX < aWidth; ++lX) {
                    size_t lIdx = (size_t)(lY * aWidth + lX);
                    // Add random noise to current value, with spatial correlation
                    float lNoise = frandRange(-0.1f, 0.1f);
                    float lNewVal = mTargetValues[lIdx] + lNoise;

                    // Add some spatial waves for visual interest
                    float lNx = (float)lX / (float)(aWidth - 1);
                    float lNy = (float)lY / (float)(aHeight - 1);
                    float lWave = 0.05f * sinf(lNx * 6.0f + mUpdateTimer * 10.0f) * cosf(lNy * 4.0f);
                    lNewVal += lWave;

                    // Clamp and apply
                    if (lNewVal < 0.0f) lNewVal = 0.0f;
                    if (lNewVal > 1.0f) lNewVal = 1.0f;
                    mTargetValues[lIdx] = lNewVal;
                }
            }
        }

        // Smooth interpolation towards target
        float lAlpha = 1.0f - expf(-8.0f * aDt);
        for (size_t i = 0; i < mCurrentValues.size(); ++i) {
            mCurrentValues[i] += (mTargetValues[i] - mCurrentValues[i]) * lAlpha;
        }
    }
};

// Partial update state - demonstrates updating specific regions
struct PartialUpdateState {
    int mActiveRegionX = 0;
    int mActiveRegionY = 0;
    int mRegionWidth = 10;
    int mRegionHeight = 10;
    float mRegionTimer = 0.0f;
    float mRegionMoveInterval = 1.5f; // Move region every 1.5 seconds
    std::vector<float> mRegionValues;
    std::vector<float> mBaseValues;
    float mPulsePhase = 0.0f;

    void init(int aWidth, int aHeight) {
        // Initialize base values with a simple gradient
        mBaseValues.resize((size_t)(aWidth * aHeight));
        for (int lY = 0; lY < aHeight; ++lY) {
            for (int lX = 0; lX < aWidth; ++lX) {
                float lNx = (float)lX / (float)(aWidth - 1);
                float lNy = (float)lY / (float)(aHeight - 1);
                mBaseValues[(size_t)(lY * aWidth + lX)] = 0.2f + 0.1f * (lNx + lNy);
            }
        }

        mRegionValues.resize((size_t)(mRegionWidth * mRegionHeight));
        mActiveRegionX = aWidth / 4;
        mActiveRegionY = aHeight / 4;
    }

    void update(float aDt, int aWidth, int aHeight, RLHeatMap3D& rHeatMap) {
        mRegionTimer += aDt;
        mPulsePhase += aDt * 4.0f;

        // Move the active region periodically
        if (mRegionTimer >= mRegionMoveInterval) {
            mRegionTimer = 0.0f;

            // Move to a new random position
            mActiveRegionX = (int)(randFast() % (unsigned)(aWidth - mRegionWidth));
            mActiveRegionY = (int)(randFast() % (unsigned)(aHeight - mRegionHeight));
        }

        // Generate pulsing hotspot data for the active region
        for (int lY = 0; lY < mRegionHeight; ++lY) {
            for (int lX = 0; lX < mRegionWidth; ++lX) {
                float lCx = (float)mRegionWidth * 0.5f;
                float lCy = (float)mRegionHeight * 0.5f;
                float lDx = (float)lX - lCx;
                float lDy = (float)lY - lCy;
                float lDist = sqrtf(lDx * lDx + lDy * lDy);
                float lMaxDist = sqrtf(lCx * lCx + lCy * lCy);

                // Pulsing gaussian hotspot
                float lPulse = 0.5f + 0.5f * sinf(mPulsePhase);
                float lValue = 0.3f + 0.7f * lPulse * expf(-lDist * lDist / (lMaxDist * 0.5f));
                mRegionValues[(size_t)(lY * mRegionWidth + lX)] = lValue;
            }
        }

        // Apply partial update to the heat map
        rHeatMap.updatePartialValues(mActiveRegionX, mActiveRegionY,
                                      mRegionWidth, mRegionHeight,
                                      mRegionValues.data());
    }

    void resetBase(RLHeatMap3D& rHeatMap) {
        rHeatMap.setValues(mBaseValues.data(), (int)mBaseValues.size());
    }
};

int main() {
    // Window setup
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLHeatMap3D - Scientific 3D Plot Demo");
    SetTargetFPS(60);

    // Camera setup for orbit control
    Camera3D lCamera = {};
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

    // Initialize streaming and partial update states
    StreamingState lStreamingState;
    lStreamingState.init(GRID_WIDTH, GRID_HEIGHT);

    PartialUpdateState lPartialState;
    lPartialState.init(GRID_WIDTH, GRID_HEIGHT);

    // Initial data
    generateGaussianHill(lValues, GRID_WIDTH, GRID_HEIGHT);
    lHeatMap.setValues(lValues.data(), (int)lValues.size());

    // Demo state
    DemoMode lMode = DemoMode::SurfaceStatic;
    DemoMode lPrevMode = lMode;
    float lTime = 0.0f;
    int lDatasetIndex = 0; // 0 = Gaussian, 1 = Saddle
    bool lAutoRange = true; // Auto-range vs fixed range toggle
    RenderStyle lRenderStyle = RenderStyle::SurfaceWireframe; // W key cycle state

    // Load font for UI
    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTime += lDt;

        // Handle keyboard input
        if (IsKeyPressed(KEY_SPACE)) {
            // Cycle through modes
            int lModeInt = (int)lMode;
            lModeInt = (lModeInt + 1) % (int)DemoMode::ModeCount;
            lMode = (DemoMode)lModeInt;
        }

        // Handle mode change - reset data when switching modes
        if (lMode != lPrevMode) {
            // Update render mode and style based on new mode
            if (lMode == DemoMode::ScatterStatic || lMode == DemoMode::ScatterLive) {
                lStyle.mMode = RLHeatMap3DMode::Scatter;
                lStyle.mShowWireframe = false;
                lRenderStyle = RenderStyle::Scatter;
            } else {
                lStyle.mMode = RLHeatMap3DMode::Surface;
                lStyle.mShowWireframe = true;
                lRenderStyle = RenderStyle::SurfaceWireframe;
            }
            lHeatMap.setStyle(lStyle);

            // Reset base data for partial mode
            if (lMode == DemoMode::SurfacePartial) {
                lPartialState.resetBase(lHeatMap);
            }

            lPrevMode = lMode;
        }

        if (IsKeyPressed(KEY_W)) {
            // Cycle through render styles: Scatter -> Surface -> SurfaceWireframe -> Scatter
            int lStyleInt = (int)lRenderStyle;
            lStyleInt = (lStyleInt + 1) % (int)RenderStyle::StyleCount;
            lRenderStyle = (RenderStyle)lStyleInt;

            // Apply the new render style - update lStyle.mMode so setStyle() applies it correctly
            switch (lRenderStyle) {
                case RenderStyle::Scatter:
                    lStyle.mMode = RLHeatMap3DMode::Scatter;
                    lStyle.mShowWireframe = false;
                    break;
                case RenderStyle::Surface:
                    lStyle.mMode = RLHeatMap3DMode::Surface;
                    lStyle.mShowWireframe = false;
                    break;
                case RenderStyle::SurfaceWireframe:
                    lStyle.mMode = RLHeatMap3DMode::Surface;
                    lStyle.mShowWireframe = true;
                    break;
                default:
                    break;
            }
            lHeatMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_G)) {
            lStyle.mShowFloorGrid = !lStyle.mShowFloorGrid;
            lHeatMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_B)) {
            lStyle.mShowAxisBox = !lStyle.mShowAxisBox;
            lHeatMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_D)) {
            lDatasetIndex = (lDatasetIndex + 1) % 2;
        }

        if (IsKeyPressed(KEY_A)) {
            // Toggle auto-range vs fixed range
            lAutoRange = !lAutoRange;
            if (lAutoRange) {
                lHeatMap.setAutoRange(true);
            } else {
                // Set fixed range 0.0 to 1.0
                lHeatMap.setValueRange(0.0f, 1.0f);
            }
        }

        if (IsKeyPressed(KEY_R)) {
            lCameraDistance = 3.0f;
            lCameraYaw = 0.8f;
            lCameraPitch = 0.5f;
        }

        // Update data based on mode
        switch (lMode) {
            case DemoMode::SurfaceStatic:
            case DemoMode::ScatterStatic: {
                // Static dataset with slow breathing
                float lPulse = 1.0f + 0.05f * sinf(lTime * 0.5f);
                if (lDatasetIndex == 0) {
                    generateGaussianHill(lValues, GRID_WIDTH, GRID_HEIGHT);
                } else {
                    generateSaddle(lValues, GRID_WIDTH, GRID_HEIGHT);
                }
                for (size_t i = 0; i < lValues.size(); ++i) {
                    lValues[i] *= lPulse;
                }
                lHeatMap.setValues(lValues.data(), (int)lValues.size());
                break;
            }

            case DemoMode::SurfaceLive: {
                // Animated sine waves
                generateSineWaves(lValues, GRID_WIDTH, GRID_HEIGHT, lTime);
                lHeatMap.setValues(lValues.data(), (int)lValues.size());
                break;
            }

            case DemoMode::SurfaceStreaming: {
                // Live streaming data - simulates sensor feed
                lStreamingState.update(lDt, GRID_WIDTH, GRID_HEIGHT);
                lHeatMap.setValues(lStreamingState.mCurrentValues.data(),
                                   (int)lStreamingState.mCurrentValues.size());
                break;
            }

            case DemoMode::SurfacePartial: {
                // Partial region updates - hotspot moves around
                lPartialState.update(lDt, GRID_WIDTH, GRID_HEIGHT, lHeatMap);
                break;
            }

            case DemoMode::ScatterLive: {
                // Animated ripple for scatter mode
                generateRipple(lValues, GRID_WIDTH, GRID_HEIGHT, lTime);
                lHeatMap.setValues(lValues.data(), (int)lValues.size());
                break;
            }

            default:
                break;
        }

        // Mouse controls for camera orbit
        Vector2 lMousePos = GetMousePosition();
        Vector2 lMouseDelta = Vector2{lMousePos.x - lLastMousePos.x, lMousePos.y - lLastMousePos.y};

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            lCameraYaw -= lMouseDelta.x * 0.005f;
            lCameraPitch -= lMouseDelta.y * 0.005f;

            if (lCameraPitch < 0.1f) lCameraPitch = 0.1f;
            if (lCameraPitch > 1.4f) lCameraPitch = 1.4f;
        }

        float lWheel = GetMouseWheelMove();
        if (lWheel != 0.0f) {
            lCameraDistance -= lWheel * 0.2f;
            if (lCameraDistance < 1.5f) lCameraDistance = 1.5f;
            if (lCameraDistance > 8.0f) lCameraDistance = 8.0f;
        }

        lLastMousePos = lMousePos;

        // Update camera position
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
        lHeatMap.draw(Vector3{0.0f, 0.0f, 0.0f}, 1.0f, lCamera);
        EndMode3D();

        // Draw UI
        const char* lModeNames[] = {
            "Mode: SURFACE (Static)",
            "Mode: SURFACE (Animated Waves)",
            "Mode: SURFACE (Live Streaming)",
            "Mode: SURFACE (Partial Updates)",
            "Mode: SCATTER (Static)",
            "Mode: SCATTER (Animated)"
        };
        DrawTextEx(lFont, lModeNames[(int)lMode], Vector2{20, 20}, 20, 1, WHITE);

        // Mode-specific info
        const char* lModeDesc = "";
        switch (lMode) {
            case DemoMode::SurfaceStatic:
            case DemoMode::ScatterStatic: {
                const char* lDatasetNames[] = {"Dataset: Gaussian Hill", "Dataset: Saddle Surface"};
                lModeDesc = lDatasetNames[lDatasetIndex];
                break;
            }
            case DemoMode::SurfaceLive:
                lModeDesc = "Overlapping sine waves animation";
                break;
            case DemoMode::SurfaceStreaming:
                lModeDesc = "Simulated live sensor data feed (20 Hz)";
                break;
            case DemoMode::SurfacePartial:
                lModeDesc = "Hotspot region updates every 1.5s";
                break;
            case DemoMode::ScatterLive:
                lModeDesc = "Moving ripple pattern";
                break;
            default:
                break;
        }
        DrawTextEx(lFont, lModeDesc, Vector2{20, 45}, 16, 1, Color{180, 180, 190, 255});

        // Show partial update region info
        if (lMode == DemoMode::SurfacePartial) {
            char lRegionBuf[64];
            snprintf(lRegionBuf, sizeof(lRegionBuf), "Active region: (%d, %d) %dx%d",
                     lPartialState.mActiveRegionX, lPartialState.mActiveRegionY,
                     lPartialState.mRegionWidth, lPartialState.mRegionHeight);
            DrawTextEx(lFont, lRegionBuf, Vector2{20, 65}, 14, 1, Color{255, 200, 100, 255});
        }

        DrawTextEx(lFont, "Controls:", Vector2{20, 90}, 16, 1, LIGHTGRAY);
        DrawTextEx(lFont, "  Mouse Drag: Rotate view", Vector2{20, 110}, 14, 1, GRAY);
        DrawTextEx(lFont, "  Mouse Wheel: Zoom", Vector2{20, 128}, 14, 1, GRAY);
        DrawTextEx(lFont, "  SPACE: Cycle modes (6 total)", Vector2{20, 146}, 14, 1, GRAY);
        DrawTextEx(lFont, "  W: Cycle style (Scatter/Surface/Wire)", Vector2{20, 164}, 14, 1, GRAY);
        DrawTextEx(lFont, "  G: Toggle floor grid", Vector2{20, 182}, 14, 1, GRAY);
        DrawTextEx(lFont, "  B: Toggle axis box", Vector2{20, 200}, 14, 1, GRAY);
        DrawTextEx(lFont, "  A: Toggle auto-range", Vector2{20, 218}, 14, 1, GRAY);
        DrawTextEx(lFont, "  D: Cycle datasets (static mode)", Vector2{20, 236}, 14, 1, GRAY);
        DrawTextEx(lFont, "  R: Reset camera", Vector2{20, 254}, 14, 1, GRAY);

        // Status indicators
        int lStatusY = SCREEN_HEIGHT - 100;
        char lStatusBuf[64];

        // Render style status
        const char* lRenderStyleNames[] = {"SCATTER", "SURFACE", "SURFACE+WIRE"};
        snprintf(lStatusBuf, sizeof(lStatusBuf), "Style: %s", lRenderStyleNames[(int)lRenderStyle]);
        Color lStyleColor = (lRenderStyle == RenderStyle::Scatter) ? Color{255, 180, 80, 255} :
                            (lRenderStyle == RenderStyle::Surface) ? Color{80, 200, 255, 255} : GREEN;
        DrawTextEx(lFont, lStatusBuf, Vector2{20, (float)lStatusY}, 14, 1, lStyleColor);

        snprintf(lStatusBuf, sizeof(lStatusBuf), "Floor Grid: %s", lStyle.mShowFloorGrid ? "ON" : "OFF");
        DrawTextEx(lFont, lStatusBuf, Vector2{20, (float)(lStatusY + 18)}, 14, 1, lStyle.mShowFloorGrid ? GREEN : GRAY);

        snprintf(lStatusBuf, sizeof(lStatusBuf), "Axis Box: %s", lStyle.mShowAxisBox ? "ON" : "OFF");
        DrawTextEx(lFont, lStatusBuf, Vector2{20, (float)(lStatusY + 36)}, 14, 1, lStyle.mShowAxisBox ? GREEN : GRAY);

        // Auto-range status
        snprintf(lStatusBuf, sizeof(lStatusBuf), "Range: %s", lAutoRange ? "AUTO" : "FIXED (0-1)");
        DrawTextEx(lFont, lStatusBuf, Vector2{20, (float)(lStatusY + 54)}, 14, 1, lAutoRange ? Color{100, 200, 255, 255} : Color{255, 200, 100, 255});

        // Value range
        snprintf(lStatusBuf, sizeof(lStatusBuf), "Z Range: %.2f - %.2f", lHeatMap.getMinValue(), lHeatMap.getMaxValue());
        DrawTextEx(lFont, lStatusBuf, Vector2{SCREEN_WIDTH - 180.0f, 20}, 14, 1, GRAY);

        // Mode counter
        snprintf(lStatusBuf, sizeof(lStatusBuf), "Mode %d/%d", (int)lMode + 1, (int)DemoMode::ModeCount);
        DrawTextEx(lFont, lStatusBuf, Vector2{SCREEN_WIDTH - 100.0f, 45}, 14, 1, Color{150, 150, 160, 255});

        DrawFPS(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 30);

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();

    return 0;
}

