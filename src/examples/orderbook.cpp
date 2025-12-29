
// orderbook.cpp
// Order Book Visualization Demo
// Demonstrates 2D heatmap and 3D landscape views of depth-of-market data
#include "RLOrderBookVis.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstdint>

// Fast PRNG (Xorshift)
static uint32_t gRngState = 123456789;

static inline void seedFast(uint32_t aSeed) {
    if (aSeed == 0) aSeed = 123456789;
    gRngState = aSeed;
}

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

// Simulated order book generator
class OrderBookSimulator {
public:
    OrderBookSimulator(float aBasePrice, float aTickSize, int aDepthLevels)
        : mBasePrice(aBasePrice)
        , mTickSize(aTickSize)
        , mDepthLevels(aDepthLevels)
        , mMidPrice(aBasePrice)
        , mSpread(aTickSize)
        , mDrift(0.0f)
        , mVolatility(0.0f)
    {
        // Initialize level sizes
        mBidSizes.resize((size_t)aDepthLevels, 0.0f);
        mAskSizes.resize((size_t)aDepthLevels, 0.0f);

        // Initial random sizes with distance decay
        for (int i = 0; i < aDepthLevels; ++i) {
            float lDecay = expf(-(float)i * 0.1f);
            mBidSizes[(size_t)i] = frandRange(100.0f, 5000.0f) * lDecay;
            mAskSizes[(size_t)i] = frandRange(100.0f, 5000.0f) * lDecay;
        }

        // Add some initial "walls"
        int lBidWall = (int)(randFast() % (unsigned)(aDepthLevels / 2)) + 3;
        int lAskWall = (int)(randFast() % (unsigned)(aDepthLevels / 2)) + 3;
        mBidSizes[(size_t)lBidWall] = frandRange(20000.0f, 50000.0f);
        mAskSizes[(size_t)lAskWall] = frandRange(20000.0f, 60000.0f);
    }

    void update(float aDt) {
        // Random walk for mid price
        mDrift += frandRange(-0.5f, 0.5f) * aDt;
        mDrift *= 0.99f; // Mean reversion

        mVolatility = frandRange(0.0f, 2.0f);
        float lPriceChange = (mDrift + frandRange(-1.0f, 1.0f) * mVolatility) * mTickSize * aDt * 10.0f;
        mMidPrice += lPriceChange;

        // Clamp mid price to reasonable range
        if (mMidPrice < mBasePrice * 0.9f) mMidPrice = mBasePrice * 0.9f;
        if (mMidPrice > mBasePrice * 1.1f) mMidPrice = mBasePrice * 1.1f;

        // Spread can widen/narrow
        mSpread += frandRange(-0.1f, 0.1f) * mTickSize * aDt;
        if (mSpread < mTickSize) mSpread = mTickSize;
        if (mSpread > mTickSize * 5.0f) mSpread = mTickSize * 5.0f;

        // Update level sizes with some randomness
        for (int i = 0; i < mDepthLevels; ++i) {
            // Add/remove liquidity
            float lChange = frandRange(-500.0f, 500.0f) * aDt;
            mBidSizes[(size_t)i] += lChange;
            mAskSizes[(size_t)i] += frandRange(-500.0f, 500.0f) * aDt;

            // Clamp to reasonable values
            if (mBidSizes[(size_t)i] < 0.0f) mBidSizes[(size_t)i] = frandRange(50.0f, 200.0f);
            if (mAskSizes[(size_t)i] < 0.0f) mAskSizes[(size_t)i] = frandRange(50.0f, 200.0f);

            // Cap sizes
            if (mBidSizes[(size_t)i] > 100000.0f) mBidSizes[(size_t)i] = 100000.0f;
            if (mAskSizes[(size_t)i] > 100000.0f) mAskSizes[(size_t)i] = 100000.0f;
        }

        // Occasionally create/remove walls
        if (randFast() % 1000 < 5) {
            int lLevel = (int)(randFast() % (unsigned)mDepthLevels);
            if (randFast() % 2 == 0) {
                mBidSizes[(size_t)lLevel] = frandRange(15000.0f, 60000.0f);
            } else {
                mAskSizes[(size_t)lLevel] = frandRange(15000.0f, 60000.0f);
            }
        }

        // Occasionally wipe a wall (hit/lifted)
        if (randFast() % 1000 < 3) {
            int lLevel = (int)(randFast() % (unsigned)mDepthLevels);
            if (mBidSizes[(size_t)lLevel] > 10000.0f) {
                mBidSizes[(size_t)lLevel] = frandRange(100.0f, 500.0f);
            }
            if (mAskSizes[(size_t)lLevel] > 10000.0f) {
                mAskSizes[(size_t)lLevel] = frandRange(100.0f, 500.0f);
            }
        }
    }

    RLOrderBookSnapshot getSnapshot() const {
        RLOrderBookSnapshot lSnap;

        float lBestBid = mMidPrice - mSpread * 0.5f;
        float lBestAsk = mMidPrice + mSpread * 0.5f;

        // Build bids (descending from best bid)
        lSnap.mBids.reserve((size_t)mDepthLevels);
        for (int i = 0; i < mDepthLevels; ++i) {
            float lPrice = lBestBid - (float)i * mTickSize;
            float lSize = mBidSizes[(size_t)i];
            lSnap.mBids.push_back(std::make_pair(lPrice, lSize));
        }

        // Build asks (ascending from best ask)
        lSnap.mAsks.reserve((size_t)mDepthLevels);
        for (int i = 0; i < mDepthLevels; ++i) {
            float lPrice = lBestAsk + (float)i * mTickSize;
            float lSize = mAskSizes[(size_t)i];
            lSnap.mAsks.push_back(std::make_pair(lPrice, lSize));
        }

        return lSnap;
    }

    float getMidPrice() const { return mMidPrice; }
    float getSpread() const { return mSpread; }

private:
    float mBasePrice;
    float mTickSize;
    int mDepthLevels;
    float mMidPrice;
    float mSpread;
    float mDrift;
    float mVolatility;
    std::vector<float> mBidSizes;
    std::vector<float> mAskSizes;
};

int main() {

    //Disable the mesh and vertex info logs
    SetTraceLogLevel(LOG_WARNING);

    seedFast((uint32_t)time(nullptr));

    const int SCREEN_WIDTH = 1600;
    const int SCREEN_HEIGHT = 900;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Order Book Visualization - 2D Heatmap & 3D Landscape");
    SetTargetFPS(60);

    Font lFont = LoadFontEx("base.ttf", 30, nullptr, 250);

    // Create order book visualization
    Rectangle lBounds2D = {50, 100, 700, 700};
    Rectangle lBounds3DInfo = {800, 100, 750, 700};

    RLOrderBookVisStyle lStyle;
    lStyle.mBackground = Color{15, 17, 22, 255};
    lStyle.mShowBorder = true;
    lStyle.mBorderColor = Color{50, 55, 65, 255};
    lStyle.mShowGrid = true;
    lStyle.mGridColor = Color{35, 40, 50, 100};
    lStyle.mGridLinesX = 10;
    lStyle.mGridLinesY = 8;
    lStyle.mShowMidLine = true;
    lStyle.mMidLineColor = Color{255, 255, 255, 200};
    lStyle.mShowSpreadArea = true;
    lStyle.mSpreadAreaColor = Color{255, 255, 255, 25};
    lStyle.mIntensityScale = 1.5f;
    lStyle.mHeightScale = 3.0f;
    lStyle.m3DCellSize = 0.15f;

    size_t lHistoryLength = 150;
    size_t lPriceLevels = 80;

    RLOrderBookVis lOrderBook(lBounds2D, lHistoryLength, lPriceLevels);
    lOrderBook.setStyle(lStyle);
    lOrderBook.setPriceMode(RLOrderBookPriceMode::SpreadTicks);
    lOrderBook.setSpreadTicks(40);

    // Custom color gradients (OSQF-inspired)
    std::vector<Color> lBidColors = {
        Color{5, 15, 25, 255},      // Dark blue-black
        Color{0, 60, 80, 255},      // Deep teal
        Color{0, 140, 100, 255},    // Teal-green
        Color{50, 220, 150, 255}    // Bright cyan-green
    };
    std::vector<Color> lAskColors = {
        Color{25, 10, 10, 255},     // Dark red-black
        Color{80, 30, 20, 255},     // Deep maroon
        Color{160, 50, 30, 255},    // Orange-red
        Color{255, 100, 60, 255}    // Bright orange
    };
    lOrderBook.setBidColorStops(lBidColors);
    lOrderBook.setAskColorStops(lAskColors);

    // Create simulator
    OrderBookSimulator lSim(100.0f, 0.01f, 50);

    // 3D camera setup
    Camera3D lCamera = {};
    lCamera.position = Vector3{15.0f, 12.0f, 15.0f};
    lCamera.target = Vector3{0.0f, 0.0f, 0.0f};
    lCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    lCamera.fovy = 45.0f;
    lCamera.projection = CAMERA_PERSPECTIVE;

    // Camera orbit state
    float lCameraAngle = 0.8f;
    float lCameraElevation = 0.4f;
    float lCameraDistance = 25.0f;
    bool lCameraAutoRotate = true;

    // View state
    bool lShow3D = false;
    float lSnapshotTimer = 0.0f;
    float lSnapshotInterval = 0.05f; // 20 updates per second

    // Controls state
    int lSpreadTicks = 40;
    bool lPaused = false;

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();

        // Handle input
        if (IsKeyPressed(KEY_TAB)) {
            lShow3D = !lShow3D;
        }
        if (IsKeyPressed(KEY_SPACE)) {
            lPaused = !lPaused;
        }
        if (IsKeyPressed(KEY_R)) {
            lOrderBook.clear();
        }
        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
            lSpreadTicks += 5;
            if (lSpreadTicks > 100) lSpreadTicks = 100;
            lOrderBook.setSpreadTicks(lSpreadTicks);
        }
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
            lSpreadTicks -= 5;
            if (lSpreadTicks < 5) lSpreadTicks = 5;
            lOrderBook.setSpreadTicks(lSpreadTicks);
        }
        if (IsKeyPressed(KEY_A)) {
            lCameraAutoRotate = !lCameraAutoRotate;
        }

        // 3D camera controls
        if (lShow3D) {
            if (lCameraAutoRotate) {
                lCameraAngle += 0.2f * lDt;
            }

            // Mouse drag for camera rotation
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vector2 lDelta = GetMouseDelta();
                lCameraAngle -= lDelta.x * 0.01f;
                lCameraElevation += lDelta.y * 0.01f;
                lCameraElevation = fmaxf(0.1f, fminf(1.4f, lCameraElevation));
            }

            // Mouse wheel for zoom
            float lWheel = GetMouseWheelMove();
            lCameraDistance -= lWheel * 2.0f;
            lCameraDistance = fmaxf(10.0f, fminf(50.0f, lCameraDistance));

            // Update camera position
            lCamera.position.x = cosf(lCameraAngle) * cosf(lCameraElevation) * lCameraDistance;
            lCamera.position.y = sinf(lCameraElevation) * lCameraDistance;
            lCamera.position.z = sinf(lCameraAngle) * cosf(lCameraElevation) * lCameraDistance;
        }

        // Update simulation and push snapshots
        if (!lPaused) {
            lSim.update(lDt);

            lSnapshotTimer += lDt;
            if (lSnapshotTimer >= lSnapshotInterval) {
                lSnapshotTimer = 0.0f;
                RLOrderBookSnapshot lSnap = lSim.getSnapshot();
                lOrderBook.pushSnapshot(lSnap);
            }
        }

        // Update visualization
        lOrderBook.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground(Color{10, 12, 18, 255});

        // Title
        DrawTextEx(lFont, "Order Book Visualization", Vector2{50, 20}, 30, 1.0f, Color{220, 220, 230, 255});

        // Draw 2D or 3D view
        if (!lShow3D) {
            lOrderBook.draw2D();

            // Draw price labels on the right side
            Rectangle lPlot = {lBounds2D.x + lStyle.mPadding, lBounds2D.y + lStyle.mPadding,
                               lBounds2D.width - 2*lStyle.mPadding, lBounds2D.height - 2*lStyle.mPadding};

            float lMinP = lOrderBook.getCurrentMidPrice() - lOrderBook.getSpreadTicks() * 0.01f;
            float lMaxP = lOrderBook.getCurrentMidPrice() + lOrderBook.getSpreadTicks() * 0.01f;

            for (int i = 0; i <= 5; ++i) {
                float lT = (float)i / 5.0f;
                float lPrice = lMaxP - lT * (lMaxP - lMinP);
                float lY = lPlot.y + lT * lPlot.height;

                char lBuf[32];
                snprintf(lBuf, sizeof(lBuf), "%.2f", (double)lPrice);
                DrawTextEx(lFont, lBuf, Vector2{lBounds2D.x + lBounds2D.width + 10, lY - 8}, 14, 1.0f, Color{150, 150, 160, 255});
            }

            // Axis labels
            DrawTextEx(lFont, "Time ->", Vector2{lBounds2D.x + lBounds2D.width/2 - 30, lBounds2D.y + lBounds2D.height + 15}, 16, 1.0f, Color{120, 120, 130, 255});
            DrawTextPro(lFont, "Price", Vector2{lBounds2D.x - 25, lBounds2D.y + lBounds2D.height/2 + 20}, Vector2{0, 0}, -90.0f, 16, 1, Color{120, 120, 130, 255});
        } else {
            // 3D view - draw3D handles viewport centering internally via RenderTexture
            lOrderBook.draw3D(lCamera);
        }

        // Info panel
        DrawRectangle((int)lBounds3DInfo.x, (int)lBounds3DInfo.y, (int)lBounds3DInfo.width, (int)lBounds3DInfo.height, Color{20, 22, 28, 255});
        DrawRectangleLinesEx(lBounds3DInfo, 1.0f, Color{50, 55, 65, 255});

        int lInfoY = (int)lBounds3DInfo.y + 20;
        int lInfoX = (int)lBounds3DInfo.x + 20;
        int lLineH = 28;

        DrawTextEx(lFont, "MARKET DATA", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        char lBuf[128];

        snprintf(lBuf, sizeof(lBuf), "Mid Price:  $%.4f", (double)lOrderBook.getCurrentMidPrice());
        DrawTextEx(lFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 18, 1.0f, Color{180, 180, 190, 255});
        lInfoY += lLineH;

        snprintf(lBuf, sizeof(lBuf), "Spread:     $%.4f (%.2f bps)", (double)lOrderBook.getCurrentSpread(),
                 (double)(lOrderBook.getCurrentSpread() / lOrderBook.getCurrentMidPrice() * 10000.0f));
        DrawTextEx(lFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 18, 1.0f, Color{180, 180, 190, 255});
        lInfoY += lLineH;

        snprintf(lBuf, sizeof(lBuf), "Snapshots:  %zu / %zu", lOrderBook.getSnapshotCount(), lOrderBook.getHistoryLength());
        DrawTextEx(lFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 18, 1.0f, Color{180, 180, 190, 255});
        lInfoY += lLineH;

        snprintf(lBuf, sizeof(lBuf), "Depth View: +/- %d ticks", lSpreadTicks);
        DrawTextEx(lFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 18, 1.0f, Color{180, 180, 190, 255});
        lInfoY += lLineH + 20;

        // Controls
        DrawTextEx(lFont, "CONTROLS", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        DrawTextEx(lFont, "[TAB]    Toggle 2D/3D view", Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lFont, "[+/-]    Adjust price depth", Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lFont, "[SPACE]  Pause/Resume", Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lFont, "[R]      Reset history", Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;

        if (lShow3D) {
            DrawTextEx(lFont, "[A]      Toggle auto-rotate", Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{140, 140, 150, 255});
            lInfoY += 22;
            DrawTextEx(lFont, "[Mouse]  Drag to orbit, wheel to zoom", Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{140, 140, 150, 255});
            lInfoY += 22;
        }

        lInfoY += 20;

        // Status indicators
        DrawTextEx(lFont, "STATUS", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        const char* lViewMode = lShow3D ? "3D Landscape" : "2D Heatmap";
        snprintf(lBuf, sizeof(lBuf), "View Mode:  %s", lViewMode);
        DrawTextEx(lFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 18, 1.0f, lShow3D ? Color{100, 180, 255, 255} : Color{100, 255, 150, 255});
        lInfoY += lLineH;

        const char* lStatus = lPaused ? "PAUSED" : "STREAMING";
        Color lStatusColor = lPaused ? Color{255, 180, 80, 255} : Color{80, 220, 120, 255};
        snprintf(lBuf, sizeof(lBuf), "Data Feed:  %s", lStatus);
        DrawTextEx(lFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 18, 1.0f, lStatusColor);
        lInfoY += lLineH + 30;

        // Legend
        DrawTextEx(lFont, "LEGEND", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        // Bid gradient
        DrawRectangle(lInfoX, lInfoY, 150, 20, lBidColors[0]);
        DrawRectangleGradientH(lInfoX, lInfoY, 150, 20, lBidColors[0], lBidColors[3]);
        DrawTextEx(lFont, "Bids (Buy Orders)", Vector2{(float)(lInfoX + 160), (float)(lInfoY + 2)}, 16, 1.0f, Color{50, 220, 150, 255});
        lInfoY += 30;

        // Ask gradient
        DrawRectangleGradientH(lInfoX, lInfoY, 150, 20, lAskColors[0], lAskColors[3]);
        DrawTextEx(lFont, "Asks (Sell Orders)", Vector2{(float)(lInfoX + 160), (float)(lInfoY + 2)}, 16, 1.0f, Color{255, 100, 60, 255});
        lInfoY += 30;

        // Mid line
        DrawLine(lInfoX, lInfoY + 10, lInfoX + 150, lInfoY + 10, Color{255, 255, 255, 200});
        DrawTextEx(lFont, "Mid Price / Spread", Vector2{(float)(lInfoX + 160), (float)(lInfoY + 2)}, 16, 1.0f, Color{255, 255, 255, 200});
        lInfoY += 30;

        // Interpretation guide
        DrawTextEx(lFont, "INTERPRETATION", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 5;

        DrawTextEx(lFont, "Bright colors = High liquidity (walls)", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{130, 130, 140, 255});
        lInfoY += 20;
        DrawTextEx(lFont, "Dark colors = Low liquidity", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{130, 130, 140, 255});
        lInfoY += 20;
        DrawTextEx(lFont, "Moving patterns = Price drift", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{130, 130, 140, 255});
        lInfoY += 20;
        DrawTextEx(lFont, "Sudden changes = Order fills/cancels", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{130, 130, 140, 255});

        // FPS
        DrawFPS(SCREEN_WIDTH - 100, 10);

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}
