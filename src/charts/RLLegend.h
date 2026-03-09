// RLLegend.h
#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <cstdint>

// Shared legend rendering utility for all chart types

struct RLLegendEntry {
    std::string mLabel;
    Color mColor{200, 200, 210, 255};
    float mVisibility{1.0f};
};

struct RLLegendStyle {
    Color mTextColor{200, 200, 210, 255};
    Color mBackground{0, 0, 0, 0};
    float mFontSize{12.0f};
    float mPadding{8.0f};
    float mSpacing{4.0f};
    float mIndicatorSize{10.0f};
    Font mFont{};
};

namespace RLLegend {

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

inline Vector2 measure(const std::vector<RLLegendEntry>& rEntries,
                       const RLLegendStyle& rStyle = {}) {
    auto lMaxW = 0.0f;
    auto lCount = 0;
    for (const auto& rEntry : rEntries) {
        if (rEntry.mVisibility < 0.01f || rEntry.mLabel.empty()) continue;
        auto lSize = measureText(rEntry.mLabel, rStyle.mFontSize, rStyle.mFont);
        auto lEntryW = rStyle.mIndicatorSize + rStyle.mSpacing + lSize.x;
        if (lEntryW > lMaxW) lMaxW = lEntryW;
        ++lCount;
    }
    auto lLineH = rStyle.mIndicatorSize + rStyle.mSpacing;
    return {lMaxW + rStyle.mPadding * 2.0f,
            (float)lCount * lLineH + rStyle.mPadding * 2.0f};
}

inline void draw(const std::vector<RLLegendEntry>& rEntries,
                 Rectangle aBounds,
                 const RLLegendStyle& rStyle = {}) {
    auto lSize = measure(rEntries, rStyle);
    auto lX = aBounds.x + aBounds.width - lSize.x - rStyle.mPadding;
    auto lY = aBounds.y + rStyle.mPadding;

    // Draw background if non-transparent
    if (rStyle.mBackground.a > 0) {
        DrawRectangleRounded({lX, lY, lSize.x, lSize.y}, 0.1f, 4, rStyle.mBackground);
    }

    auto lEntryX = lX + rStyle.mPadding;
    auto lEntryY = lY + rStyle.mPadding;
    auto lIndicator = rStyle.mIndicatorSize;
    auto lLineH = lIndicator + rStyle.mSpacing;

    for (const auto& rEntry : rEntries) {
        if (rEntry.mVisibility < 0.01f || rEntry.mLabel.empty()) continue;

        auto lBoxColor = rEntry.mColor;
        lBoxColor.a = (uint8_t)((float)lBoxColor.a * rEntry.mVisibility);
        DrawRectangle((int32_t)lEntryX, (int32_t)lEntryY,
                      (int32_t)lIndicator, (int32_t)lIndicator, lBoxColor);

        auto lTextColor = rStyle.mTextColor;
        lTextColor.a = (uint8_t)((float)lTextColor.a * rEntry.mVisibility);
        auto lTextPos = Vector2{lEntryX + lIndicator + rStyle.mSpacing, lEntryY};
        drawText(rEntry.mLabel, lTextPos, rStyle.mFontSize, lTextColor, rStyle.mFont);

        lEntryY += lLineH;
    }
}

} // namespace RLLegend
