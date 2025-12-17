// RLTreeMap.h
#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <functional>

// D3-style TreeMap visualization for raylib
// Usage: construct with bounds, set hierarchy via setData(), call update(dt) and draw() each frame.

// Node structure for building hierarchical data
struct RLTreeNode {
    std::string mLabel;                         // Node label
    float mValue{0.0f};                         // Leaf value (determines area)
    Color mColor{80, 180, 255, 255};            // Node color (optional, uses fallback if alpha == 0)
    bool mUseColor{false};                      // If false, use color mapping rules
    std::vector<RLTreeNode> mChildren;          // Child nodes (empty for leaves)
};

// Layout algorithm options
enum class RLTreeMapLayout {
    SQUARIFIED,     // Squarified treemap (best aspect ratios)
    SLICE,          // Simple slice layout
    DICE,           // Simple dice layout
    SLICE_DICE      // Alternating slice/dice by depth
};

// Style configuration for RLTreeMap
struct RLTreeMapStyle {
    // Background
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};

    // Node appearance
    float mPaddingOuter{4.0f};          // Padding around the entire treemap
    float mPaddingInner{2.0f};          // Padding between sibling nodes
    float mPaddingTop{18.0f};           // Extra top padding for parent nodes (label space)
    float mBorderThickness{1.0f};       // Border thickness around nodes
    Color mBorderColor{40, 44, 52, 255};
    float mCornerRadius{3.0f};          // Rounded corner radius

    // Internal nodes
    bool mShowInternalNodes{true};      // Show rectangles for internal nodes
    Color mInternalNodeColor{30, 32, 40, 200}; // Background color for internal nodes
    bool mShowInternalLabels{true};     // Show labels on internal nodes

    // Leaf nodes
    bool mShowLeafLabels{true};         // Show labels on leaf nodes
    float mMinNodeSize{8.0f};           // Minimum node size to display
    bool mLabelFitCheck{true};          // Only show label if it fits

    // Labels
    int mLabelFontSize{14};
    Color mLabelColor{230, 230, 240, 255};
    bool mAutoLabelColor{true};         // Choose white/black based on node color
    Font mLabelFont{};                  // Optional custom font; if .baseSize==0 use default

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{6.0f};          // Approach speed for size transitions
    float mColorSpeed{4.0f};            // Color blend speed

    // Color mapping (used when node mUseColor == false)
    bool mUseDepthColors{true};         // Color by depth level
    std::vector<Color> mDepthPalette;   // Colors for each depth level
};

// Computed rectangle for a node (internal use and optional user access)
struct RLTreeRect {
    Rectangle mRect{0, 0, 0, 0};        // Current animated rectangle
    Rectangle mTargetRect{0, 0, 0, 0};  // Target rectangle
    Color mColor{80, 180, 255, 255};    // Current color
    Color mTargetColor{80, 180, 255, 255}; // Target color
    float mAlpha{1.0f};                 // Current visibility alpha
    float mTargetAlpha{1.0f};           // Target alpha
    std::string mLabel;                 // Node label
    int mDepth{0};                      // Depth in hierarchy
    bool mIsLeaf{true};                 // Whether this is a leaf node
    float mValue{0.0f};                 // Node value
    size_t mParentIndex{(size_t)-1};    // Index of parent in a flat list
};

class RLTreeMap {
public:
    explicit RLTreeMap(Rectangle aBounds, RLTreeMapStyle  rStyle = {});

    // Configuration
    void setBounds(Rectangle aBounds);
    void setStyle(const RLTreeMapStyle& rStyle);
    void setLayout(RLTreeMapLayout aLayout);

    // Set hierarchy data (triggers layout recomputation)
    void setData(const RLTreeNode& rRoot);

    // Dynamic update: set new data with animation
    void setTargetData(const RLTreeNode& rRoot);

    // Update a single node value by path (e.g., {"Parent", "Child"})
    void updateValue(const std::vector<std::string>& rPath, float aNewValue);

    // Force layout recomputation
    void recomputeLayout();

    // Per-frame update (call each frame with delta time)
    void update(float aDt);

    // Draw the treemap
    void draw() const;

    // Accessors
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] const std::vector<RLTreeRect>& getComputedRects() const { return mRects; }
    [[nodiscard]] size_t getNodeCount() const { return mRects.size(); }

    // Optional: get node at point (returns index or -1)
    [[nodiscard]] int getNodeAtPoint(Vector2 aPoint) const;

    // Optional: highlight a node
    void setHighlightedNode(int aIndex);
    [[nodiscard]] int getHighlightedNode() const { return mHighlightedIndex; }

private:
    Rectangle mBounds{};
    RLTreeMapStyle mStyle{};
    RLTreeMapLayout mLayout{RLTreeMapLayout::SQUARIFIED};

    // Hierarchy storage
    RLTreeNode mRoot;
    bool mDataDirty{false};

    // Flattened computed rectangles
    std::vector<RLTreeRect> mRects;
    std::vector<RLTreeRect> mTargetRects;

    // Highlight
    int mHighlightedIndex{-1};

    // Layout computation
    void computeLayout();
    void flattenHierarchy(const RLTreeNode& rNode, int aDepth, size_t aParentIdx);
    void layoutNode(size_t aNodeIdx, Rectangle aAvailable, int aDepth);
    void layoutSquarified(std::vector<size_t>& rChildIndices, Rectangle aAvailable);
    void layoutSlice(std::vector<size_t>& rChildIndices, Rectangle aAvailable, bool aVertical);

    // Helper: sum values of children
    [[nodiscard]] float sumChildValues(const std::vector<size_t>& rIndices) const;

    // Helper: compute total value of a subtree
    [[nodiscard]] float computeSubtreeValue(const RLTreeNode& rNode) const;

    // Color computation
    [[nodiscard]] Color computeNodeColor(const RLTreeNode& rNode, int aDepth) const;

    // Animation helpers
    [[nodiscard]] static float approach(float a, float b, float aSpeedDt);
    [[nodiscard]] static Color lerpColor(const Color& a, const Color& b, float t);
    [[nodiscard]] static Rectangle lerpRect(const Rectangle& a, const Rectangle& b, float t);

    // Default palette
    void ensureDefaultPalette();
};

