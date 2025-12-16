// candlestick2.cpp - Demo with 1 value per candlestick
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "RLCandlestickChart.h"

struct CSVRow {
    std::string aDate;
    float aOpen{0.0f};
    float aHigh{0.0f};
    float aLow{0.0f};
    float aClose{0.0f};
    float aVolume{0.0f};
    int aBarCount{0};
    int aOfBars{0};
};

static bool parseRow(const std::string &aLine, CSVRow &rOut){
    // Expect: date,open,high,low,close,volume,bar_count,of_bars
    std::vector<std::string> lParts;
    lParts.reserve(8);
    std::stringstream lSS(aLine);
    std::string lItem;
    while (std::getline(lSS, lItem, ',')) lParts.push_back(lItem);
    if (lParts.size() < 6) return false;
    rOut.aDate = lParts[0];
    try {
        rOut.aOpen = std::stof(lParts[1]);
        rOut.aHigh = std::stof(lParts[2]);
        rOut.aLow = std::stof(lParts[3]);
        rOut.aClose = std::stof(lParts[4]);
        rOut.aVolume = std::stof(lParts[5]);
        if (lParts.size() >= 7) rOut.aBarCount = std::stoi(lParts[6]);
        if (lParts.size() >= 8) rOut.aOfBars = std::stoi(lParts[7]);
    } catch (...) { return false; }
    return true;
}

static std::vector<CSVRow> loadCSV(const std::string &aPath){
    std::ifstream lIn(aPath);
    std::vector<CSVRow> lRows;
    if (!lIn.is_open()) return lRows;
    std::string lLine;
    // Optional header: detect if first line has alpha characters
    if (std::getline(lIn, lLine)){
        bool lHasAlpha = false;
        for (char c : lLine){ if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')) { lHasAlpha = true; break; } }
        if (!lHasAlpha){
            CSVRow lRow{};
            if (parseRow(lLine, lRow)) lRows.push_back(lRow);
        }
    }
    while (std::getline(lIn, lLine)){
        CSVRow lRow{};
        if (parseRow(lLine, lRow)) lRows.push_back(lRow);
    }
    return lRows;
}

static std::string resolveCSVPath(){
    // Try common paths relative to typical CMake build dirs
    // Note: For WASM builds, the file is preloaded at root path
    const char* lCandidates[] = {
        "JPM_1_minute_bars.csv",
        "./JPM_1_minute_bars.csv",
        "../JPM_1_minute_bars.csv",
        "../../JPM_1_minute_bars.csv",
        "../../../JPM_1_minute_bars.csv"
    };
    for (const char* lPath : lCandidates){
        std::ifstream lTry(lPath);
        if (lTry.is_open()) {
            std::cerr << "Found CSV at: " << lPath << "\n";
            return std::string(lPath);
        }
    }
    std::cerr << "CSV not found in any of the candidate paths\n";
    return std::string();
}

int main(){
    const int lScreenW = 1280;
    const int lScreenH = 800;
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(lScreenW, lScreenH, "Raylib Candlestick Demo - 1 Value Per Candle");

    Font lFont = LoadFontEx("base.ttf", 20, nullptr, 250);

    std::string lCsvPath = resolveCSVPath();
    if (lCsvPath.empty()) {
        std::cerr << "Could not locate JPM_1_minute_bars.csv. Place it next to the executable or project root.\n";
    } else {
        std::cerr << "Loading CSV from: " << lCsvPath << "\n";
    }
    std::vector<CSVRow> lData = lCsvPath.empty() ? std::vector<CSVRow>{} : loadCSV(lCsvPath);
    std::cerr << "Loaded rows: " << lData.size() << "\n";
    size_t lCursor = 0;

    // Create styles
    RLCandleStyle lStyleDefault{};
    RLCandleStyle lStyleAlt = lStyleDefault;
    lStyleAlt.mUpBody = {90, 200, 255, 255};
    lStyleAlt.mDownBody = {255, 140, 100, 255};
    lStyleAlt.mVolumeUp = {90, 200, 255, 150};
    lStyleAlt.mVolumeDown = {255, 140, 100, 150};

    // Layout three demo charts stacked - ALL WITH 1 VALUE PER CANDLE
    float lPad = 12.0f;
    float lH = (lScreenH - lPad*4) / 3.0f;
    Rectangle lR1{ lPad, lPad, lScreenW - 2*lPad, lH };
    Rectangle lR2{ lPad, lPad*2 + lH, lScreenW - 2*lPad, lH };
    Rectangle lR3{ lPad, lPad*3 + lH*2, lScreenW - 2*lPad, lH };

    RLCandlestickChart lChart1(lR1, 1, 30, lStyleDefault); // 1 value per candle, 30 visible
    RLCandlestickChart lChart2(lR2, 1, 50, lStyleAlt);     // 1 value per candle, 50 visible (more candles)
    RLCandlestickChart lChart3(lR3, 1, 20, lStyleDefault); // 1 value per candle, 20 visible (fewer candles)

    float lAccum = 0.0f;

    while (!WindowShouldClose()){
        float lDt = GetFrameTime();
        lAccum += lDt;

        // Feed one CSV row per 0.5 seconds for faster demo
        if (lAccum >= 0.5f && lCursor < lData.size()){
            lAccum -= 0.5f;
            const CSVRow &lRow = lData[lCursor++];
            RLCandlestickChart::CandleInput lIn{};
            lIn.aDate = lRow.aDate;
            lIn.aOpen = lRow.aOpen;
            lIn.aHigh = lRow.aHigh;
            lIn.aLow = lRow.aLow;
            lIn.aClose = lRow.aClose;
            lIn.aVolume = lRow.aVolume;
            lChart1.addSample(lIn);
            lChart2.addSample(lIn);
            lChart3.addSample(lIn);
        }

        lChart1.update(lDt);
        lChart2.update(lDt);
        lChart3.update(lDt);

        BeginDrawing();
        ClearBackground({12, 14, 18, 255});

        lChart1.draw();
        lChart2.draw();
        lChart3.draw();

        // Titles
        DrawTextEx(lFont, "valuesPerCandle=1, visible=30", Vector2{lR1.x + 10, lR1.y + 10}, 18, 1.0f, {220, 220, 230, 200});
        DrawTextEx(lFont, "valuesPerCandle=1, visible=50 (alt colors)", Vector2{lR2.x + 10, lR2.y + 10}, 18, 1.0f, {220, 220, 230, 200});
        DrawTextEx(lFont, "valuesPerCandle=1, visible=20", Vector2{lR3.x + 10, lR3.y + 10}, 18, 1.0f, {220, 220, 230, 200});

        // Status
        if (lData.empty()){
            DrawTextEx(lFont, "CSV not found or empty. Place JPM_1_minute_bars.csv in project root or build dir.", Vector2{20, (float)(lScreenH - 28)}, 20, 1.0f, {255, 120, 120, 255});
        } else if (lCursor >= lData.size()){
            DrawTextEx(lFont, "End of CSV reached", Vector2{20, (float)(lScreenH - 28)}, 20, 1.0f, {200, 200, 210, 255});
        } else {
            DrawTextEx(lFont, "Streaming 1 row per 0.5sec (1 value per candle) from JPM_1_minute_bars.csv", Vector2{20, (float)(lScreenH - 28)}, 20, 1.0f, {200, 200, 210, 255});
        }

        EndDrawing();
    }

    UnloadFont(lFont);
    CloseWindow();
    return 0;
}
