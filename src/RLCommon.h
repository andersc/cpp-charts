// RLCommon.h
#pragma once
#include "raylib.h"
#include "raymath.h"
#include <cmath>

// Common utilities for raylib charts
// Leverages raylib/raymath built-in functions where possible (raylib 6.0+)

namespace RLCharts {

// Template for clamping values to [0, 1] range
template<typename T>
inline T clamp01(T aValue) {
    return aValue < static_cast<T>(0) ? static_cast<T>(0) :
           (aValue > static_cast<T>(1) ? static_cast<T>(1) : aValue);
}

// Template for clamping to arbitrary range (supports int and float)
template<typename T>
inline T clamp(T aValue, T aMin, T aMax) {
    return aValue < aMin ? aMin : (aValue > aMax ? aMax : aValue);
}

// Template for integer clamping with bounds checking
template<typename T>
inline T clampIdx(T aValue, T aMaxLimit) {
    return aValue < static_cast<T>(0) ? static_cast<T>(0) :
           (aValue >= aMaxLimit ? aMaxLimit - static_cast<T>(1) : aValue);
}

// Template for min/max
template<typename T>
inline T minVal(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
inline T maxVal(T a, T b) {
    return a > b ? a : b;
}

// Calculate luminance of a color (for auto text color selection)
inline float colorLuma(const Color& rColor) {
    return 0.2126f * rColor.r + 0.7152f * rColor.g + 0.0722f * rColor.b;
}

// Smooth approach function for exponential smoothing
inline float approach(float a, float b, float aSpeedDt) {
    float lDiff = b - a;
    return a + lDiff * (lDiff * lDiff < 1e-8f ? 1.0f : Clamp(aSpeedDt, 0.0f, 1.0f));
}

// Multiply alpha channel by a factor
inline unsigned char mulAlpha(unsigned char a, float f) {
    float lValue = (float)a * f;
    return (unsigned char)Clamp(lValue + 0.5f, 0.0f, 255.0f);
}

} // namespace RLCharts

