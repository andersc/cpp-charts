// RLSankey.h
#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <cstdint>

// Sankey diagram for raylib - flow visualization with weighted connections
// between nodes organized in columns/layers. Supports smooth animations,
// dynamic data updates, and configurable styling.
// Usage: construct with bounds, addNode()/addLink(), then per-frame update(dt) and draw().

// Color mode for links
enum class RLSankeyLinkColorMode {
    SOURCE,         // Use source node color
    TARGET,         // Use target node color
    GRADIENT,       // Gradient from source to target color
    CUSTOM          // Use per-link custom color
};

// Node definition
struct RLSankeyNode {
    std::string mLabel;
    Color mColor{80, 180, 255, 255};
    int mColumn{-1};            // Column index (-1 = auto-assign)
};

// Link definition
struct RLSankeyLink {
    size_t mSourceId{0};
    size_t mTargetId{0};
    float mValue{1.0f};         // Flow weight (affects thickness)
    Color mColor{255, 255, 255, 255}; // Custom color (used when mode is CUSTOM)
};

// Style configuration
struct RLSankeyStyle {
    // Background
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};

    // Nodes
    float mNodeWidth{20.0f};            // Width of node rectangles
    float mNodePadding{10.0f};          // Vertical spacing between nodes in same column
    float mNodeCornerRadius{4.0f};      // Rounded corners (0 = sharp)
    bool mShowNodeBorder{true};
    Color mNodeBorderColor{255, 255, 255, 40};
    float mNodeBorderThickness{1.0f};

    // Links
    float mColumnSpacing{150.0f};       // Horizontal spacing between columns
    float mMinLinkThickness{2.0f};      // Minimum link thickness for visibility
    float mLinkAlpha{0.6f};             // Alpha for link ribbons
    int mLinkSegments{24};              // Number of segments for Bezier curves
    RLSankeyLinkColorMode mLinkColorMode{RLSankeyLinkColorMode::GRADIENT};

    // Labels
    bool mShowLabels{true};
    Color mLabelColor{220, 225, 235, 255};
    Font mLabelFont{};
    int mLabelFontSize{14};
    float mLabelPadding{8.0f};          // Distance from node to label

    // Chart area
    float mPadding{40.0f};              // Padding from bounds to chart area

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{5.0f};          // Value interpolation speed
    float mFadeSpeed{4.0f};             // Fade in/out speed
};

class RLSankey {
public:
    explicit RLSankey(Rectangle aBounds, const RLSankeyStyle& rStyle = {});
    ~RLSankey() = default;

    // Configuration
    void setBounds(Rectangle aBounds);
    void setStyle(const RLSankeyStyle& rStyle);

    // Node management - returns node id
    size_t addNode(const std::string& rLabel, Color aColor = {80, 180, 255, 255}, int aColumn = -1);
    size_t addNode(const RLSankeyNode& rNode);
    void setNodeColor(size_t aNodeId, Color aColor);
    void setNodeColumn(size_t aNodeId, int aColumn);
    void removeNode(size_t aNodeId);

    // Link management - returns link id
    size_t addLink(size_t aSourceId, size_t aTargetId, float aValue, Color aColor = {255, 255, 255, 255});
    size_t addLink(const RLSankeyLink& rLink);
    void setLinkValue(size_t aLinkId, float aValue);
    void setLinkColor(size_t aLinkId, Color aColor);
    void removeLink(size_t aLinkId);

    // Batch data setting
    void setData(const std::vector<RLSankeyNode>& rNodes, const std::vector<RLSankeyLink>& rLinks);
    void clear();

    // Per-frame update and draw
    void update(float aDt);
    void draw() const;

    // Interaction
    int getHoveredNode(Vector2 aMousePos) const;   // Returns node id or -1
    int getHoveredLink(Vector2 aMousePos) const;   // Returns link id or -1
    void setHighlightedNode(int aNodeId);          // -1 to clear
    void setHighlightedLink(int aLinkId);          // -1 to clear

    // Getters
    [[nodiscard]] Rectangle getBounds() const { return mBounds; }
    [[nodiscard]] size_t getNodeCount() const;
    [[nodiscard]] size_t getLinkCount() const;
    [[nodiscard]] int getColumnCount() const { return mColumnCount; }

private:
    // Internal node with animation state
    struct NodeDyn {
        std::string mLabel;
        Color mColor{80, 180, 255, 255};
        Color mColorTarget{80, 180, 255, 255};
        int mColumn{0};
        float mVisibility{0.0f};
        float mVisibilityTarget{1.0f};
        bool mPendingRemoval{false};

        // Computed layout (animated)
        float mY{0.0f};                 // Top Y position
        float mYTarget{0.0f};
        float mHeight{0.0f};            // Node height
        float mHeightTarget{0.0f};

        // Link offset tracking (for stacking links)
        float mOutflowOffset{0.0f};     // Current offset for outgoing links
        float mInflowOffset{0.0f};      // Current offset for incoming links
    };

    // Internal link with animation state
    struct LinkDyn {
        size_t mSourceId{0};
        size_t mTargetId{0};
        float mValue{1.0f};
        float mValueTarget{1.0f};
        Color mColor{255, 255, 255, 255};
        Color mColorTarget{255, 255, 255, 255};
        float mVisibility{0.0f};
        float mVisibilityTarget{1.0f};
        bool mPendingRemoval{false};

        // Computed layout
        float mThickness{0.0f};
        float mThicknessTarget{0.0f};
        float mSourceY{0.0f};           // Y offset within source node
        float mSourceYTarget{0.0f};
        float mTargetY{0.0f};           // Y offset within target node
        float mTargetYTarget{0.0f};

        // Cached ribbon vertices
        mutable std::vector<Vector2> mCachedTopCurve;
        mutable std::vector<Vector2> mCachedBottomCurve;
        mutable bool mCacheDirty{true};
    };

    // Layout computation
    void computeLayout();
    void assignColumns();
    void computeNodePositions();
    void computeLinkPositions();
    float computeTotalFlow(size_t aNodeId, bool aOutgoing) const;

    // Drawing helpers
    void drawBackground() const;
    void drawLinks() const;
    void drawNodes() const;
    void drawLabels() const;
    void drawLink(const LinkDyn& rLink) const;
    void drawNode(const NodeDyn& rNode, size_t aNodeId) const;
    void computeLinkCurve(const LinkDyn& rLink) const;

    // Bezier curve helpers
    Vector2 cubicBezier(Vector2 aP0, Vector2 aP1, Vector2 aP2, Vector2 aP3, float aT) const;

    // Animation helpers
    static float clamp01(float aX) { return aX < 0.0f ? 0.0f : (aX > 1.0f ? 1.0f : aX); }
    static float approach(float aFrom, float aTo, float aSpeedDt);
    static Color lerpColor(Color aFrom, Color aTo, float aT);

    // Utility
    float getNodeX(int aColumn) const;

    // Member data
    Rectangle mBounds{};
    RLSankeyStyle mStyle{};
    std::vector<NodeDyn> mNodes;
    std::vector<LinkDyn> mLinks;

    // Layout state
    mutable bool mLayoutDirty{true};
    int mColumnCount{0};
    float mChartLeft{0.0f};
    float mChartTop{0.0f};
    float mChartWidth{0.0f};
    float mChartHeight{0.0f};
    float mValueToPixelScale{1.0f};     // Scale factor: value -> pixel height

    // Interaction state
    int mHighlightedNode{-1};
    int mHighlightedLink{-1};
};

