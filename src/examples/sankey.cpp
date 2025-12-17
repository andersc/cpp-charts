// sankey.cpp
// Demo: Sankey diagram with animated flow visualization
// Shows multi-layer flow diagram with smooth value transitions,
// node/link add/remove with fade effects, and interactive controls.
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include "RLSankey.h"

// ============================================================================
// Demo Configuration
// ============================================================================

static const int SCREEN_WIDTH = 1400;
static const int SCREEN_HEIGHT = 800;
static const float UPDATE_INTERVAL = 2.0f;

// Color palette for nodes
static const Color NODE_COLORS[] = {
    {66, 133, 244, 255},    // Blue
    {52, 168, 83, 255},     // Green
    {251, 188, 4, 255},     // Yellow
    {234, 67, 53, 255},     // Red
    {154, 99, 191, 255},    // Purple
    {0, 188, 212, 255},     // Cyan
    {255, 112, 67, 255},    // Deep Orange
    {156, 204, 101, 255},   // Light Green
    {121, 134, 203, 255},   // Indigo
    {255, 167, 38, 255}     // Orange
};

// ============================================================================
// Helper Functions
// ============================================================================

static float randFloat(float aMin, float aMax) {
    return aMin + ((float)rand() / (float)RAND_MAX) * (aMax - aMin);
}

static Color getNodeColor(int aIndex) {
    return NODE_COLORS[aIndex % 10];
}

// ============================================================================
// Demo Data Setup
// ============================================================================

struct DemoData {
    std::vector<RLSankeyNode> mNodes;
    std::vector<RLSankeyLink> mLinks;
};

static DemoData createEnergyFlowDemo() {
    DemoData lData;

    // Column 0: Energy sources
    lData.mNodes.push_back({"Coal", getNodeColor(0), 0});
    lData.mNodes.push_back({"Natural Gas", getNodeColor(1), 0});
    lData.mNodes.push_back({"Nuclear", getNodeColor(2), 0});
    lData.mNodes.push_back({"Renewable", getNodeColor(3), 0});

    // Column 1: Conversion
    lData.mNodes.push_back({"Power Plants", getNodeColor(4), 1});
    lData.mNodes.push_back({"Direct Use", getNodeColor(5), 1});

    // Column 2: Distribution
    lData.mNodes.push_back({"Grid", getNodeColor(6), 2});
    lData.mNodes.push_back({"Local Gen", getNodeColor(7), 2});

    // Column 3: Sectors
    lData.mNodes.push_back({"Residential", getNodeColor(8), 3});
    lData.mNodes.push_back({"Commercial", getNodeColor(9), 3});
    lData.mNodes.push_back({"Industrial", getNodeColor(0), 3});
    lData.mNodes.push_back({"Transport", getNodeColor(1), 3});

    // Column 4: End use
    lData.mNodes.push_back({"Heating", getNodeColor(2), 4});
    lData.mNodes.push_back({"Cooling", getNodeColor(3), 4});
    lData.mNodes.push_back({"Lighting", getNodeColor(4), 4});
    lData.mNodes.push_back({"Motors", getNodeColor(5), 4});
    lData.mNodes.push_back({"Electronics", getNodeColor(6), 4});

    // Links from sources to conversion
    lData.mLinks.push_back({0, 4, 35.0f});  // Coal -> Power Plants
    lData.mLinks.push_back({1, 4, 25.0f});  // Gas -> Power Plants
    lData.mLinks.push_back({1, 5, 15.0f});  // Gas -> Direct Use
    lData.mLinks.push_back({2, 4, 20.0f});  // Nuclear -> Power Plants
    lData.mLinks.push_back({3, 4, 15.0f});  // Renewable -> Power Plants
    lData.mLinks.push_back({3, 5, 10.0f});  // Renewable -> Direct Use

    // Links from conversion to distribution
    lData.mLinks.push_back({4, 6, 80.0f});  // Power Plants -> Grid
    lData.mLinks.push_back({4, 7, 15.0f});  // Power Plants -> Local Gen
    lData.mLinks.push_back({5, 7, 25.0f});  // Direct Use -> Local Gen

    // Links from distribution to sectors
    lData.mLinks.push_back({6, 8, 25.0f});   // Grid -> Residential
    lData.mLinks.push_back({6, 9, 20.0f});   // Grid -> Commercial
    lData.mLinks.push_back({6, 10, 30.0f});  // Grid -> Industrial
    lData.mLinks.push_back({6, 11, 5.0f});   // Grid -> Transport
    lData.mLinks.push_back({7, 8, 15.0f});   // Local -> Residential
    lData.mLinks.push_back({7, 10, 20.0f});  // Local -> Industrial
    lData.mLinks.push_back({7, 11, 5.0f});   // Local -> Transport

    // Links from sectors to end use
    lData.mLinks.push_back({8, 12, 20.0f});   // Residential -> Heating
    lData.mLinks.push_back({8, 13, 10.0f});   // Residential -> Cooling
    lData.mLinks.push_back({8, 14, 5.0f});    // Residential -> Lighting
    lData.mLinks.push_back({8, 16, 5.0f});    // Residential -> Electronics
    lData.mLinks.push_back({9, 13, 12.0f});   // Commercial -> Cooling
    lData.mLinks.push_back({9, 14, 5.0f});    // Commercial -> Lighting
    lData.mLinks.push_back({9, 16, 3.0f});    // Commercial -> Electronics
    lData.mLinks.push_back({10, 12, 15.0f});  // Industrial -> Heating
    lData.mLinks.push_back({10, 15, 30.0f});  // Industrial -> Motors
    lData.mLinks.push_back({10, 16, 5.0f});   // Industrial -> Electronics
    lData.mLinks.push_back({11, 15, 10.0f});  // Transport -> Motors

    return lData;
}

static DemoData createWebsiteFlowDemo() {
    DemoData lData;

    // Column 0: Traffic sources
    lData.mNodes.push_back({"Search", {66, 133, 244, 255}, 0});
    lData.mNodes.push_back({"Social", {234, 67, 53, 255}, 0});
    lData.mNodes.push_back({"Direct", {52, 168, 83, 255}, 0});
    lData.mNodes.push_back({"Referral", {251, 188, 4, 255}, 0});

    // Column 1: Landing pages
    lData.mNodes.push_back({"Homepage", {154, 99, 191, 255}, 1});
    lData.mNodes.push_back({"Blog", {0, 188, 212, 255}, 1});
    lData.mNodes.push_back({"Products", {255, 112, 67, 255}, 1});

    // Column 2: Actions
    lData.mNodes.push_back({"Browse", {156, 204, 101, 255}, 2});
    lData.mNodes.push_back({"Read", {121, 134, 203, 255}, 2});
    lData.mNodes.push_back({"Add to Cart", {255, 167, 38, 255}, 2});

    // Column 3: Outcomes
    lData.mNodes.push_back({"Purchase", {76, 175, 80, 255}, 3});
    lData.mNodes.push_back({"Subscribe", {33, 150, 243, 255}, 3});
    lData.mNodes.push_back({"Bounce", {158, 158, 158, 255}, 3});

    // Source -> Landing
    lData.mLinks.push_back({0, 4, 40.0f});
    lData.mLinks.push_back({0, 5, 25.0f});
    lData.mLinks.push_back({0, 6, 35.0f});
    lData.mLinks.push_back({1, 4, 15.0f});
    lData.mLinks.push_back({1, 5, 30.0f});
    lData.mLinks.push_back({2, 4, 50.0f});
    lData.mLinks.push_back({2, 6, 20.0f});
    lData.mLinks.push_back({3, 5, 15.0f});
    lData.mLinks.push_back({3, 6, 10.0f});

    // Landing -> Actions
    lData.mLinks.push_back({4, 7, 60.0f});
    lData.mLinks.push_back({4, 9, 25.0f});
    lData.mLinks.push_back({4, 12, 20.0f});
    lData.mLinks.push_back({5, 8, 50.0f});
    lData.mLinks.push_back({5, 12, 20.0f});
    lData.mLinks.push_back({6, 7, 30.0f});
    lData.mLinks.push_back({6, 9, 25.0f});
    lData.mLinks.push_back({6, 12, 10.0f});

    // Actions -> Outcomes
    lData.mLinks.push_back({7, 9, 40.0f});
    lData.mLinks.push_back({7, 12, 50.0f});
    lData.mLinks.push_back({8, 11, 30.0f});
    lData.mLinks.push_back({8, 12, 20.0f});
    lData.mLinks.push_back({9, 10, 50.0f});

    return lData;
}

// ============================================================================
// Main Demo
// ============================================================================

int main() {
    srand((unsigned int)time(nullptr));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLSankey Demo - Flow Visualization");
    SetTargetFPS(60);

    // Load font
    Font lFont = LoadFontEx("base.ttf", 18, nullptr, 250);

    // ========================================================================
    // Create two Sankey charts side by side
    // ========================================================================
    float lMargin = 25.0f;
    float lGap = 30.0f;
    float lChartWidth = (SCREEN_WIDTH - 2.0f * lMargin - lGap) / 2.0f;
    float lChartHeight = SCREEN_HEIGHT - 2.0f * lMargin - 80.0f;

    Rectangle lBounds1 = {lMargin, lMargin + 60.0f, lChartWidth, lChartHeight};
    Rectangle lBounds2 = {lMargin + lChartWidth + lGap, lMargin + 60.0f, lChartWidth, lChartHeight};

    // Style configuration
    RLSankeyStyle lStyle;
    lStyle.mShowBackground = true;
    lStyle.mBackground = Color{18, 22, 30, 255};
    lStyle.mNodeWidth = 18.0f;
    lStyle.mNodePadding = 8.0f;
    lStyle.mNodeCornerRadius = 3.0f;
    lStyle.mShowNodeBorder = true;
    lStyle.mNodeBorderColor = Color{255, 255, 255, 30};
    lStyle.mColumnSpacing = 120.0f;
    lStyle.mMinLinkThickness = 2.0f;
    lStyle.mLinkAlpha = 0.55f;
    lStyle.mLinkSegments = 32;
    lStyle.mLinkColorMode = RLSankeyLinkColorMode::GRADIENT;
    lStyle.mShowLabels = true;
    lStyle.mLabelColor = Color{200, 210, 225, 255};
    lStyle.mLabelFont = lFont;
    lStyle.mLabelFontSize = 13;
    lStyle.mLabelPadding = 6.0f;
    lStyle.mPadding = 50.0f;
    lStyle.mSmoothAnimate = true;
    lStyle.mAnimateSpeed = 4.0f;
    lStyle.mFadeSpeed = 3.0f;

    // Chart 1: Energy flow
    RLSankey lChart1(lBounds1, lStyle);
    DemoData lEnergyData = createEnergyFlowDemo();
    lChart1.setData(lEnergyData.mNodes, lEnergyData.mLinks);

    // Chart 2: Website flow
    RLSankeyStyle lStyle2 = lStyle;
    lStyle2.mBackground = Color{22, 18, 30, 255};
    RLSankey lChart2(lBounds2, lStyle2);
    DemoData lWebData = createWebsiteFlowDemo();
    lChart2.setData(lWebData.mNodes, lWebData.mLinks);

    // ========================================================================
    // Animation state
    // ========================================================================
    float lFluctuateTimer = 0.0f;
    int lColorModeIndex = 0;
    const char* COLOR_MODE_NAMES[] = {"Gradient", "Source", "Target"};
    int lFlowModeIndex = 0;
    const char* FLOW_MODE_NAMES[] = {"Normalized", "Raw Value"};
    bool lStrictMode = false;

    // Store original link values for fluctuation
    std::vector<float> lOriginalValues1;
    for (const auto& rLink : lEnergyData.mLinks) {
        lOriginalValues1.push_back(rLink.mValue);
    }
    std::vector<float> lOriginalValues2;
    for (const auto& rLink : lWebData.mLinks) {
        lOriginalValues2.push_back(rLink.mValue);
    }

    // Track added nodes/links for removal demo
    int lExtraNodeId = -1;
    std::vector<size_t> lExtraLinkIds;
    bool lExtraAdded = false;

    // ========================================================================
    // Main loop
    // ========================================================================
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lFluctuateTimer += lDt;

        // ====================================================================
        // Input handling
        // ====================================================================
        Vector2 lMouse = GetMousePosition();

        // Toggle color modes with keys
        if (IsKeyPressed(KEY_C)) {
            lColorModeIndex = (lColorModeIndex + 1) % 3;
            RLSankeyLinkColorMode lNewMode;
            switch (lColorModeIndex) {
                case 0: lNewMode = RLSankeyLinkColorMode::GRADIENT; break;
                case 1: lNewMode = RLSankeyLinkColorMode::SOURCE; break;
                case 2: lNewMode = RLSankeyLinkColorMode::TARGET; break;
                default: lNewMode = RLSankeyLinkColorMode::GRADIENT; break;
            }
            lStyle.mLinkColorMode = lNewMode;
            lStyle2.mLinkColorMode = lNewMode;
            lChart1.setStyle(lStyle);
            lChart2.setStyle(lStyle2);
        }

        // Toggle labels
        if (IsKeyPressed(KEY_L)) {
            lStyle.mShowLabels = !lStyle.mShowLabels;
            lStyle2.mShowLabels = lStyle.mShowLabels;
            lChart1.setStyle(lStyle);
            lChart2.setStyle(lStyle2);
        }

        // Toggle flow mode
        if (IsKeyPressed(KEY_N)) {
            lFlowModeIndex = (lFlowModeIndex + 1) % 2;
            RLSankeyFlowMode lNewMode = (lFlowModeIndex == 0)
                ? RLSankeyFlowMode::NORMALIZED
                : RLSankeyFlowMode::RAW_VALUE;
            lStyle.mFlowMode = lNewMode;
            lStyle2.mFlowMode = lNewMode;
            lChart1.setStyle(lStyle);
            lChart2.setStyle(lStyle2);
        }

        // Toggle strict flow conservation
        if (IsKeyPressed(KEY_S)) {
            lStrictMode = !lStrictMode;
            lStyle.mStrictFlowConservation = lStrictMode;
            lStyle2.mStrictFlowConservation = lStrictMode;
            lChart1.setStyle(lStyle);
            lChart2.setStyle(lStyle2);
            // Validate and log warnings if strict mode is enabled
            if (lStrictMode) {
                lChart1.validateFlowConservation();
                lChart2.validateFlowConservation();
            }
        }

        // Manual add/remove demo
        // Only allow add if not currently added AND no pending removals in the chart
        if (IsKeyPressed(KEY_A) && !lExtraAdded && !lChart1.hasPendingRemovals()) {
            // Add a new node and links to chart 1
            lExtraNodeId = (int)lChart1.addNode("New Source", {255, 100, 150, 255}, 0);
            lExtraLinkIds.clear();
            lExtraLinkIds.push_back(lChart1.addLink(lExtraNodeId, 4, 20.0f));
            lExtraLinkIds.push_back(lChart1.addLink(lExtraNodeId, 5, 10.0f));
            lExtraAdded = true;
        }

        if (IsKeyPressed(KEY_R) && lExtraAdded) {
            // Remove the extra node (links will be removed automatically)
            lChart1.removeNode(lExtraNodeId);
            lExtraAdded = false;
            lExtraNodeId = -1;
            lExtraLinkIds.clear();
        }

        // ====================================================================
        // Fluctuate link values periodically
        // ====================================================================
        if (lFluctuateTimer > UPDATE_INTERVAL) {
            lFluctuateTimer = 0.0f;

            // Fluctuate chart 1 values
            for (size_t i = 0; i < lOriginalValues1.size(); ++i) {
                float lBase = lOriginalValues1[i];
                float lNew = lBase * randFloat(0.7f, 1.3f);
                lChart1.setLinkValue(i, lNew);
            }

            // Fluctuate chart 2 values
            for (size_t i = 0; i < lOriginalValues2.size(); ++i) {
                float lBase = lOriginalValues2[i];
                float lNew = lBase * randFloat(0.75f, 1.25f);
                lChart2.setLinkValue(i, lNew);
            }
        }

        // ====================================================================
        // Hover detection
        // ====================================================================
        int lHoveredNode1 = lChart1.getHoveredNode(lMouse);
        int lHoveredLink1 = (lHoveredNode1 < 0) ? lChart1.getHoveredLink(lMouse) : -1;
        lChart1.setHighlightedNode(lHoveredNode1);
        lChart1.setHighlightedLink(lHoveredLink1);

        int lHoveredNode2 = lChart2.getHoveredNode(lMouse);
        int lHoveredLink2 = (lHoveredNode2 < 0) ? lChart2.getHoveredLink(lMouse) : -1;
        lChart2.setHighlightedNode(lHoveredNode2);
        lChart2.setHighlightedLink(lHoveredLink2);

        // ====================================================================
        // Update charts
        // ====================================================================
        lChart1.update(lDt);
        lChart2.update(lDt);

        // ====================================================================
        // Draw
        // ====================================================================
        BeginDrawing();
        ClearBackground(Color{12, 14, 18, 255});

        // Draw charts
        lChart1.draw();
        lChart2.draw();

        // Draw titles
        const char* lTitle1 = "Energy Flow";
        const char* lTitle2 = "Website Analytics";
        int lTitleSize = 22;
        Color lTitleColor = {230, 235, 245, 255};

        Vector2 lTitle1Size = MeasureTextEx(lFont, lTitle1, (float)lTitleSize, 1.0f);
        Vector2 lTitle2Size = MeasureTextEx(lFont, lTitle2, (float)lTitleSize, 1.0f);

        DrawTextEx(lFont, lTitle1,
                   {lBounds1.x + (lBounds1.width - lTitle1Size.x) * 0.5f, lMargin + 15.0f},
                   (float)lTitleSize, 1.0f, lTitleColor);
        DrawTextEx(lFont, lTitle2,
                   {lBounds2.x + (lBounds2.width - lTitle2Size.x) * 0.5f, lMargin + 15.0f},
                   (float)lTitleSize, 1.0f, lTitleColor);

        // Draw help text
        Color lHelpColor = {140, 150, 170, 255};
        int lHelpSize = 14;
        float lHelpY = SCREEN_HEIGHT - 30.0f;

        char lHelpText[512];
        snprintf(lHelpText, sizeof(lHelpText),
                 "[C] Color: %s  [N] Flow: %s  [S] Strict: %s  [L] Labels  [A] Add  [R] Remove  |  Values fluctuate every %.1fs",
                 COLOR_MODE_NAMES[lColorModeIndex], FLOW_MODE_NAMES[lFlowModeIndex],
                 lStrictMode ? "ON" : "OFF", UPDATE_INTERVAL);

        DrawTextEx(lFont, lHelpText, {lMargin, lHelpY}, (float)lHelpSize, 1.0f, lHelpColor);

        // Draw hover info
        if (lHoveredNode1 >= 0 || lHoveredNode2 >= 0) {
            const char* lHoverText = "Hovering node";
            DrawTextEx(lFont, lHoverText, {SCREEN_WIDTH - 150.0f, lHelpY}, (float)lHelpSize, 1.0f, {100, 200, 255, 255});
        } else if (lHoveredLink1 >= 0 || lHoveredLink2 >= 0) {
            const char* lHoverText = "Hovering link";
            DrawTextEx(lFont, lHoverText, {SCREEN_WIDTH - 150.0f, lHelpY}, (float)lHelpSize, 1.0f, {255, 180, 100, 255});
        }

        // FPS
        DrawFPS(SCREEN_WIDTH - 100, 10);

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();

    return 0;
}

