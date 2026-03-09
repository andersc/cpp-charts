// RLEasing.h
#pragma once
#include <cmath>
#include <cstdint>

// Easing library for smooth chart animations.
// Provides standalone easing functions (t ∈ [0,1] → [0,1]) and an
// RLEaseMode enum for quick integration with the chart animation system.

#ifndef PI
#define PI 3.14159265358979323846f
#endif

enum class RLEaseMode : uint8_t {
    LINEAR,     // Constant speed
    SMOOTH,     // Exponential decay (default approach behavior)
    SNAPPY,     // Fast start, gradual settle (doubled decay rate)
    SPRINGY,    // Overshoots target, springs back
    ELASTIC     // Damped oscillation around target
};

namespace RLEasing {

// ---- Standalone easing functions (t ∈ [0,1] → [0,1]) ----

inline float linear(float aT) { return aT; }

inline float easeInQuad(float aT) { return aT * aT; }
inline float easeOutQuad(float aT) { return aT * (2.0f - aT); }
inline float easeInOutQuad(float aT) {
    return aT < 0.5f ? 2.0f * aT * aT : -1.0f + (4.0f - 2.0f * aT) * aT;
}

inline float easeInCubic(float aT) { return aT * aT * aT; }
inline float easeOutCubic(float aT) {
    auto lF = aT - 1.0f;
    return lF * lF * lF + 1.0f;
}
inline float easeInOutCubic(float aT) {
    return aT < 0.5f
        ? 4.0f * aT * aT * aT
        : 1.0f + (aT - 1.0f) * (2.0f * aT - 2.0f) * (2.0f * aT - 2.0f);
}

inline float easeInExpo(float aT) {
    return aT <= 0.0f ? 0.0f : powf(2.0f, 10.0f * (aT - 1.0f));
}
inline float easeOutExpo(float aT) {
    return aT >= 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * aT);
}

inline float easeOutElastic(float aT) {
    if (aT <= 0.0f) return 0.0f;
    if (aT >= 1.0f) return 1.0f;
    return powf(2.0f, -10.0f * aT) * sinf((aT - 0.075f) * (2.0f * PI) / 0.3f) + 1.0f;
}

inline float easeOutBounce(float aT) {
    if (aT < 1.0f / 2.75f) {
        return 7.5625f * aT * aT;
    } else if (aT < 2.0f / 2.75f) {
        auto lT = aT - 1.5f / 2.75f;
        return 7.5625f * lT * lT + 0.75f;
    } else if (aT < 2.5f / 2.75f) {
        auto lT = aT - 2.25f / 2.75f;
        return 7.5625f * lT * lT + 0.9375f;
    } else {
        auto lT = aT - 2.625f / 2.75f;
        return 7.5625f * lT * lT + 0.984375f;
    }
}

inline float easeOutBack(float aT) {
    auto lS = 1.70158f;
    auto lT = aT - 1.0f;
    return lT * lT * ((lS + 1.0f) * lT + lS) + 1.0f;
}

// ---- Per-frame approach with easing mode ----
// Drop-in replacement for RLCharts::approach() that supports different
// animation characters. aSpeedDt = animateSpeed * dt.

inline float approachEased(float aCurrent, float aTarget, float aSpeedDt, RLEaseMode aMode) {
    auto lDiff = aTarget - aCurrent;
    if (lDiff * lDiff < 1e-8f) return aTarget;

    float lAlpha;
    switch (aMode) {
        case RLEaseMode::LINEAR:
            lAlpha = aSpeedDt;
            break;
        case RLEaseMode::SMOOTH:
            lAlpha = 1.0f - expf(-aSpeedDt);
            break;
        case RLEaseMode::SNAPPY:
            lAlpha = 1.0f - expf(-aSpeedDt * 2.0f);
            break;
        case RLEaseMode::SPRINGY: {
            auto lBase = 1.0f - expf(-aSpeedDt);
            auto lOver = sinf(lBase * PI) * 0.15f;
            return aCurrent + lDiff * (lBase + lOver);
        }
        case RLEaseMode::ELASTIC: {
            auto lBase = 1.0f - expf(-aSpeedDt);
            auto lOsc = sinf(lBase * PI * 3.0f) * expf(-lBase * 4.0f) * 0.2f;
            return aCurrent + lDiff * (lBase + lOsc);
        }
    }
    // Clamp to [0,1] for non-overshooting modes
    if (lAlpha < 0.0f) lAlpha = 0.0f;
    if (lAlpha > 1.0f) lAlpha = 1.0f;
    return aCurrent + lDiff * lAlpha;
}

// Color approach with easing
inline Color approachColorEased(Color aCurrent, Color aTarget, float aSpeedDt, RLEaseMode aMode) {
    Color lResult;
    lResult.r = (uint8_t)approachEased((float)aCurrent.r, (float)aTarget.r, aSpeedDt, aMode);
    lResult.g = (uint8_t)approachEased((float)aCurrent.g, (float)aTarget.g, aSpeedDt, aMode);
    lResult.b = (uint8_t)approachEased((float)aCurrent.b, (float)aTarget.b, aSpeedDt, aMode);
    lResult.a = (uint8_t)approachEased((float)aCurrent.a, (float)aTarget.a, aSpeedDt, aMode);
    return lResult;
}

} // namespace RLEasing
