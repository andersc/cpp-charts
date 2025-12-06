# RLTreeMap

A D3-style treemap visualization for raylib supporting hierarchical data with smooth animations.

## Features

- Squarified treemap layout (best aspect ratios) plus slice/dice options
- Smooth animations between data states
- Hierarchical data support with parent/child nesting
- Customizable colors, labels, and styling
- Interactive node highlighting
- Depth-based color palettes or custom per-node colors

## Constructor

```cpp
RLTreeMap(Rectangle aBounds, const RLTreeMapStyle &rStyle = {});
```

**Parameters:**
- `aBounds` - The rectangle defining the treemap's position and size
- `rStyle` - Optional style configuration

## Data Structure

```cpp
struct RLTreeNode {
    std::string mLabel;                     // Node label
    float mValue{0.0f};                     // Leaf value (determines area)
    Color mColor{80, 180, 255, 255};        // Node color
    bool mUseColor{false};                  // If false, use color mapping rules
    std::vector<RLTreeNode> mChildren;      // Child nodes (empty for leaves)
};
```

## Style Configuration

```cpp
struct RLTreeMapStyle {
    // Background
    bool mShowBackground{true};
    Color mBackground{20, 22, 28, 255};

    // Node appearance
    float mPaddingOuter{4.0f};          // Padding around entire treemap
    float mPaddingInner{2.0f};          // Padding between sibling nodes
    float mPaddingTop{18.0f};           // Extra top padding for parent labels
    float mBorderThickness{1.0f};       // Border thickness around nodes
    Color mBorderColor{40, 44, 52, 255};
    float mCornerRadius{3.0f};          // Rounded corner radius

    // Internal nodes
    bool mShowInternalNodes{true};      // Show rectangles for internal nodes
    Color mInternalNodeColor{30, 32, 40, 200};
    bool mShowInternalLabels{true};     // Show labels on internal nodes

    // Leaf nodes
    bool mShowLeafLabels{true};         // Show labels on leaf nodes
    float mMinNodeSize{8.0f};           // Minimum node size to display
    bool mLabelFitCheck{true};          // Only show label if it fits

    // Labels
    int mLabelFontSize{14};
    Color mLabelColor{230, 230, 240, 255};
    bool mAutoLabelColor{true};         // White/black based on background
    Font mLabelFont{};                  // Optional custom font; if .baseSize==0 use default

    // Animation
    bool mSmoothAnimate{true};
    float mAnimateSpeed{6.0f};          // Size transition speed
    float mColorSpeed{4.0f};            // Color blend speed

    // Color mapping
    bool mUseDepthColors{true};         // Color by depth level
    std::vector<Color> mDepthPalette;   // Colors for each depth level
};
```

## Layout Algorithms

```cpp
enum class RLTreeMapLayout {
    SQUARIFIED,     // Squarified treemap (best aspect ratios)
    SLICE,          // Simple vertical slice layout
    DICE,           // Simple horizontal dice layout
    SLICE_DICE      // Alternating slice/dice by depth
};
```

## Methods

### Configuration

| Method | Description |
|--------|-------------|
| `setBounds(Rectangle aBounds)` | Set the treemap bounds |
| `setStyle(const RLTreeMapStyle &rStyle)` | Apply a style configuration |
| `setLayout(RLTreeMapLayout aLayout)` | Set the layout algorithm |

### Data

| Method | Description |
|--------|-------------|
| `setData(const RLTreeNode &rRoot)` | Set data immediately (no animation) |
| `setTargetData(const RLTreeNode &rRoot)` | Set target data (animates to new values) |
| `updateValue(const std::vector<std::string> &rPath, float aNewValue)` | Update a specific node's value by path |
| `recomputeLayout()` | Force layout recomputation |

### Rendering

| Method | Description |
|--------|-------------|
| `update(float aDt)` | Update animations (call each frame with delta time) |
| `draw() const` | Draw the treemap |

### Interaction

| Method | Description |
|--------|-------------|
| `getNodeAtPoint(Vector2 aPoint) const` | Get node index at screen position (-1 if none) |
| `setHighlightedNode(int aIndex)` | Set which node to highlight |
| `getHighlightedNode() const` | Get currently highlighted node index |

### Getters

| Method | Description |
|--------|-------------|
| `getBounds() const` | Get current bounds |
| `getComputedRects() const` | Get computed rectangles for all nodes |
| `getNodeCount() const` | Get total number of nodes |

## Complete Example

```cpp
#include "raylib.h"
#include "RLTreeMap.h"
#include <vector>

int main() {
    InitWindow(800, 600, "TreeMap Example");
    SetTargetFPS(60);
    
    // Define treemap area
    Rectangle lBounds = {50, 50, 700, 500};
    
    // Create treemap with default style
    RLTreeMapStyle lStyle;
    lStyle.mBackground = Color{20, 22, 28, 255};
    lStyle.mShowInternalNodes = true;
    lStyle.mShowLeafLabels = true;
    
    RLTreeMap lTreeMap(lBounds, lStyle);
    lTreeMap.setLayout(RLTreeMapLayout::SQUARIFIED);
    
    // Build hierarchy
    RLTreeNode lRoot;
    lRoot.mLabel = "Portfolio";
    
    RLTreeNode lStocks;
    lStocks.mLabel = "Stocks";
    lStocks.mChildren.push_back({"AAPL", 150.0f, GREEN, true, {}});
    lStocks.mChildren.push_back({"GOOGL", 120.0f, BLUE, true, {}});
    lStocks.mChildren.push_back({"MSFT", 100.0f, ORANGE, true, {}});
    lRoot.mChildren.push_back(lStocks);
    
    RLTreeNode lBonds;
    lBonds.mLabel = "Bonds";
    lBonds.mChildren.push_back({"Treasury", 200.0f, PURPLE, true, {}});
    lBonds.mChildren.push_back({"Corporate", 80.0f, SKYBLUE, true, {}});
    lRoot.mChildren.push_back(lBonds);
    
    // Set data
    lTreeMap.setData(lRoot);
    
    while (!WindowShouldClose()) {
        float lDt = GetFrameTime();
        
        // Interactive highlighting
        Vector2 lMouse = GetMousePosition();
        int lHovered = lTreeMap.getNodeAtPoint(lMouse);
        lTreeMap.setHighlightedNode(lHovered);
        
        lTreeMap.update(lDt);  // Animate
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        lTreeMap.draw();       // Render
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

## Animated Updates

```cpp
// Update specific node value
lTreeMap.updateValue({"Stocks", "AAPL"}, 200.0f);

// Or update entire tree with animation
RLTreeNode lNewRoot = lRoot;
lNewRoot.mChildren[0].mChildren[0].mValue = 200.0f;
lTreeMap.setTargetData(lNewRoot);
```

## Layout Comparison

| Layout | Description | Best For |
|--------|-------------|----------|
| `SQUARIFIED` | Optimizes aspect ratios to be more square | General use, best visual appeal |
| `SLICE` | Stacks children vertically | Deep hierarchies |
| `DICE` | Stacks children horizontally | Wide hierarchies |
| `SLICE_DICE` | Alternates direction by depth | Balanced hierarchies |

## Tips

1. **Squarified layout** produces the most visually appealing results for most data
2. Use **custom colors** (`mUseColor = true`) for category-based coloring
3. Enable **mShowInternalNodes** to see the hierarchy structure
4. Set **mLabelFitCheck** to `true` to avoid overlapping labels
5. Use **setTargetData()** for smooth animated transitions when data changes
6. Call **getNodeAtPoint()** for interactive tooltip/selection behavior

