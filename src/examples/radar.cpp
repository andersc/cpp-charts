// radar.cpp
// Demo: Radar/Spider chart with single and multi-series visualization
// Shows smooth data transitions, series add/remove with fade effects,
// and interactive controls for toggling fill, markers, and datasets.
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include "RLRadarChart.h"

// ============================================================================
// Demo Configuration
// ============================================================================

static const int SCREEN_WIDTH = 1400;
static const int SCREEN_HEIGHT = 800;
static const float UPDATE_INTERVAL = 2.5f;

// Color palette for series
static const Color SERIES_COLORS[] = {
    {80, 180, 255, 255},    // Blue
    {255, 120, 80, 255},    // Orange
    {120, 220, 120, 255},   // Green
    {220, 100, 220, 255},   // Purple
    {255, 200, 80, 255}     // Yellow
};

// Axis labels for skill profile
static const char* SKILL_AXES[] = {
    "Strength", "Speed", "Intelligence", "Stamina", "Magic", "Defense"
};

// Axis labels for product comparison
static const char* PRODUCT_AXES[] = {
    "Performance", "Reliability", "Cost", "Features", "Support", "Ease of Use", "Security"
};

// ============================================================================
// Helper Functions
// ============================================================================

static float randFloat(float aMin, float aMax) {
    return aMin + ((float)rand() / (float)RAND_MAX) * (aMax - aMin);
}

static Color withAlpha(Color aColor, unsigned char aAlpha) {
    return Color{aColor.r, aColor.g, aColor.b, aAlpha};
}


static std::vector<float> generateProfileValues(size_t aCount, float aBase, float aVariance) {
    std::vector<float> lValues;
    lValues.reserve(aCount);
    for (size_t i = 0; i < aCount; ++i) {
        float lValue = aBase + randFloat(-aVariance, aVariance);
        lValues.push_back(fmaxf(10.0f, fminf(100.0f, lValue)));
    }
    return lValues;
}

// ============================================================================
// Main Demo
// ============================================================================

int main() {
    srand((unsigned int)time(nullptr));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RLRadarChart Demo - Radar/Spider Charts");
    SetTargetFPS(60);

    // Load font
    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    // ========================================================================
    // Chart 1: Single-series skill profile
    // ========================================================================
    float lMargin = 25.0f;
    float lGap = 20.0f;
    float lChartWidth = (SCREEN_WIDTH - 2.0f * lMargin - lGap) / 2.0f;
    float lChartHeight = SCREEN_HEIGHT - 2.0f * lMargin - 80.0f;

    Rectangle lBounds1 = {lMargin, lMargin + 60.0f, lChartWidth, lChartHeight};
    Rectangle lBounds2 = {lMargin + lChartWidth + lGap, lMargin + 60.0f, lChartWidth, lChartHeight};

    // Style for single-series chart
    RLRadarChartStyle lStyle1;
    lStyle1.mShowBackground = true;
    lStyle1.mBackground = Color{20, 24, 32, 255};
    lStyle1.mShowGrid = true;
    lStyle1.mGridRings = 5;
    lStyle1.mGridColor = Color{45, 50, 60, 255};
    lStyle1.mGridThickness = 1.0f;
    lStyle1.mShowAxes = true;
    lStyle1.mAxisColor = Color{60, 65, 75, 255};
    lStyle1.mAxisThickness = 1.5f;
    lStyle1.mShowLabels = true;
    lStyle1.mLabelColor = Color{180, 190, 210, 255};
    lStyle1.mLabelFont = lFont;
    lStyle1.mLabelFontSize = 14;
    lStyle1.mLabelOffset = 15.0f;
    lStyle1.mShowLegend = true;
    lStyle1.mPadding = 70.0f;
    lStyle1.mNormMode = RLRadarNormMode::GLOBAL;
    lStyle1.mSmoothAnimate = true;
    lStyle1.mAnimateSpeed = 5.0f;
    lStyle1.mFadeSpeed = 4.0f;

    RLRadarChart lChart1(lBounds1, lStyle1);

    // Set up axes for skill profile
    std::vector<std::string> lSkillLabels;
    for (const char* pLabel : SKILL_AXES) {
        lSkillLabels.push_back(pLabel);
    }
    lChart1.setAxes(lSkillLabels, 0.0f, 100.0f);

    // Add initial series - "Profile A"
    RLRadarSeries lSeries1;
    lSeries1.mLabel = "Profile A";
    lSeries1.mValues = generateProfileValues(6, 70.0f, 25.0f);
    lSeries1.mLineColor = SERIES_COLORS[0];
    lSeries1.mFillColor = withAlpha(SERIES_COLORS[0], 60);
    lSeries1.mLineThickness = 2.5f;
    lSeries1.mShowFill = true;
    lSeries1.mShowMarkers = true;
    lSeries1.mMarkerScale = 1.8f;
    lChart1.addSeries(lSeries1);

    // ========================================================================
    // Chart 2: Multi-series product comparison
    // ========================================================================
    RLRadarChartStyle lStyle2 = lStyle1;
    lStyle2.mBackground = Color{18, 22, 30, 255};
    lStyle2.mGridColor = Color{40, 45, 55, 255};

    RLRadarChart lChart2(lBounds2, lStyle2);

    // Set up axes for product comparison
    std::vector<std::string> lProductLabels;
    for (const char* pLabel : PRODUCT_AXES) {
        lProductLabels.push_back(pLabel);
    }
    lChart2.setAxes(lProductLabels, 0.0f, 100.0f);

    // Add multiple series
    const char* PRODUCT_NAMES[] = {"Product A", "Product B", "Product C"};
    float BASE_VALUES[] = {75.0f, 60.0f, 85.0f};

    for (int i = 0; i < 3; ++i) {
        RLRadarSeries lSeries;
        lSeries.mLabel = PRODUCT_NAMES[i];
        lSeries.mValues = generateProfileValues(7, BASE_VALUES[i], 20.0f);
        lSeries.mLineColor = SERIES_COLORS[i];
        lSeries.mFillColor = withAlpha(SERIES_COLORS[i], 40);
        lSeries.mLineThickness = 2.0f;
        lSeries.mShowFill = true;
        lSeries.mShowMarkers = true;
        lSeries.mMarkerScale = 1.5f;
        lChart2.addSeries(lSeries);
    }

    // ========================================================================
    // Animation state
    // ========================================================================
    float lTimer = 0.0f;
    int lDatasetIndex = 0;
    bool lShowFill = true;
    bool lShowMarkers = true;
    bool lAddingRemovingSeries = false;
    float lAddRemoveTimer = 0.0f;

    // Profile presets for Chart 1
    const char* PROFILE_NAMES[] = {"Warrior", "Mage", "Rogue", "Tank"};
    float PROFILE_BASES[][6] = {
        {90.0f, 60.0f, 40.0f, 80.0f, 20.0f, 85.0f},  // Warrior
        {30.0f, 40.0f, 95.0f, 50.0f, 100.0f, 35.0f}, // Mage
        {50.0f, 95.0f, 70.0f, 60.0f, 40.0f, 40.0f},  // Rogue
        {70.0f, 30.0f, 50.0f, 95.0f, 30.0f, 95.0f}   // Tank
    };

    // ========================================================================
    // Main loop
    // ========================================================================
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lTimer += lDt;
        lAddRemoveTimer += lDt;

        // ====================================================================
        // Input handling
        // ====================================================================

        // Toggle fill (F key)
        if (IsKeyPressed(KEY_F)) {
            lShowFill = !lShowFill;
            // Update Chart 1
            for (size_t i = 0; i < lChart1.getSeriesCount(); ++i) {
                RLRadarSeries lS;
                lS.mLabel = PROFILE_NAMES[lDatasetIndex];
                lS.mShowFill = lShowFill;
                lS.mShowMarkers = lShowMarkers;
                lS.mLineColor = SERIES_COLORS[lDatasetIndex];
                lS.mFillColor = withAlpha(SERIES_COLORS[lDatasetIndex], lShowFill ? 60 : 0);
                lS.mLineThickness = 2.5f;
                lS.mMarkerScale = 1.8f;
                std::vector<float> lValues;
                for (int j = 0; j < 6; ++j) {
                    lValues.push_back(PROFILE_BASES[lDatasetIndex][j] + randFloat(-10.0f, 10.0f));
                }
                lS.mValues = lValues;
                lChart1.setSeriesData(i, lS);
            }
            // Update Chart 2
            for (size_t i = 0; i < lChart2.getSeriesCount(); ++i) {
                RLRadarSeries lS;
                lS.mLabel = PRODUCT_NAMES[i % 3];
                lS.mValues = generateProfileValues(7, BASE_VALUES[i % 3], 15.0f);
                lS.mLineColor = SERIES_COLORS[i];
                lS.mFillColor = withAlpha(SERIES_COLORS[i], lShowFill ? 40 : 0);
                lS.mLineThickness = 2.0f;
                lS.mShowFill = lShowFill;
                lS.mShowMarkers = lShowMarkers;
                lChart2.setSeriesData(i, lS);
            }
        }

        // Toggle markers (M key)
        if (IsKeyPressed(KEY_M)) {
            lShowMarkers = !lShowMarkers;
            for (size_t i = 0; i < lChart2.getSeriesCount(); ++i) {
                RLRadarSeries lS;
                lS.mLabel = PRODUCT_NAMES[i % 3];
                lS.mValues = generateProfileValues(7, BASE_VALUES[i % 3], 15.0f);
                lS.mLineColor = SERIES_COLORS[i];
                lS.mFillColor = withAlpha(SERIES_COLORS[i], lShowFill ? 40 : 0);
                lS.mLineThickness = 2.0f;
                lS.mShowFill = lShowFill;
                lS.mShowMarkers = lShowMarkers;
                lChart2.setSeriesData(i, lS);
            }
        }

        // Cycle datasets (SPACE key)
        if (IsKeyPressed(KEY_SPACE)) {
            lDatasetIndex = (lDatasetIndex + 1) % 4;

            // Update Chart 1 with new profile
            std::vector<float> lNewValues;
            for (int i = 0; i < 6; ++i) {
                lNewValues.push_back(PROFILE_BASES[lDatasetIndex][i] + randFloat(-10.0f, 10.0f));
            }

            RLRadarSeries lNewSeries;
            lNewSeries.mLabel = PROFILE_NAMES[lDatasetIndex];
            lNewSeries.mValues = lNewValues;
            lNewSeries.mLineColor = SERIES_COLORS[lDatasetIndex];
            lNewSeries.mFillColor = withAlpha(SERIES_COLORS[lDatasetIndex], 60);
            lNewSeries.mLineThickness = 2.5f;
            lNewSeries.mShowFill = lShowFill;
            lNewSeries.mShowMarkers = lShowMarkers;
            lNewSeries.mMarkerScale = 1.8f;
            lChart1.setSeriesData(0, lNewSeries);
        }

        // Add/remove series in Chart 2 (A/R keys)
        if (IsKeyPressed(KEY_A) && lChart2.getSeriesCount() < 5) {
            size_t lIdx = lChart2.getSeriesCount();
            RLRadarSeries lNewSeries;
            lNewSeries.mLabel = "Product " + std::string(1, (char)('A' + lIdx));
            lNewSeries.mValues = generateProfileValues(7, randFloat(50.0f, 80.0f), 20.0f);
            lNewSeries.mLineColor = SERIES_COLORS[lIdx % 5];
            lNewSeries.mFillColor = withAlpha(SERIES_COLORS[lIdx % 5], lShowFill ? 40 : 0);
            lNewSeries.mLineThickness = 2.0f;
            lNewSeries.mShowFill = lShowFill;
            lNewSeries.mShowMarkers = lShowMarkers;
            lChart2.addSeries(lNewSeries);
        }

        if (IsKeyPressed(KEY_R) && lChart2.getSeriesCount() > 1) {
            lChart2.removeSeries(lChart2.getSeriesCount() - 1);
        }

        // ====================================================================
        // Automatic data updates
        // ====================================================================
        if (lTimer >= UPDATE_INTERVAL) {
            lTimer = 0.0f;

            // Update Chart 2 series values (smooth morphing)
            for (size_t i = 0; i < lChart2.getSeriesCount(); ++i) {
                std::vector<float> lNewValues = generateProfileValues(7, BASE_VALUES[i % 3], 20.0f);
                lChart2.setSeriesData(i, lNewValues);
            }
        }

        // Periodic series add/remove demo (every 8 seconds)
        if (lAddRemoveTimer >= 8.0f) {
            lAddRemoveTimer = 0.0f;
            lAddingRemovingSeries = !lAddingRemovingSeries;

            if (lAddingRemovingSeries && lChart2.getSeriesCount() < 4) {
                // Add a new series
                size_t lIdx = lChart2.getSeriesCount();
                RLRadarSeries lNewSeries;
                lNewSeries.mLabel = "Product " + std::string(1, (char)('A' + lIdx));
                lNewSeries.mValues = generateProfileValues(7, randFloat(55.0f, 75.0f), 18.0f);
                lNewSeries.mLineColor = SERIES_COLORS[lIdx % 5];
                lNewSeries.mFillColor = withAlpha(SERIES_COLORS[lIdx % 5], lShowFill ? 40 : 0);
                lNewSeries.mLineThickness = 2.0f;
                lNewSeries.mShowFill = lShowFill;
                lNewSeries.mShowMarkers = lShowMarkers;
                lChart2.addSeries(lNewSeries);
            } else if (!lAddingRemovingSeries && lChart2.getSeriesCount() > 2) {
                // Remove a series
                lChart2.removeSeries(lChart2.getSeriesCount() - 1);
            }
        }

        // ====================================================================
        // Update charts
        // ====================================================================
        lChart1.update(lDt);
        lChart2.update(lDt);

        // ====================================================================
        // Render
        // ====================================================================
        BeginDrawing();
        ClearBackground(Color{12, 14, 18, 255});

        // Draw title
        const char* lTitle = "RLRadarChart Demo - Radar/Spider Charts";
        int lTitleWidth = MeasureText(lTitle, 24);
        DrawTextEx(lFont, lTitle, Vector2{(float)((SCREEN_WIDTH - lTitleWidth) / 2), 18}, 24, 1.0f, Color{200, 210, 230, 255});

        // Draw chart titles
        DrawTextEx(lFont, "Single Series - Character Profile",
                   Vector2{lBounds1.x + 10.0f, lBounds1.y - 28.0f}, 16.0f, 1.0f,
                   Color{160, 170, 190, 255});

        DrawTextEx(lFont, "Multi-Series - Product Comparison",
                   Vector2{lBounds2.x + 10.0f, lBounds2.y - 28.0f}, 16.0f, 1.0f,
                   Color{160, 170, 190, 255});

        // Draw charts
        lChart1.draw();
        lChart2.draw();

        // Draw instructions
        const char* lInstructions =
            "Controls:  [SPACE] Cycle profiles  |  [F] Toggle fill  |  [M] Toggle markers  |  "
            "[A] Add series  |  [R] Remove series";
        int lInstrWidth = MeasureText(lInstructions, 12);
        DrawTextEx(lFont, lInstructions, Vector2{(float)((SCREEN_WIDTH - lInstrWidth) / 2), (float)(SCREEN_HEIGHT - 25)},
                 12, 1.0f, Color{120, 130, 150, 255});

        // Draw current profile name for Chart 1
        char lProfileText[64];
        snprintf(lProfileText, sizeof(lProfileText), "Current: %s", PROFILE_NAMES[lDatasetIndex]);
        DrawTextEx(lFont, lProfileText,
                   Vector2{lBounds1.x + lBounds1.width - 120.0f, lBounds1.y + lBounds1.height - 25.0f},
                   14.0f, 1.0f, Color{140, 150, 170, 255});

        // Draw series count for Chart 2
        char lSeriesText[64];
        snprintf(lSeriesText, sizeof(lSeriesText), "Series: %zu", lChart2.getSeriesCount());
        DrawTextEx(lFont, lSeriesText,
                   Vector2{lBounds2.x + lBounds2.width - 80.0f, lBounds2.y + lBounds2.height - 25.0f},
                   14.0f, 1.0f, Color{140, 150, 170, 255});

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}

