// RLTheme.h
#pragma once
#include "raylib.h"
#include <cstdint>
#include <array>

// Global theming for cpp-charts.
// Define color roles once, then apply across any chart with applyTheme().

struct RLChartTheme {
    Color mBackground{20, 22, 28, 255};       // Chart background
    Color mForeground{220, 220, 230, 255};     // Primary text / labels
    Color mForegroundDim{150, 155, 165, 255};  // Secondary text / tick labels
    Color mGrid{40, 44, 52, 255};              // Grid lines
    Color mAxis{70, 75, 85, 255};              // Axis lines
    Color mBorder{40, 44, 52, 255};            // Cell / node borders
    Color mPositive{60, 190, 120, 255};        // Positive values (green)
    Color mNegative{220, 90, 90, 255};         // Negative values (red)
    Color mWarning{255, 200, 80, 255};         // Warning / mid-range (yellow)

    // Series accent palette — up to 8 data series
    std::array<Color, 8> mAccents{{
        {80, 180, 255, 255},   // cyan
        {255, 120, 80, 255},   // orange
        {120, 220, 120, 255},  // green
        {255, 200, 80, 255},   // gold
        {200, 120, 255, 255},  // purple
        {255, 140, 180, 255},  // pink
        {120, 240, 220, 255},  // teal
        {180, 180, 200, 255}   // silver
    }};

    Color accent(uint32_t aIndex) const {
        return mAccents[aIndex % mAccents.size()];
    }
};

namespace RLThemes {

inline RLChartTheme dark() {
    RLChartTheme lTheme;
    lTheme.mBackground    = {20, 22, 28, 255};
    lTheme.mForeground    = {220, 220, 230, 255};
    lTheme.mForegroundDim = {150, 155, 165, 255};
    lTheme.mGrid          = {40, 44, 52, 255};
    lTheme.mAxis          = {70, 75, 85, 255};
    lTheme.mBorder        = {40, 44, 52, 255};
    lTheme.mPositive      = {60, 190, 120, 255};
    lTheme.mNegative      = {220, 90, 90, 255};
    lTheme.mWarning       = {255, 200, 80, 255};
    lTheme.mAccents = {{
        {80, 180, 255, 255},
        {255, 120, 80, 255},
        {120, 220, 120, 255},
        {255, 200, 80, 255},
        {200, 120, 255, 255},
        {255, 140, 180, 255},
        {120, 240, 220, 255},
        {180, 180, 200, 255}
    }};
    return lTheme;
}

inline RLChartTheme light() {
    RLChartTheme lTheme;
    lTheme.mBackground    = {245, 245, 248, 255};
    lTheme.mForeground    = {30, 30, 40, 255};
    lTheme.mForegroundDim = {100, 100, 115, 255};
    lTheme.mGrid          = {210, 215, 220, 255};
    lTheme.mAxis          = {160, 165, 175, 255};
    lTheme.mBorder        = {190, 195, 205, 255};
    lTheme.mPositive      = {40, 160, 90, 255};
    lTheme.mNegative      = {200, 60, 60, 255};
    lTheme.mWarning       = {220, 170, 40, 255};
    lTheme.mAccents = {{
        {50, 130, 220, 255},
        {220, 90, 50, 255},
        {40, 170, 80, 255},
        {200, 160, 40, 255},
        {150, 80, 210, 255},
        {210, 90, 140, 255},
        {40, 180, 170, 255},
        {120, 120, 140, 255}
    }};
    return lTheme;
}

inline RLChartTheme midnight() {
    RLChartTheme lTheme;
    lTheme.mBackground    = {10, 12, 22, 255};
    lTheme.mForeground    = {190, 200, 220, 255};
    lTheme.mForegroundDim = {100, 110, 140, 255};
    lTheme.mGrid          = {25, 30, 50, 255};
    lTheme.mAxis          = {50, 55, 80, 255};
    lTheme.mBorder        = {30, 35, 55, 255};
    lTheme.mPositive      = {50, 210, 140, 255};
    lTheme.mNegative      = {255, 70, 90, 255};
    lTheme.mWarning       = {255, 190, 60, 255};
    lTheme.mAccents = {{
        {100, 160, 255, 255},
        {255, 150, 100, 255},
        {100, 240, 160, 255},
        {255, 210, 100, 255},
        {180, 130, 255, 255},
        {255, 130, 170, 255},
        {100, 230, 220, 255},
        {160, 170, 210, 255}
    }};
    return lTheme;
}

inline RLChartTheme neon() {
    RLChartTheme lTheme;
    lTheme.mBackground    = {5, 5, 15, 255};
    lTheme.mForeground    = {230, 240, 255, 255};
    lTheme.mForegroundDim = {120, 140, 180, 255};
    lTheme.mGrid          = {20, 25, 45, 255};
    lTheme.mAxis          = {40, 50, 80, 255};
    lTheme.mBorder        = {30, 40, 70, 255};
    lTheme.mPositive      = {0, 255, 140, 255};
    lTheme.mNegative      = {255, 40, 80, 255};
    lTheme.mWarning       = {255, 220, 0, 255};
    lTheme.mAccents = {{
        {0, 200, 255, 255},
        {255, 80, 120, 255},
        {0, 255, 160, 255},
        {255, 200, 0, 255},
        {180, 80, 255, 255},
        {255, 100, 200, 255},
        {0, 255, 240, 255},
        {200, 200, 255, 255}
    }};
    return lTheme;
}

} // namespace RLThemes

// Reusable theme selector bar with clickable buttons.
// Draws 4 small labeled buttons and returns the theme when one is clicked.
struct RLThemeSelector {
    int32_t mSelected{0};  // 0=Dark, 1=Light, 2=Midnight, 3=Neon

    // Draw buttons at the given top-left position. Returns true if selection changed.
    bool draw(float aX, float aY, Font aFont, float aFontSize = 14.0f) {
        const char* lNames[] = {"Dark", "Light", "Midnight", "Neon"};
        const Color lBgColors[] = {
            {30, 32, 38, 255}, {235, 235, 240, 255}, {15, 18, 30, 255}, {10, 10, 20, 255}
        };
        const Color lFgColors[] = {
            {200, 200, 210, 255}, {30, 30, 40, 255}, {160, 170, 200, 255}, {0, 200, 255, 255}
        };
        const Color lAccentColors[] = {
            {80, 180, 255, 255}, {50, 130, 220, 255}, {100, 160, 255, 255}, {0, 200, 255, 255}
        };

        auto lPrevSelected = mSelected;
        auto lMouse = GetMousePosition();
        float lCurX = aX;
        const float BUTTON_H = 24.0f;
        const float PAD_X = 10.0f;
        const float SPACING = 6.0f;
        const float INDICATOR_W = 8.0f;

        // Draw "Theme:" label
        auto lLabelSize = MeasureTextEx(aFont, "Theme:", aFontSize, 1.0f);
        DrawTextEx(aFont, "Theme:", Vector2{lCurX, aY + (BUTTON_H - lLabelSize.y) * 0.5f},
                   aFontSize, 1.0f, Color{160, 160, 170, 255});
        lCurX += lLabelSize.x + SPACING * 2.0f;

        for (int32_t i = 0; i < 4; ++i) {
            auto lTextSize = MeasureTextEx(aFont, lNames[i], aFontSize, 1.0f);
            float lBtnW = lTextSize.x + PAD_X * 2.0f + INDICATOR_W + 4.0f;
            auto lRect = Rectangle{lCurX, aY, lBtnW, BUTTON_H};
            auto lHover = CheckCollisionPointRec(lMouse, lRect);
            auto lSelected = (i == mSelected);

            // Button background
            auto lBg = lSelected ? lAccentColors[i] : (lHover ? Color{60, 65, 75, 255} : Color{35, 38, 46, 255});
            DrawRectangleRounded(lRect, 0.4f, 4, lBg);

            // Color indicator dot
            DrawCircle((int)(lCurX + PAD_X * 0.5f + INDICATOR_W * 0.5f),
                       (int)(aY + BUTTON_H * 0.5f), INDICATOR_W * 0.4f, lBgColors[i]);

            // Text
            auto lFg = lSelected ? Color{255, 255, 255, 255} : (lHover ? Color{220, 220, 230, 255} : Color{150, 155, 165, 255});
            DrawTextEx(aFont, lNames[i],
                       Vector2{lCurX + PAD_X + INDICATOR_W + 2.0f, aY + (BUTTON_H - lTextSize.y) * 0.5f},
                       aFontSize, 1.0f, lFg);

            if (lHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                mSelected = i;
            }

            lCurX += lBtnW + SPACING;
        }

        return mSelected != lPrevSelected;
    }

    RLChartTheme getTheme() const {
        switch (mSelected) {
            case 1: return RLThemes::light();
            case 2: return RLThemes::midnight();
            case 3: return RLThemes::neon();
            default: return RLThemes::dark();
        }
    }

    // Background color that complements the current theme
    Color getWindowBackground() const {
        switch (mSelected) {
            case 1: return Color{230, 232, 238, 255};
            case 2: return Color{6, 8, 16, 255};
            case 3: return Color{2, 2, 10, 255};
            default: return Color{15, 17, 20, 255};
        }
    }
};
