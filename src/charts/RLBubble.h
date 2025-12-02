#pragma once
#include "raylib.h"
#include <vector>

// Simple and fast Bubble Chart for raylib.
// Two modes:
//  - Scatter: bubbles have x,y in [0,1], size (arbitrary), and color. They animate between datasets.
//  - Gravity: the largest bubble is centered; others are attracted by mass ~ size with soft collisions.

struct RLBubblePoint {
    float mX{0.0f};    // normalized [0,1] (scatter mode)
    float mY{0.0f};    // normalized [0,1]
    float mSize{1.0f}; // arbitrary value; mapped to radius by sizeScale
    Color mColor{ 80, 180, 255, 255 };
};

enum class RLBubbleMode {
    Scatter,
    Gravity
};

struct RLBubbleStyle {
    Color mBackground{20,22,28,255};
    Color mAxesColor{70,75,85,255};
    Color mGridColor{40,44,52,255};
    int mGridLines = 4;         // per axis
    float mSizeScale = 24.0f;   // pixel radius per sqrt(size)
    float mMinRadius = 3.0f;    // minimum visual radius in pixels
    float mOutline = 2.0f;      // outline thickness
    Color mOutlineColor{0,0,0,80};
    bool mShowAxes = true;
    bool mSmooth = true;
};

class RLBubble {
public:
    explicit RLBubble(Rectangle bounds, RLBubbleMode mode = RLBubbleMode::Scatter, const RLBubbleStyle &style = {});

    void SetBounds(Rectangle bounds);
    void SetStyle(const RLBubbleStyle &style);
    void SetMode(RLBubbleMode mode);

    // Set current data immediately (no animation)
    void SetData(const std::vector<RLBubblePoint> &data);
    // Set target data to animate towards (by index). If counts differ, will resize smoothly.
    void SetTargetData(const std::vector<RLBubblePoint> &data);

    // Update simulation/animation. dt in seconds.
    void Update(float dt);
    // Draw chart inside bounds.
    void Draw() const;

    // Helpers
    [[nodiscard]] Rectangle GetBounds() const { return mBounds; }
    [[nodiscard]] RLBubbleMode GetMode() const { return mMode; }

private:
    struct BubbleDyn {
        // current state
        Vector2 mPos{0,0};
        Vector2 mPrevPos{0,0}; // Required for stable physics (Verlet)
        float mRadius{5.0f};
        Color mColor{ 80,180,255,255 };

        // target state (scatter)
        Vector2 mPosTarget{0,0};
        float mRadiusTarget{5.0f};
        Color mColorTarget{ 80,180,255,255 };

        // velocity (gravity)
        Vector2 mVel{0,0};
        float mMass{1.0f};
    };

    Rectangle mBounds{};
    RLBubbleMode mMode{RLBubbleMode::Scatter};
    RLBubbleStyle mStyle{};

    std::vector<BubbleDyn> mBubbles;
    int mLargestIndex{-1};

    // animation parameters
    float mLerpSpeed = 6.0f;         // scatter smoothing
    float mGravity = 200.0f;         // gravity constant
    float mDamping = 0.85f;          // velocity damping (0.85 = high friction to stop spinning)

    // internal helpers
    void buildTargetsForAnimation(const std::vector<RLBubblePoint> &data);
    void setImmediateDataInternal(const std::vector<RLBubblePoint> &data);
    [[nodiscard]] Rectangle chartRect() const;
    [[nodiscard]] float sizeToRadius(float size) const;
    [[nodiscard]] Vector2 lerp(const Vector2 &a, const Vector2 &b, float t) const;
    [[nodiscard]] Color lerp(const Color &a, const Color &b, float t) const;
};