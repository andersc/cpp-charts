// RLTooltip.h
#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <cstdint>

// Shared tooltip rendering utility for all chart types

struct RLTooltipEntry {
    std::string mLabel;
    std::string mValue;
    Color mColor{200, 200, 210, 255};
};

struct RLTooltipStyle {
    Color mBackground{30, 33, 40, 230};
    Color mBorder{80, 85, 95, 255};
    Color mTitleColor{240, 240, 250, 255};
    Color mTextColor{200, 200, 210, 255};
    float mFontSize{14.0f};
    float mPadding{8.0f};
    float mBorderWidth{1.0f};
    float mCornerRadius{4.0f};
    float mSpacing{4.0f};
    float mCursorOffset{16.0f};
    Font mFont{};
};

namespace RLTooltip {

inline Vector2 measureText(const std::string& rText, float aFontSize, const Font& rFont) {
    if (rFont.texture.id == 0) {
        auto lW = (float)MeasureText(rText.c_str(), (int32_t)aFontSize);
        return {lW, aFontSize};
    }
    return MeasureTextEx(rFont, rText.c_str(), aFontSize, 1.0f);
}

inline void drawText(const std::string& rText, Vector2 aPos, float aFontSize,
                     Color aColor, const Font& rFont) {
    if (rFont.texture.id == 0) {
        DrawText(rText.c_str(), (int32_t)aPos.x, (int32_t)aPos.y, (int32_t)aFontSize, aColor);
    } else {
        DrawTextEx(rFont, rText.c_str(), aPos, aFontSize, 1.0f, aColor);
    }
}

inline void draw(const std::string& rTitle,
                 const std::vector<RLTooltipEntry>& rEntries,
                 Vector2 aScreenPos,
                 const RLTooltipStyle& rStyle = {}) {
    auto lFontSize = rStyle.mFontSize;
    auto lPad = rStyle.mPadding;
    auto lSpacing = rStyle.mSpacing;
    auto lTitleSize = measureText(rTitle, lFontSize, rStyle.mFont);

    // Measure all entries to find maximum width
    auto lMaxW = lTitleSize.x;
    for (const auto& rEntry : rEntries) {
        auto lText = rEntry.mLabel + ": " + rEntry.mValue;
        auto lSize = measureText(lText, lFontSize - 2.0f, rStyle.mFont);
        if (lSize.x > lMaxW) {
            lMaxW = lSize.x;
        }
    }

    auto lBoxW = lMaxW + lPad * 2.0f;
    auto lLineH = lFontSize + lSpacing;
    auto lBoxH = lPad + lTitleSize.y + lSpacing
                 + (float)rEntries.size() * lLineH + lPad;

    // Position near cursor with offset
    auto lX = aScreenPos.x + rStyle.mCursorOffset;
    auto lY = aScreenPos.y + rStyle.mCursorOffset;

    // Clamp to screen bounds
    auto lScreenW = (float)GetScreenWidth();
    auto lScreenH = (float)GetScreenHeight();
    if (lX + lBoxW > lScreenW) {
        lX = aScreenPos.x - lBoxW - rStyle.mCursorOffset * 0.5f;
    }
    if (lY + lBoxH > lScreenH) {
        lY = aScreenPos.y - lBoxH - rStyle.mCursorOffset * 0.5f;
    }
    if (lX < 0.0f) lX = 0.0f;
    if (lY < 0.0f) lY = 0.0f;

    auto lRect = Rectangle{lX, lY, lBoxW, lBoxH};

    // Draw background and border
    DrawRectangleRounded(lRect, 0.15f, 4, rStyle.mBackground);
    DrawRectangleRoundedLines(lRect, 0.15f, 4, rStyle.mBorder);

    // Draw title
    auto lTextX = lX + lPad;
    auto lTextY = lY + lPad;
    drawText(rTitle, {lTextX, lTextY}, lFontSize, rStyle.mTitleColor, rStyle.mFont);
    lTextY += lTitleSize.y + lSpacing;

    // Draw entries
    auto lEntryFontSize = lFontSize - 2.0f;
    for (const auto& rEntry : rEntries) {
        auto lText = rEntry.mLabel + ": " + rEntry.mValue;
        drawText(lText, {lTextX, lTextY}, lEntryFontSize, rEntry.mColor, rStyle.mFont);
        lTextY += lLineH;
    }
}

} // namespace RLTooltip
