// treemap.cpp
// TreeMap Visualization Demo
// Demonstrates D3-style treemap with hierarchical data and animations
#include "RLTreeMap.h"
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

// Helper to get palette color
Color paletteColor(int aIndex) {
    const Color PALETTE[] = {
        Color{0, 170, 230, 255},
        Color{70, 200, 110, 255},
        Color{245, 130, 70, 255},
        Color{230, 75, 100, 255},
        Color{150, 100, 220, 255},
        Color{240, 200, 50, 255},
        Color{70, 190, 180, 255},
        Color{200, 100, 180, 255},
        Color{100, 150, 220, 255},
        Color{180, 210, 80, 255}
    };
    return PALETTE[aIndex % 10];
}

// Create a sample file system hierarchy
RLTreeNode createFileSystemData() {
    RLTreeNode lRoot;
    lRoot.mLabel = "Root";

    // Documents folder
    RLTreeNode lDocs;
    lDocs.mLabel = "Documents";

    RLTreeNode lWork;
    lWork.mLabel = "Work";
    lWork.mChildren.push_back({"Report.pdf", 150.0f, paletteColor(0), true, {}});
    lWork.mChildren.push_back({"Presentation.pptx", 280.0f, paletteColor(0), true, {}});
    lWork.mChildren.push_back({"Spreadsheet.xlsx", 95.0f, paletteColor(0), true, {}});
    lWork.mChildren.push_back({"Notes.txt", 12.0f, paletteColor(0), true, {}});
    lDocs.mChildren.push_back(lWork);

    RLTreeNode lPersonal;
    lPersonal.mLabel = "Personal";
    lPersonal.mChildren.push_back({"Resume.pdf", 45.0f, paletteColor(1), true, {}});
    lPersonal.mChildren.push_back({"Budget.xlsx", 78.0f, paletteColor(1), true, {}});
    lPersonal.mChildren.push_back({"Ideas.txt", 8.0f, paletteColor(1), true, {}});
    lDocs.mChildren.push_back(lPersonal);

    lRoot.mChildren.push_back(lDocs);

    // Photos folder
    RLTreeNode lPhotos;
    lPhotos.mLabel = "Photos";

    RLTreeNode lVacation;
    lVacation.mLabel = "Vacation 2024";
    for (int i = 0; i < 6; ++i) {
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "IMG_%04d.jpg", i + 1);
        lVacation.mChildren.push_back({lBuf, frandRange(2.0f, 8.0f) * 1000.0f, paletteColor(2), true, {}});
    }
    lPhotos.mChildren.push_back(lVacation);

    RLTreeNode lFamily;
    lFamily.mLabel = "Family";
    for (int i = 0; i < 4; ++i) {
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "Photo_%d.png", i + 1);
        lFamily.mChildren.push_back({lBuf, frandRange(1.5f, 5.0f) * 1000.0f, paletteColor(3), true, {}});
    }
    lPhotos.mChildren.push_back(lFamily);

    lRoot.mChildren.push_back(lPhotos);

    // Code folder
    RLTreeNode lCode;
    lCode.mLabel = "Code";

    RLTreeNode lProject1;
    lProject1.mLabel = "cpp-charts";
    lProject1.mChildren.push_back({"main.cpp", 320.0f, paletteColor(4), true, {}});
    lProject1.mChildren.push_back({"RLTreeMap.cpp", 580.0f, paletteColor(4), true, {}});
    lProject1.mChildren.push_back({"RLTreeMap.h", 180.0f, paletteColor(4), true, {}});
    lProject1.mChildren.push_back({"CMakeLists.txt", 45.0f, paletteColor(4), true, {}});
    lCode.mChildren.push_back(lProject1);

    RLTreeNode lProject2;
    lProject2.mLabel = "web-app";
    lProject2.mChildren.push_back({"index.html", 120.0f, paletteColor(5), true, {}});
    lProject2.mChildren.push_back({"styles.css", 85.0f, paletteColor(5), true, {}});
    lProject2.mChildren.push_back({"app.js", 450.0f, paletteColor(5), true, {}});
    lProject2.mChildren.push_back({"package.json", 15.0f, paletteColor(5), true, {}});
    lCode.mChildren.push_back(lProject2);

    lRoot.mChildren.push_back(lCode);

    // Downloads folder
    RLTreeNode lDownloads;
    lDownloads.mLabel = "Downloads";
    lDownloads.mChildren.push_back({"installer.exe", 45000.0f, paletteColor(6), true, {}});
    lDownloads.mChildren.push_back({"movie.mp4", 85000.0f, paletteColor(6), true, {}});
    lDownloads.mChildren.push_back({"archive.zip", 12000.0f, paletteColor(6), true, {}});
    lDownloads.mChildren.push_back({"document.pdf", 850.0f, paletteColor(6), true, {}});

    lRoot.mChildren.push_back(lDownloads);

    // Music folder
    RLTreeNode lMusic;
    lMusic.mLabel = "Music";

    RLTreeNode lAlbum1;
    lAlbum1.mLabel = "Album A";
    for (int i = 0; i < 8; ++i) {
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "Track %02d.mp3", i + 1);
        lAlbum1.mChildren.push_back({lBuf, frandRange(3.0f, 7.0f) * 1000.0f, paletteColor(7), true, {}});
    }
    lMusic.mChildren.push_back(lAlbum1);

    RLTreeNode lAlbum2;
    lAlbum2.mLabel = "Album B";
    for (int i = 0; i < 5; ++i) {
        char lBuf[32];
        snprintf(lBuf, sizeof(lBuf), "Song %02d.flac", i + 1);
        lAlbum2.mChildren.push_back({lBuf, frandRange(15.0f, 30.0f) * 1000.0f, paletteColor(8), true, {}});
    }
    lMusic.mChildren.push_back(lAlbum2);

    lRoot.mChildren.push_back(lMusic);

    return lRoot;
}

// Create a sales data hierarchy
RLTreeNode createSalesData() {
    RLTreeNode lRoot;
    lRoot.mLabel = "Sales 2024";

    const char* lRegions[] = {"North America", "Europe", "Asia Pacific", "Latin America"};
    const char* lProducts[] = {"Software", "Hardware", "Services", "Support"};

    for (int lR = 0; lR < 4; ++lR) {
        RLTreeNode lRegion;
        lRegion.mLabel = lRegions[lR];

        for (int lP = 0; lP < 4; ++lP) {
            float lValue = frandRange(50.0f, 500.0f) * (1.0f + (float)(3 - lR) * 0.3f);
            lRegion.mChildren.push_back({lProducts[lP], lValue, paletteColor(lR * 4 + lP), true, {}});
        }

        lRoot.mChildren.push_back(lRegion);
    }

    return lRoot;
}

// Create budget allocation data
RLTreeNode createBudgetData() {
    RLTreeNode lRoot;
    lRoot.mLabel = "Budget";

    // Engineering
    RLTreeNode lEng;
    lEng.mLabel = "Engineering";
    lEng.mChildren.push_back({"Salaries", 850.0f, paletteColor(0), true, {}});
    lEng.mChildren.push_back({"Equipment", 120.0f, paletteColor(0), true, {}});
    lEng.mChildren.push_back({"Software", 80.0f, paletteColor(0), true, {}});
    lEng.mChildren.push_back({"Training", 45.0f, paletteColor(0), true, {}});
    lRoot.mChildren.push_back(lEng);

    // Marketing
    RLTreeNode lMkt;
    lMkt.mLabel = "Marketing";
    lMkt.mChildren.push_back({"Advertising", 320.0f, paletteColor(1), true, {}});
    lMkt.mChildren.push_back({"Events", 180.0f, paletteColor(1), true, {}});
    lMkt.mChildren.push_back({"Content", 95.0f, paletteColor(1), true, {}});
    lMkt.mChildren.push_back({"PR", 65.0f, paletteColor(1), true, {}});
    lRoot.mChildren.push_back(lMkt);

    // Operations
    RLTreeNode lOps;
    lOps.mLabel = "Operations";
    lOps.mChildren.push_back({"Facilities", 220.0f, paletteColor(2), true, {}});
    lOps.mChildren.push_back({"IT Infra", 180.0f, paletteColor(2), true, {}});
    lOps.mChildren.push_back({"Legal", 95.0f, paletteColor(2), true, {}});
    lOps.mChildren.push_back({"Admin", 75.0f, paletteColor(2), true, {}});
    lRoot.mChildren.push_back(lOps);

    // R&D
    RLTreeNode lRnd;
    lRnd.mLabel = "R&D";
    lRnd.mChildren.push_back({"Research", 280.0f, paletteColor(3), true, {}});
    lRnd.mChildren.push_back({"Prototypes", 150.0f, paletteColor(3), true, {}});
    lRnd.mChildren.push_back({"Testing", 95.0f, paletteColor(3), true, {}});
    lRoot.mChildren.push_back(lRnd);

    return lRoot;
}

int main() {
    // Disable verbose raylib logging
    SetTraceLogLevel(LOG_WARNING);

    seedFast((uint32_t)time(nullptr));

    const int SCREEN_WIDTH = 1400;
    const int SCREEN_HEIGHT = 900;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TreeMap Visualization - D3-Style Layout Demo");
    SetTargetFPS(60);

    // Load custom font
    Font lBaseFont = LoadFontEx("base.ttf", 24, nullptr, 250);

    // Create treemap bounds
    Rectangle lTreeMapBounds = {50, 100, 900, 700};
    Rectangle lInfoBounds = {980, 100, 370, 700};

    // Configure style
    RLTreeMapStyle lStyle;
    lStyle.mBackground = Color{15, 17, 22, 255};
    lStyle.mShowBackground = true;
    lStyle.mPaddingOuter = 6.0f;
    lStyle.mPaddingInner = 3.0f;
    lStyle.mPaddingTop = 20.0f;
    lStyle.mBorderThickness = 1.0f;
    lStyle.mBorderColor = Color{40, 44, 52, 255};
    lStyle.mCornerRadius = 4.0f;
    lStyle.mShowInternalNodes = true;
    lStyle.mInternalNodeColor = Color{30, 34, 42, 220};
    lStyle.mShowInternalLabels = true;
    lStyle.mShowLeafLabels = true;
    lStyle.mMinNodeSize = 12.0f;
    lStyle.mLabelFitCheck = true;
    lStyle.mLabelFontSize = 12;
    lStyle.mLabelColor = Color{220, 220, 230, 255};
    lStyle.mAutoLabelColor = true;
    lStyle.mSmoothAnimate = true;
    lStyle.mAnimateSpeed = 5.0f;
    lStyle.mColorSpeed = 3.0f;
    lStyle.mUseDepthColors = false;  // Use custom colors from data
    lStyle.mLabelFont = lBaseFont;

    RLTreeMap lTreeMap(lTreeMapBounds, lStyle);
    lTreeMap.setLayout(RLTreeMapLayout::SQUARIFIED);

    // Create datasets
    std::vector<RLTreeNode> lDatasets;
    lDatasets.push_back(createFileSystemData());
    lDatasets.push_back(createSalesData());
    lDatasets.push_back(createBudgetData());

    const char* lDatasetNames[] = {"File System", "Sales Data", "Budget Allocation"};
    int lCurrentDataset = 0;

    // Set initial data
    lTreeMap.setData(lDatasets[lCurrentDataset]);

    // State
    RLTreeMapLayout lCurrentLayout = RLTreeMapLayout::SQUARIFIED;
    const char* lLayoutNames[] = {"Squarified", "Slice", "Dice", "Slice-Dice"};
    int lLayoutIndex = 0;

    bool lShowInternalNodes = true;
    bool lShowLabels = true;
    bool lAnimateValues = false;
    float lAnimateTimer = 0.0f;

    // Main loop
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();

        // Handle input
        if (IsKeyPressed(KEY_ONE)) {
            lCurrentDataset = 0;
            lTreeMap.setTargetData(lDatasets[lCurrentDataset]);
        }
        if (IsKeyPressed(KEY_TWO)) {
            lCurrentDataset = 1;
            lTreeMap.setTargetData(lDatasets[lCurrentDataset]);
        }
        if (IsKeyPressed(KEY_THREE)) {
            lCurrentDataset = 2;
            lTreeMap.setTargetData(lDatasets[lCurrentDataset]);
        }

        if (IsKeyPressed(KEY_L)) {
            lLayoutIndex = (lLayoutIndex + 1) % 4;
            lCurrentLayout = (RLTreeMapLayout)lLayoutIndex;
            lTreeMap.setLayout(lCurrentLayout);
            lTreeMap.recomputeLayout();
        }

        if (IsKeyPressed(KEY_I)) {
            lShowInternalNodes = !lShowInternalNodes;
            lStyle.mShowInternalNodes = lShowInternalNodes;
            lStyle.mShowInternalLabels = lShowInternalNodes;
            lTreeMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_T)) {
            lShowLabels = !lShowLabels;
            lStyle.mShowLeafLabels = lShowLabels;
            lTreeMap.setStyle(lStyle);
        }

        if (IsKeyPressed(KEY_A)) {
            lAnimateValues = !lAnimateValues;
        }

        // Animate values periodically
        if (lAnimateValues) {
            lAnimateTimer += lDt;
            if (lAnimateTimer > 1.5f) {
                lAnimateTimer = 0.0f;

                // Randomly modify some values in the current dataset
                RLTreeNode lModified = lDatasets[lCurrentDataset];
                for (auto& rChild : lModified.mChildren) {
                    for (auto& rLeaf : rChild.mChildren) {
                        if (rLeaf.mChildren.empty()) {
                            float lChange = frandRange(0.7f, 1.4f);
                            rLeaf.mValue *= lChange;
                        } else {
                            for (auto& rSubLeaf : rLeaf.mChildren) {
                                if (rSubLeaf.mChildren.empty()) {
                                    float lChange = frandRange(0.8f, 1.25f);
                                    rSubLeaf.mValue *= lChange;
                                }
                            }
                        }
                    }
                }
                lTreeMap.setTargetData(lModified);
            }
        }

        // Mouse hover highlighting
        Vector2 lMousePos = GetMousePosition();
        int lHoveredNode = lTreeMap.getNodeAtPoint(lMousePos);
        lTreeMap.setHighlightedNode(lHoveredNode);

        // Update treemap
        lTreeMap.update(lDt);

        // Draw
        BeginDrawing();
        ClearBackground(Color{10, 12, 18, 255});

        // Title
        DrawTextEx(lBaseFont, "TreeMap Visualization", Vector2{50, 20}, 30, 1.0f, Color{220, 220, 230, 255});
        DrawTextEx(lBaseFont, "D3-Style Squarified Layout", Vector2{50, 55}, 18, 1.0f, Color{140, 140, 150, 255});

        // Draw treemap
        lTreeMap.draw();

        // Info panel
        DrawRectangle((int)lInfoBounds.x, (int)lInfoBounds.y, (int)lInfoBounds.width, (int)lInfoBounds.height, Color{20, 22, 28, 255});
        DrawRectangleLinesEx(lInfoBounds, 1.0f, Color{40, 44, 52, 255});

        int lInfoY = (int)lInfoBounds.y + 20;
        int lInfoX = (int)lInfoBounds.x + 20;
        int lLineH = 26;

        DrawTextEx(lBaseFont, "TREEMAP INFO", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        char lBuf[128];
        snprintf(lBuf, sizeof(lBuf), "Dataset: %s", lDatasetNames[lCurrentDataset]);
        DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
        lInfoY += lLineH;

        snprintf(lBuf, sizeof(lBuf), "Layout: %s", lLayoutNames[lLayoutIndex]);
        DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
        lInfoY += lLineH;

        snprintf(lBuf, sizeof(lBuf), "Node Count: %zu", lTreeMap.getNodeCount());
        DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
        lInfoY += lLineH + 15;

        // Hovered node info
        DrawTextEx(lBaseFont, "HOVERED NODE", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        if (lHoveredNode >= 0) {
            const std::vector<RLTreeRect>& lRects = lTreeMap.getComputedRects();
            if ((size_t)lHoveredNode < lRects.size()) {
                const RLTreeRect& lHovered = lRects[(size_t)lHoveredNode];

                snprintf(lBuf, sizeof(lBuf), "Label: %s", lHovered.mLabel.c_str());
                DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
                lInfoY += lLineH;

                snprintf(lBuf, sizeof(lBuf), "Value: %.1f", (double)lHovered.mValue);
                DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
                lInfoY += lLineH;

                snprintf(lBuf, sizeof(lBuf), "Depth: %d", lHovered.mDepth);
                DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
                lInfoY += lLineH;

                snprintf(lBuf, sizeof(lBuf), "Type: %s", lHovered.mIsLeaf ? "Leaf" : "Internal");
                DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
                lInfoY += lLineH;

                snprintf(lBuf, sizeof(lBuf), "Size: %.0f x %.0f", (double)lHovered.mRect.width, (double)lHovered.mRect.height);
                DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, Color{180, 180, 190, 255});
                lInfoY += lLineH;
            }
        } else {
            DrawTextEx(lBaseFont, "(hover over a node)", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{120, 120, 130, 255});
            lInfoY += lLineH;
        }

        lInfoY += 20;

        // Controls
        DrawTextEx(lBaseFont, "CONTROLS", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        DrawTextEx(lBaseFont, "[1/2/3]  Switch dataset", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lBaseFont, "[L]      Cycle layout algorithm", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lBaseFont, "[I]      Toggle internal nodes", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lBaseFont, "[T]      Toggle leaf labels", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lBaseFont, "[A]      Toggle value animation", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 22;
        DrawTextEx(lBaseFont, "[Mouse]  Hover to highlight", Vector2{(float)lInfoX, (float)lInfoY}, 14, 1.0f, Color{140, 140, 150, 255});
        lInfoY += 30;

        // Status
        DrawTextEx(lBaseFont, "STATUS", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        const char* lInternalStatus = lShowInternalNodes ? "Visible" : "Hidden";
        Color lInternalColor = lShowInternalNodes ? Color{80, 220, 120, 255} : Color{180, 180, 190, 255};
        snprintf(lBuf, sizeof(lBuf), "Internal Nodes: %s", lInternalStatus);
        DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, lInternalColor);
        lInfoY += lLineH;

        const char* lLabelStatus = lShowLabels ? "Visible" : "Hidden";
        Color lLabelColor = lShowLabels ? Color{80, 220, 120, 255} : Color{180, 180, 190, 255};
        snprintf(lBuf, sizeof(lBuf), "Leaf Labels: %s", lLabelStatus);
        DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, lLabelColor);
        lInfoY += lLineH;

        const char* lAnimStatus = lAnimateValues ? "ACTIVE" : "Off";
        Color lAnimColor = lAnimateValues ? Color{255, 180, 80, 255} : Color{180, 180, 190, 255};
        snprintf(lBuf, sizeof(lBuf), "Value Animation: %s", lAnimStatus);
        DrawTextEx(lBaseFont, lBuf, Vector2{(float)lInfoX, (float)lInfoY}, 16, 1.0f, lAnimColor);
        lInfoY += lLineH + 5;

        // Algorithm info
        DrawTextEx(lBaseFont, "LAYOUT ALGORITHM", Vector2{(float)lInfoX, (float)lInfoY}, 20, 1.0f, Color{200, 200, 210, 255});
        lInfoY += lLineH + 10;

        const char* lAlgoDesc[] = {
            "Squarified: Optimizes for\nsquare-like aspect ratios",
            "Slice: Divides vertically,\nstacking rows",
            "Dice: Divides horizontally,\nstacking columns",
            "Slice-Dice: Alternates\nby tree depth"
        };

        DrawTextEx(lBaseFont, lAlgoDesc[lLayoutIndex], Vector2{(float)lInfoX, (float)lInfoY}, 13, 1.0f, Color{130, 130, 140, 255});

        // FPS
        DrawFPS(SCREEN_WIDTH - 100, 10);

        EndDrawing();
    }

    UnloadFont(lBaseFont);
    CloseWindow();
    return 0;
}

