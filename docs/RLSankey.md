# RLSankey

A Sankey diagram visualization for raylib showing weighted flow connections between nodes organized in columns/layers. Supports smooth animations, dynamic data updates, and customizable styling.

## Features

- Multi-layer flow visualization with weighted connections
- Smooth Bezier ribbon curves between nodes
- Automatic or explicit column assignment for nodes
- Smooth animation on value changes
- Node/link add/remove with fade in/out effects
- Multiple link color modes (gradient, source, target, custom)
- Hover detection for nodes and links
- Full styling customization

## Constructor

```cpp
RLSankey(Rectangle aBounds, const RLSankeyStyle& rStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the chart's position and size
- `rStyle` - Optional style configuration

## Data Structures

### Node Definition

```cpp
struct RLSankeyNode {
    std::string mLabel;             // Node label text
    Color mColor{80, 180, 255, 255}; // Node color
    int mColumn{-1};                // Column index (-1 = auto-assign)
};
```

### Link Definition

```cpp
struct RLSankeyLink {
    size_t mSourceId{0};            // Source node id
    size_t mTargetId{0};            // Target node id
    float mValue{1.0f};             // Flow weight (affects thickness)
    Color mColor{255, 255, 255, 255}; // Custom color (used when mode is CUSTOM)
};
```

### Link Color Mode

```cpp
enum class RLSankeyLinkColorMode {
    SOURCE,     // Use source node color
    TARGET,     // Use target node color
    GRADIENT,   // Gradient from source to target color
    CUSTOM      // Use per-link custom color
};
```

| Mode | Description |
|------|-------------|
| `SOURCE` | Links use their source node's color |
| `TARGET` | Links use their target node's color |
| `GRADIENT` | Links fade from source to target color |
| `CUSTOM` | Links use their own custom color |

### Flow Mode

```cpp
enum class RLSankeyFlowMode {
    RAW_VALUE,   // Link thickness is proportional to value (may not fill node height)
    NORMALIZED   // Link bands are scaled to fill node height on both sides
};
```

| Mode | Description |
|------|-------------|
| `RAW_VALUE` | Link thickness directly represents value; bands may not fill node height if inflow ≠ outflow |
| `NORMALIZED` | Link bands are scaled proportionally to fill the full node height on both sides (proper Sankey behavior) |

## Style Configuration

```cpp
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
    RLSankeyFlowMode mFlowMode{RLSankeyFlowMode::NORMALIZED}; // Band width mode

    // Flow conservation
    bool mStrictFlowConservation{false}; // Validate inflow == outflow for intermediate nodes
    float mFlowTolerance{0.001f};        // Tolerance for flow conservation validation

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
```

## Methods

### Configuration

```cpp
void setBounds(Rectangle aBounds);
void setStyle(const RLSankeyStyle& rStyle);
```

### Node Management

```cpp
// Add a node with label, color, and optional column
size_t addNode(const std::string& rLabel, Color aColor = {80, 180, 255, 255}, int aColumn = -1);

// Add a node from struct
size_t addNode(const RLSankeyNode& rNode);

// Modify existing nodes
void setNodeColor(size_t aNodeId, Color aColor);
void setNodeColumn(size_t aNodeId, int aColumn);

// Remove a node (also removes connected links)
void removeNode(size_t aNodeId);
```

### Link Management

```cpp
// Add a link between nodes
size_t addLink(size_t aSourceId, size_t aTargetId, float aValue, Color aColor = {255, 255, 255, 255});

// Add a link from struct
size_t addLink(const RLSankeyLink& rLink);

// Modify existing links
void setLinkValue(size_t aLinkId, float aValue);
void setLinkColor(size_t aLinkId, Color aColor);

// Remove a link
void removeLink(size_t aLinkId);
```

### Batch Data

```cpp
// Set all nodes and links at once
// Returns true if flow conservation is valid (when strict mode enabled)
bool setData(const std::vector<RLSankeyNode>& rNodes, const std::vector<RLSankeyLink>& rLinks);

// Clear all data
void clear();
```

### Flow Conservation Validation

```cpp
// Check if flow is conserved at all intermediate nodes
// Returns true if all intermediate nodes have inflow == outflow (within tolerance)
// Logs warnings for any violations
bool validateFlowConservation() const;
```

### Update and Draw

```cpp
void update(float aDt);    // Call each frame for animations
void draw() const;         // Render the chart
```

### Interaction

```cpp
int getHoveredNode(Vector2 aMousePos) const;   // Returns node id or -1
int getHoveredLink(Vector2 aMousePos) const;   // Returns link id or -1
void setHighlightedNode(int aNodeId);          // -1 to clear
void setHighlightedLink(int aLinkId);          // -1 to clear
```

### Getters

```cpp
Rectangle getBounds() const;
size_t getNodeCount() const;
size_t getLinkCount() const;
int getColumnCount() const;
bool hasPendingRemovals() const;  // Returns true if any nodes/links are still fading out
```

## Column Assignment

Nodes can be assigned to columns in two ways:

1. **Explicit**: Set `mColumn` to a non-negative value (0, 1, 2, ...)
2. **Automatic**: Set `mColumn` to -1 (default)

When automatic assignment is used, the layout algorithm assigns columns based on link topology:
- Nodes with no incoming links are placed in column 0
- Other nodes are placed in `max(source columns) + 1`

## Example Usage

### Basic Sankey

```cpp
#include "RLSankey.h"

int main() {
    InitWindow(800, 600, "Sankey Demo");
    SetTargetFPS(60);

    Rectangle lBounds = {50, 50, 700, 500};
    RLSankey lSankey(lBounds);

    // Add nodes (3 columns)
    size_t lSrc1 = lSankey.addNode("Source A", RED, 0);
    size_t lSrc2 = lSankey.addNode("Source B", GREEN, 0);
    size_t lMid = lSankey.addNode("Middle", BLUE, 1);
    size_t lDst1 = lSankey.addNode("Target X", ORANGE, 2);
    size_t lDst2 = lSankey.addNode("Target Y", PURPLE, 2);

    // Add links (value = flow weight)
    lSankey.addLink(lSrc1, lMid, 50.0f);
    lSankey.addLink(lSrc2, lMid, 30.0f);
    lSankey.addLink(lMid, lDst1, 45.0f);
    lSankey.addLink(lMid, lDst2, 35.0f);

    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        lSankey.update(lDt);

        BeginDrawing();
        ClearBackground(DARKGRAY);
        lSankey.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

### With Custom Styling

```cpp
RLSankeyStyle lStyle;
lStyle.mBackground = Color{20, 22, 28, 255};
lStyle.mNodeWidth = 24.0f;
lStyle.mNodePadding = 12.0f;
lStyle.mNodeCornerRadius = 6.0f;
lStyle.mLinkAlpha = 0.7f;
lStyle.mLinkColorMode = RLSankeyLinkColorMode::GRADIENT;
lStyle.mShowLabels = true;
lStyle.mLabelFontSize = 16;

RLSankey lSankey(lBounds, lStyle);
```

### Flow Mode Configuration

```cpp
RLSankeyStyle lStyle;

// NORMALIZED mode (default): Link bands fill node height on both sides
// This is the proper Sankey behavior where bands are scaled proportionally
lStyle.mFlowMode = RLSankeyFlowMode::NORMALIZED;

// RAW_VALUE mode: Link thickness directly represents value
// Bands may not fill node height if inflow ≠ outflow
lStyle.mFlowMode = RLSankeyFlowMode::RAW_VALUE;

RLSankey lSankey(lBounds, lStyle);
```

### Strict Flow Conservation

```cpp
RLSankeyStyle lStyle;
lStyle.mStrictFlowConservation = true;  // Enable validation
lStyle.mFlowTolerance = 0.01f;          // Allow small rounding errors

RLSankey lSankey(lBounds, lStyle);

// setData returns false and logs warnings if flow conservation is violated
bool lValid = lSankey.setData(lNodes, lLinks);
if (!lValid) {
    // Handle flow conservation violation
    // Warnings are logged for each node where inflow ≠ outflow
}

// You can also validate at any time
if (!lSankey.validateFlowConservation()) {
    // Flow is not conserved at one or more intermediate nodes
}
```

### Dynamic Updates with Animation

```cpp
// Values animate smoothly when changed
lSankey.setLinkValue(0, 75.0f);  // Animate link 0 to new value

// Nodes fade in when added
size_t lNewNode = lSankey.addNode("New Node", YELLOW, 1);
lSankey.addLink(0, lNewNode, 20.0f);

// Nodes fade out when removed
lSankey.removeNode(lNewNode);

// Check if removal animation is complete before adding new nodes
if (!lSankey.hasPendingRemovals()) {
    // Safe to add new nodes - all pending removals have completed
    lNewNode = lSankey.addNode("Another Node", GREEN, 1);
}
```

**Important:** When nodes are removed, they first fade out (marked as pending removal) and are only physically removed from the internal vector once fully faded. When nodes are physically removed, all link source/target IDs are automatically updated to maintain correct references. However, any node IDs stored externally (like `lNewNode` above) become invalid after the physical removal occurs. Use `hasPendingRemovals()` to check if removal animations have completed before adding new nodes.

### Interactive Hover

```cpp
while (!WindowShouldClose()) {
    Vector2 lMouse = GetMousePosition();
    
    int lHoveredNode = lSankey.getHoveredNode(lMouse);
    int lHoveredLink = (lHoveredNode < 0) ? lSankey.getHoveredLink(lMouse) : -1;
    
    lSankey.setHighlightedNode(lHoveredNode);
    lSankey.setHighlightedLink(lHoveredLink);
    
    lSankey.update(GetFrameTime());
    
    BeginDrawing();
    lSankey.draw();
    EndDrawing();
}
```

## Demo

Run the `sankey` demo to see a full interactive example with:
- Multi-layer energy flow visualization
- Website analytics flow
- Dynamic value fluctuations
- Color mode toggling (press C)
- Flow mode toggling (press N) - switch between Normalized and Raw Value
- Strict flow conservation toggle (press S) - validates inflow == outflow
- Label visibility toggle (press L)
- Add/remove nodes dynamically (press A/R)
- Hover highlighting

```bash
./raylib_sankey
```

## Performance Notes

- Bezier curves are cached and only recomputed when link values change
- Layout is computed once and cached until data changes
- Alpha-blended ribbons use triangle strips for efficient rendering
- Animation uses smooth interpolation for minimal visual artifacts

