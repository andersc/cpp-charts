// RLCommon.h
#pragma once
#include "raylib.h"
#include <cmath>

// Common utilities for raylib charts
// All functions are in the RLCharts namespace to avoid conflicts

namespace RLCharts {

// Template for clamping values to [0, 1] range
template<typename T>
inline T clamp01(T aValue) {
    return aValue < static_cast<T>(0) ? static_cast<T>(0) :
           (aValue > static_cast<T>(1) ? static_cast<T>(1) : aValue);
}

// Template for clamping to arbitrary range
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

// Template for linear interpolation
template<typename T>
inline T lerp(T a, T b, float t) {
    return static_cast<T>(a + (b - a) * t);
}

// Specialized lerp for floats (explicit to avoid casting overhead)
inline float lerpF(float a, float b, float t) {
    return a + (b - a) * t;
}

// Color interpolation with clamping
inline Color lerpColor(const Color& a, const Color& b, float t) {
    t = clamp01(t);
    Color lResult;
    lResult.r = (unsigned char)(a.r + (int)((b.r - a.r) * t));
    lResult.g = (unsigned char)(a.g + (int)((b.g - a.g) * t));
    lResult.b = (unsigned char)(a.b + (int)((b.b - a.b) * t));
    lResult.a = (unsigned char)(a.a + (int)((b.a - a.a) * t));
    return lResult;
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

// Degree to radian conversion
inline float degToRad(float aDeg) {
    return aDeg * PI / 180.0f;
}

// Radian to degree conversion
inline float radToDeg(float aRad) {
    return aRad * 180.0f / PI;
}

// Calculate luminance of a color (for auto text color selection)
inline float colorLuma(const Color& rColor) {
    return 0.2126f * rColor.r + 0.7152f * rColor.g + 0.0722f * rColor.b;
}

// Smooth approach function for exponential smoothing
inline float approach(float a, float b, float aSpeedDt) {
    float lDiff = b - a;
    return a + lDiff * (lDiff * lDiff < 1e-8f ? 1.0f : clamp01(aSpeedDt));
}

// Multiply alpha channel by a factor
inline unsigned char mulAlpha(unsigned char a, float f) {
    float lValue = (float)a * f;
    if (lValue < 0.0f) lValue = 0.0f;
    if (lValue > 255.0f) lValue = 255.0f;
    return (unsigned char)(lValue + 0.5f);
}

// Vector2 linear interpolation
inline Vector2 lerpVector2(const Vector2& a, const Vector2& b, float t) {
    return Vector2{ lerpF(a.x, b.x, t), lerpF(a.y, b.y, t) };
}

// Vector2 distance
inline float distance(const Vector2& a, const Vector2& b) {
    float lDx = a.x - b.x;
    float lDy = a.y - b.y;
    return sqrtf(lDx * lDx + lDy * lDy);
}

// Catmull-Rom spline interpolation
inline Vector2 catmullRom(const Vector2& aP0, const Vector2& aP1,
                          const Vector2& aP2, const Vector2& aP3, float aT) {
    float lT2 = aT * aT;
    float lT3 = lT2 * aT;
    float lX = 0.5f * ((2.0f * aP1.x) +
                       (-aP0.x + aP2.x) * aT +
                       (2 * aP0.x - 5 * aP1.x + 4 * aP2.x - aP3.x) * lT2 +
                       (-aP0.x + 3 * aP1.x - 3 * aP2.x + aP3.x) * lT3);
    float lY = 0.5f * ((2.0f * aP1.y) +
                       (-aP0.y + aP2.y) * aT +
                       (2 * aP0.y - 5 * aP1.y + 4 * aP2.y - aP3.y) * lT2 +
                       (-aP0.y + 3 * aP1.y - 3 * aP2.y + aP3.y) * lT3);
    return { lX, lY };
}

} // namespace RLCharts

