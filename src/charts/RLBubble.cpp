#include "RLBubble.h"
#include "RLCommon.h"
#include <cmath>
#include <algorithm>
#include <vector>

// --- Internal Helper: Spatial Grid ---
// This optimizes collision detection from O(N^2) to roughly O(N)
// Moved to anonymous namespace to avoid external linkage conflicts
namespace {
struct SpatialGrid {
    int cols{0}, rows{0};
    float cellSize{1.0f};
    float startX{0.0f}, startY{0.0f};
    // Stores indices of bubbles in each cell
    std::vector<std::vector<int>> cells;

    void Setup(Rectangle bounds, float maxDiameter) {
        cellSize = maxDiameter;
        if (cellSize < 1.0f) cellSize = 10.0f; 
        
        startX = bounds.x;
        startY = bounds.y;
        cols = (int)(bounds.width / cellSize) + 2;
        rows = (int)(bounds.height / cellSize) + 2;
        
        size_t total = (size_t)(cols * rows);
        if (cells.size() < total) cells.resize(total);
        
        // Fast clear
        for(auto& c : cells) c.clear();
    }

    void Insert(int bubbleIndex, Vector2 pos) {
        int cx = (int)((pos.x - startX) / cellSize);
        int cy = (int)((pos.y - startY) / cellSize);
        // Bounds check
        if (cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
            cells[cy * cols + cx].push_back(bubbleIndex);
        }
    }

    const std::vector<int>* GetCell(int cx, int cy) const {
        if (cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
            return &cells[cy * cols + cx];
        }
        return nullptr;
    }
};

// Local instance within anonymous namespace
SpatialGrid gGrid;

} // anonymous namespace

RLBubble::RLBubble(Rectangle bounds, RLBubbleMode mode, const RLBubbleStyle &style)
    : mBounds(bounds), mMode(mode), mStyle(style)
{
}

Rectangle RLBubble::chartRect() const{
    float pad = mStyle.mShowAxes ? 32.0f : 8.0f;
    return Rectangle{ mBounds.x+pad, mBounds.y+pad, std::max(0.0f,mBounds.width-2*pad), std::max(0.0f,mBounds.height-2*pad) };
}

void RLBubble::SetBounds(Rectangle bounds){ mBounds = bounds; }
void RLBubble::SetStyle(const RLBubbleStyle &style){ mStyle = style; }
void RLBubble::SetMode(RLBubbleMode mode){ mMode = mode; }

float RLBubble::sizeToRadius(float size) const{
    float r = std::sqrtf(std::max(0.0f, size)) * mStyle.mSizeScale;
    return std::max(mStyle.mMinRadius, r);
}

Vector2 RLBubble::lerp(const Vector2 &a, const Vector2 &b, float t) const{
    return Vector2{ RLCharts::lerpF(a.x,b.x,t), RLCharts::lerpF(a.y,b.y,t) };
}

Color RLBubble::lerp(const Color &a, const Color &b, float t) const{
    return Color{
        (unsigned char)(a.r + (b.r - a.r)*t),
        (unsigned char)(a.g + (b.g - a.g)*t),
        (unsigned char)(a.b + (b.b - a.b)*t),
        (unsigned char)(a.a + (b.a - a.a)*t)
    };
}

void RLBubble::setImmediateDataInternal(const std::vector<RLBubblePoint> &data){
    Rectangle cr = chartRect();
    size_t n = data.size();
    mBubbles.clear();
    mBubbles.reserve(n);
    mLargestIndex = -1;
    float largestR = -1.0f;
    for (size_t i=0;i<n;i++){
        const auto &p = data[i];
        BubbleDyn b;
        b.mPos = { cr.x + RLCharts::clamp01(p.mX)*cr.width, cr.y + (1.0f- RLCharts::clamp01(p.mY))*cr.height };
        b.mPrevPos = b.mPos;
        b.mRadius = sizeToRadius(p.mSize);
        b.mColor = p.mColor;
        
        b.mPosTarget = b.mPos;
        b.mRadiusTarget = b.mRadius;
        b.mColorTarget = b.mColor;
        
        b.mVel = {0,0};
        b.mMass = std::max(1.0f, b.mRadius * b.mRadius); // Mass proportional to area
        
        mBubbles.push_back(b);
        if (b.mRadius > largestR){ largestR = b.mRadius; mLargestIndex = (int)i; }
    }
}

void RLBubble::buildTargetsForAnimation(const std::vector<RLBubblePoint> &data){
    Rectangle cr = chartRect();
    const size_t oldN = mBubbles.size();
    const size_t newN = data.size();

    mLargestIndex = -1; float largestR = -1.0f;
    const size_t minN = std::min(oldN, newN);

    // Update existing bubbles
    for (size_t i=0;i<minN;i++){
        const auto &p = data[i];
        auto &b = mBubbles[i];
        
        // If invisible/uninitialized, spawn at target location with 0 radius
        if (b.mRadius <= 0.1f && b.mColor.a == 0){
            b.mPos = { cr.x + RLCharts::clamp01(p.mX)*cr.width, cr.y + (1.0f- RLCharts::clamp01(p.mY))*cr.height };
            b.mPrevPos = b.mPos;
            b.mColor = { p.mColor.r, p.mColor.g, p.mColor.b, 0 };
            b.mRadius = 0.0f;
        }

        b.mPosTarget = { cr.x + RLCharts::clamp01(p.mX)*cr.width, cr.y + (1.0f- RLCharts::clamp01(p.mY))*cr.height };
        b.mRadiusTarget = sizeToRadius(p.mSize);
        b.mColorTarget = p.mColor;
        b.mMass = std::max(1.0f, b.mRadiusTarget * b.mRadiusTarget);
        
        if (b.mRadiusTarget > largestR){ largestR = b.mRadiusTarget; mLargestIndex = (int)i; }
    }

    // Fade out excess bubbles
    for (size_t i=minN; i<oldN; ++i){
        auto &b = mBubbles[i];
        b.mPosTarget = b.mPos; 
        b.mRadiusTarget = 0.0f;
        b.mColorTarget = b.mColor; 
        b.mColorTarget.a = 0;
    }

    // Spawn new bubbles
    if (newN > oldN){
        mBubbles.reserve(newN);
        for (size_t i=oldN; i<newN; ++i){
            const auto &p = data[i];
            BubbleDyn b{};
            b.mPos = { cr.x + RLCharts::clamp01(p.mX)*cr.width, cr.y + (1.0f- RLCharts::clamp01(p.mY))*cr.height };
            b.mPrevPos = b.mPos;
            b.mPosTarget = b.mPos;
            b.mRadius = 0.0f;
            b.mRadiusTarget = sizeToRadius(p.mSize);
            b.mColor = { p.mColor.r, p.mColor.g, p.mColor.b, 0 };
            b.mColorTarget = p.mColor;
            b.mVel = {0,0};
            b.mMass = std::max(1.0f, b.mRadiusTarget * b.mRadiusTarget);
            mBubbles.push_back(b);
            
            if (b.mRadiusTarget > largestR){ largestR = b.mRadiusTarget; mLargestIndex = (int)i; }
        }
    }
}

void RLBubble::SetData(const std::vector<RLBubblePoint> &data){
    setImmediateDataInternal(data);
}

void RLBubble::SetTargetData(const std::vector<RLBubblePoint> &data){
    buildTargetsForAnimation(data);
}

void RLBubble::Update(float dt){
    if (mBubbles.empty()) return;

    // Cap dt to prevent physics explosions on lag
    dt = std::max(0.001f, std::min(dt, 0.05f)); 

    // 1. Interpolate Radius and Color (Visuals)
    float lerpT = 1.0f - std::exp(-mLerpSpeed * dt);
    float maxDiameter = 0.0f;

    for (auto &b : mBubbles){
        b.mRadius = RLCharts::lerpF(b.mRadius, b.mRadiusTarget, lerpT);
        b.mColor  = lerp(b.mColor,  b.mColorTarget,  lerpT);
        if (b.mRadius * 2.0f > maxDiameter) maxDiameter = b.mRadius * 2.0f;
    }

    if (mMode == RLBubbleMode::Scatter){
        // --- SCATTER MODE: Simple Lerp ---
        for (auto &b : mBubbles){
            b.mPos = lerp(b.mPos, b.mPosTarget, lerpT);
            b.mPrevPos = b.mPos; // Keep physics state sync
        }
    } else {
        // --- GRAVITY MODE: Stabilized PBD ---
        Rectangle cr = chartRect();
        Vector2 center{ cr.x + cr.width*0.5f, cr.y + cr.height*0.5f };

        const float friction = 0.88f;    // High friction stops spinning
        const float gravityStr = 15.0f;  // Pull to center
        const int iterations = 8;        // Solver iterations for stability

        // A. Forces & Integration
        for (auto &b : mBubbles){
            // Center Gravity
            float dx = center.x - b.mPos.x;
            float dy = center.y - b.mPos.y;
            
            // Add force to velocity
            b.mVel.x += dx * gravityStr * dt;
            b.mVel.y += dy * gravityStr * dt;

            // Apply Friction (Damping) immediately
            b.mVel.x *= friction;
            b.mVel.y *= friction;

            // Store previous pos for Verlet
            b.mPrevPos = b.mPos;

            // Apply Velocity
            b.mPos.x += b.mVel.x * dt;
            b.mPos.y += b.mVel.y * dt;

            // Wall Constraints (Hard clamp)
            if (b.mPos.x < cr.x + b.mRadius) b.mPos.x = cr.x + b.mRadius;
            if (b.mPos.x > cr.x + cr.width - b.mRadius) b.mPos.x = cr.x + cr.width - b.mRadius;
            if (b.mPos.y < cr.y + b.mRadius) b.mPos.y = cr.y + b.mRadius;
            if (b.mPos.y > cr.y + cr.height - b.mRadius) b.mPos.y = cr.y + cr.height - b.mRadius;
        }

        // B. Collision Resolution (Grid Optimized)
        gGrid.Setup(cr, maxDiameter);
        for(int i = 0; i < (int)mBubbles.size(); ++i) {
            gGrid.Insert(i, mBubbles[i].mPos);
        }

        for (int k = 0; k < iterations; ++k){
            for (int i = 0; i < (int)mBubbles.size(); ++i) {
                auto& a = mBubbles[i];
                if (a.mRadius <= 0.0f) continue;

                int cx = (int)((a.mPos.x - gGrid.startX) / gGrid.cellSize);
                int cy = (int)((a.mPos.y - gGrid.startY) / gGrid.cellSize);

                // Check 3x3 neighbors
                for (int ny = cy - 1; ny <= cy + 1; ++ny) {
                    for (int nx = cx - 1; nx <= cx + 1; ++nx) {
                        const std::vector<int>* cell = gGrid.GetCell(nx, ny);
                        if (!cell) continue;

                        for (int j : *cell) {
                            if (i == j) continue;
                            auto& b = mBubbles[j];

                            float dx = a.mPos.x - b.mPos.x;
                            float dy = a.mPos.y - b.mPos.y;
                            float rSum = a.mRadius + b.mRadius;

                            // Bounding box check
                            if (std::abs(dx) >= rSum || std::abs(dy) >= rSum) continue;

                            float distSq = dx*dx + dy*dy;
                            if (distSq < rSum*rSum && distSq > 0.0001f) {
                                float dist = std::sqrt(distSq);
                                float pen = (rSum - dist) * 0.5f; // Split overlap

                                float nx_val = dx / dist;
                                float ny_val = dy / dist;

                                // Mass-weighted push
                                // Heavier (larger) bubbles move less
                                float totalM = a.mMass + b.mMass;
                                float ratioA = b.mMass / totalM;
                                float ratioB = a.mMass / totalM;

                                a.mPos.x += nx_val * pen * ratioA;
                                a.mPos.y += ny_val * pen * ratioA;
                                b.mPos.x -= nx_val * pen * ratioB;
                                b.mPos.y -= ny_val * pen * ratioB;
                            }
                        }
                    }
                }
            }
        }

        // C. Reconstruct Velocity
        for (auto &b : mBubbles){
            b.mVel.x = (b.mPos.x - b.mPrevPos.x) / dt;
            b.mVel.y = (b.mPos.y - b.mPrevPos.y) / dt;

            // Sleep threshold to stop micro-jitter
            float speedSq = b.mVel.x*b.mVel.x + b.mVel.y*b.mVel.y;
            if (speedSq < 1.0f) {
                b.mVel = {0,0};
            }
        }
    }

    // Cleanup dead bubbles
    auto it = std::remove_if(mBubbles.begin(), mBubbles.end(), [](const BubbleDyn &b){
        return (b.mRadiusTarget <= 0.001f) && (b.mRadius < 0.5f) && (b.mColor.a < 5);
    });
    if (it != mBubbles.end()) mBubbles.erase(it, mBubbles.end());
}

void RLBubble::Draw() const{
    // 1. Background
    if (mStyle.mBackground.a > 0) 
        DrawRectangleRounded(mBounds, 0.06f, 6, mStyle.mBackground);

    // 2. Axes/Grid
    if (mStyle.mShowAxes){
        int n = std::max(0, mStyle.mGridLines);
        for (int i=0; i<=n; i++){
            float t = (float)i / (float)(n);
            float x = mBounds.x + t * mBounds.width;
            float y = mBounds.y + t * mBounds.height;
            DrawLineV({x, mBounds.y}, {x, mBounds.y + mBounds.height}, mStyle.mGridColor);
            DrawLineV({mBounds.x, y}, {mBounds.x + mBounds.width, y}, mStyle.mGridColor);
        }
        DrawRectangleLinesEx(mBounds, 1.0f, mStyle.mAxesColor);
    }

    // 3. Draw Bubbles (Sorted by size)
    // We sort pointers to avoid copying large vectors, drawing large bubbles first.
    std::vector<const BubbleDyn*> drawOrder;
    drawOrder.reserve(mBubbles.size());
    for(const auto& b : mBubbles) drawOrder.push_back(&b);
    
    // Sort: Large radius -> Small radius (Painter's algorithm)
    std::sort(drawOrder.begin(), drawOrder.end(), [](const BubbleDyn* a, const BubbleDyn* b){
        return a->mRadius > b->mRadius; 
    });

    // Draw
    for (const auto *b : drawOrder){
        if (b->mRadius < 1.0f) continue; // Skip tiny ones
        
        if (mStyle.mOutline > 0.0f){ 
            DrawCircleV(b->mPos, b->mRadius + mStyle.mOutline, mStyle.mOutlineColor); 
        }
        DrawCircleV(b->mPos, b->mRadius, b->mColor);
    }
}