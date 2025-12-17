// RLTreeMap.cpp
#include "RLTreeMap.h"
#include <algorithm>
#include <cmath>
#include <utility>

// Default color palette for depth-based coloring
static constexpr Color DEFAULT_DEPTH_PALETTE[] = {
    Color{60, 70, 90, 255},         // Level 0 (root background)
    Color{0, 150, 199, 255},        // Level 1
    Color{80, 200, 120, 255},       // Level 2
    Color{255, 160, 80, 255},       // Level 3
    Color{220, 80, 120, 255},       // Level 4
    Color{160, 100, 220, 255},      // Level 5
    Color{255, 200, 60, 255},       // Level 6
    Color{80, 200, 200, 255}        // Level 7+
};
static constexpr int DEFAULT_PALETTE_SIZE = 8;

RLTreeMap::RLTreeMap(Rectangle aBounds, RLTreeMapStyle  aStyle)
    : mBounds(aBounds)
    , mStyle(std::move(aStyle))
{
    ensureDefaultPalette();
}

void RLTreeMap::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
    mDataDirty = true;
}

void RLTreeMap::setStyle(const RLTreeMapStyle& rStyle) {
    mStyle = rStyle;
    ensureDefaultPalette();
    mDataDirty = true;
}

void RLTreeMap::setLayout(RLTreeMapLayout aLayout) {
    mLayout = aLayout;
    mDataDirty = true;
}

void RLTreeMap::setData(const RLTreeNode& rRoot) {
    mRoot = rRoot;
    computeLayout();

    // Set current state to target (no animation)
    for (auto& rRect : mRects) {
        rRect.mRect = rRect.mTargetRect;
        rRect.mColor = rRect.mTargetColor;
        rRect.mAlpha = rRect.mTargetAlpha;
    }
}

void RLTreeMap::setTargetData(const RLTreeNode& rRoot) {
    // Store old rects for animation
    const std::vector<RLTreeRect> lOldRects = mRects;

    mRoot = rRoot;
    computeLayout();

    // Match old rects by label for smooth transitions
    for (auto& rRect : mRects) {
        // Try to find matching node in old data
        for (const auto& rOld : lOldRects) {
            if (rOld.mLabel == rRect.mLabel && rOld.mDepth == rRect.mDepth) {
                // Start from old position
                rRect.mRect = rOld.mRect;
                rRect.mColor = rOld.mColor;
                rRect.mAlpha = rOld.mAlpha;
                break;
            }
        }
    }
}

void RLTreeMap::updateValue(const std::vector<std::string>& rPath, float aNewValue) {
    // Navigate to node and update its value
    RLTreeNode* pNode = &mRoot;
    for (const auto& rName : rPath) {
        bool lFound = false;
        for (auto& rChild : pNode->mChildren) {
            if (rChild.mLabel == rName) {
                pNode = &rChild;
                lFound = true;
                break;
            }
        }
        if (!lFound) {
            return; // Path not found
        }
    }
    pNode->mValue = aNewValue;
    setTargetData(mRoot);
}

void RLTreeMap::recomputeLayout() {
    computeLayout();
}

void RLTreeMap::ensureDefaultPalette() {
    if (mStyle.mDepthPalette.empty()) {
        mStyle.mDepthPalette.reserve(DEFAULT_PALETTE_SIZE);
        for (const auto& rColor : DEFAULT_DEPTH_PALETTE) {
            mStyle.mDepthPalette.push_back(rColor);
        }
    }
}

void RLTreeMap::computeLayout() {
    mRects.clear();
    mTargetRects.clear();

    if (mRoot.mLabel.empty() && mRoot.mChildren.empty()) {
        return;
    }

    // Flatten hierarchy into mRects
    flattenHierarchy(mRoot, 0, (size_t)-1);

    // Calculate the available area
    const Rectangle lAvailable = {
        mBounds.x + mStyle.mPaddingOuter,
        mBounds.y + mStyle.mPaddingOuter,
        mBounds.width - 2.0f * mStyle.mPaddingOuter,
        mBounds.height - 2.0f * mStyle.mPaddingOuter
    };

    // Start layout from root
    if (!mRects.empty()) {
        layoutNode(0, lAvailable, 0);
    }

    mDataDirty = false;
}

float RLTreeMap::computeSubtreeValue(const RLTreeNode& rNode) const {
    if (rNode.mChildren.empty()) {
        return rNode.mValue;
    }
    float lSum = 0.0f;
    for (const auto& rChild : rNode.mChildren) {
        lSum += computeSubtreeValue(rChild);
    }
    return lSum;
}

void RLTreeMap::flattenHierarchy(const RLTreeNode& rNode, int aDepth, size_t aParentIdx) {
    const size_t lMyIndex = mRects.size();

    RLTreeRect lRect;
    lRect.mLabel = rNode.mLabel;
    lRect.mDepth = aDepth;
    lRect.mIsLeaf = rNode.mChildren.empty();
    lRect.mParentIndex = aParentIdx;
    lRect.mAlpha = 1.0f;
    lRect.mTargetAlpha = 1.0f;

    // Calculate value: for leaves use mValue, for internal nodes sum children
    if (lRect.mIsLeaf) {
        lRect.mValue = rNode.mValue;
    } else {
        float lSum = 0.0f;
        for (const auto& rChild : rNode.mChildren) {
            lSum += computeSubtreeValue(rChild);
        }
        lRect.mValue = lSum;
    }

    // Compute color
    lRect.mTargetColor = computeNodeColor(rNode, aDepth);
    lRect.mColor = lRect.mTargetColor;

    mRects.push_back(lRect);

    // Process children
    for (const auto& rChild : rNode.mChildren) {
        flattenHierarchy(rChild, aDepth + 1, lMyIndex);
    }
}

void RLTreeMap::layoutNode(size_t aNodeIdx, Rectangle aAvailable, int aDepth) {
    if (aNodeIdx >= mRects.size()) {
        return;
    }

    RLTreeRect& rNode = mRects[aNodeIdx];
    rNode.mTargetRect = aAvailable;

    // Find children of this node
    std::vector<size_t> lChildIndices;
    for (size_t i = aNodeIdx + 1; i < mRects.size(); ++i) {
        if (mRects[i].mParentIndex == aNodeIdx) {
            lChildIndices.push_back(i);
        }
    }

    if (lChildIndices.empty()) {
        // Leaf node - just set the rect
        return;
    }

    // Calculate child area (accounting for internal node padding if showing)
    Rectangle lChildArea = aAvailable;
    if (mStyle.mShowInternalNodes) {
        lChildArea.x += mStyle.mPaddingInner;
        lChildArea.y += mStyle.mPaddingTop;  // Extra space for label
        lChildArea.width -= 2.0f * mStyle.mPaddingInner;
        lChildArea.height -= mStyle.mPaddingTop + mStyle.mPaddingInner;
    }

    // Skip if area too small
    if (lChildArea.width < mStyle.mMinNodeSize || lChildArea.height < mStyle.mMinNodeSize) {
        return;
    }

    // Apply layout algorithm
    switch (mLayout) {
        case RLTreeMapLayout::SQUARIFIED:
            layoutSquarified(lChildIndices, lChildArea);
            break;
        case RLTreeMapLayout::SLICE:
            layoutSlice(lChildIndices, lChildArea, true);
            break;
        case RLTreeMapLayout::DICE:
            layoutSlice(lChildIndices, lChildArea, false);
            break;
        case RLTreeMapLayout::SLICE_DICE:
            layoutSlice(lChildIndices, lChildArea, (aDepth % 2) == 0);
            break;
    }

    // Recursively layout children
    for (const size_t lIdx : lChildIndices) {
        // Find grandchildren depth
        const int lChildDepth = mRects[lIdx].mDepth;
        layoutNode(lIdx, mRects[lIdx].mTargetRect, lChildDepth);
    }
}

float RLTreeMap::sumChildValues(const std::vector<size_t>& rIndices) const {
    float lSum = 0.0f;
    for (const size_t i : rIndices) {
        lSum += mRects[i].mValue;
    }
    return lSum;
}

void RLTreeMap::layoutSquarified(std::vector<size_t>& rChildIndices, Rectangle aAvailable) {
    if (rChildIndices.empty()) {
        return;
    }

    // Sort children by value (descending) for better squarification
    std::sort(rChildIndices.begin(), rChildIndices.end(),
              [this](size_t a, size_t b) {
                  return mRects[a].mValue > mRects[b].mValue;
              });

    const float lTotalValue = sumChildValues(rChildIndices);
    if (lTotalValue <= 0.0f) {
        return;
    }

    // Working variables
    Rectangle lRemaining = aAvailable;
    std::vector<size_t> lRow;
    float lRowValue = 0.0f;
    size_t lIdx = 0;

    while (lIdx < rChildIndices.size()) {
        // Determine a layout direction based on the remaining area
        const bool lVertical = lRemaining.width >= lRemaining.height;
        const float lSide = lVertical ? lRemaining.height : lRemaining.width;

        if (lSide <= 0.0f) {
            break;
        }

        // Calculate remaining total value
        float lRemainingValue = 0.0f;
        for (size_t i = lIdx; i < rChildIndices.size(); ++i) {
            lRemainingValue += mRects[rChildIndices[i]].mValue;
        }

        // Try adding nodes to the current row
        lRow.clear();
        lRowValue = 0.0f;
        float lBestAspect = std::numeric_limits<float>::max();

        while (lIdx < rChildIndices.size()) {
            const float lNodeValue = mRects[rChildIndices[lIdx]].mValue;
            const float lTestRowValue = lRowValue + lNodeValue;
            const float lRowFraction = lTestRowValue / lRemainingValue;
            const float lRowSize = lVertical ? (lRemaining.width * lRowFraction) : (lRemaining.height * lRowFraction);

            // Calculate worst aspect ratio if we add this node
            float lWorstAspect = 0.0f;

            for (const size_t lRowItem : lRow) {
                const float lFrac = mRects[lRowItem].mValue / lTestRowValue;
                const float lNodeSize = lSide * lFrac;
                const float lAspect = (lRowSize > lNodeSize) ? (lRowSize / lNodeSize) : (lNodeSize / lRowSize);
                lWorstAspect = std::max(lAspect, lWorstAspect);
            }

            // Add current node
            const float lFrac = lNodeValue / lTestRowValue;
            const float lNodeSize = lSide * lFrac;
            const float lAspect = (lRowSize > lNodeSize) ? (lRowSize / lNodeSize) : (lNodeSize / lRowSize);
            lWorstAspect = std::max(lAspect, lWorstAspect);

            if (lRow.empty() || lWorstAspect <= lBestAspect) {
                // Accept this node
                lRow.push_back(rChildIndices[lIdx]);
                lRowValue = lTestRowValue;
                lBestAspect = lWorstAspect;
                ++lIdx;
            } else {
                // Reject - finalize current row
                break;
            }
        }

        // Layout the row
        if (!lRow.empty()) {
            const float lRowFraction = lRowValue / lRemainingValue;
            const float lRowSize = lVertical ? (lRemaining.width * lRowFraction) : (lRemaining.height * lRowFraction);
            float lOffset = 0.0f;

            for (const size_t lRowIdx : lRow) {
                const float lFrac = mRects[lRowIdx].mValue / lRowValue;
                const float lNodeSize = lSide * lFrac;

                Rectangle lNodeRect;
                if (lVertical) {
                    lNodeRect.x = lRemaining.x;
                    lNodeRect.y = lRemaining.y + lOffset;
                    lNodeRect.width = lRowSize - mStyle.mPaddingInner;
                    lNodeRect.height = lNodeSize - mStyle.mPaddingInner;
                } else {
                    lNodeRect.x = lRemaining.x + lOffset;
                    lNodeRect.y = lRemaining.y;
                    lNodeRect.width = lNodeSize - mStyle.mPaddingInner;
                    lNodeRect.height = lRowSize - mStyle.mPaddingInner;
                }

                // Clamp to positive dimensions
                lNodeRect.width = std::max<float>(lNodeRect.width, 0);
                lNodeRect.height = std::max<float>(lNodeRect.height, 0);

                mRects[lRowIdx].mTargetRect = lNodeRect;
                lOffset += lNodeSize;
            }

            // Shrink remaining area
            if (lVertical) {
                lRemaining.x += lRowSize;
                lRemaining.width -= lRowSize;
            } else {
                lRemaining.y += lRowSize;
                lRemaining.height -= lRowSize;
            }
        }
    }
}

void RLTreeMap::layoutSlice(std::vector<size_t>& rChildIndices, Rectangle aAvailable, bool aVertical) {
    const float lTotalValue = sumChildValues(rChildIndices);
    if (lTotalValue <= 0.0f) {
        return;
    }

    float lOffset = 0.0f;
    for (const size_t lIdx : rChildIndices) {
        const float lFrac = mRects[lIdx].mValue / lTotalValue;

        Rectangle lNodeRect;
        if (aVertical) {
            float lHeight = aAvailable.height * lFrac - mStyle.mPaddingInner;
            lHeight = std::max<float>(lHeight, 0);
            lNodeRect = {
                aAvailable.x,
                aAvailable.y + lOffset,
                aAvailable.width - mStyle.mPaddingInner,
                lHeight
            };
            lOffset += aAvailable.height * lFrac;
        } else {
            float lWidth = aAvailable.width * lFrac - mStyle.mPaddingInner;
            lWidth = std::max<float>(lWidth, 0);
            lNodeRect = {
                aAvailable.x + lOffset,
                aAvailable.y,
                lWidth,
                aAvailable.height - mStyle.mPaddingInner
            };
            lOffset += aAvailable.width * lFrac;
        }

        mRects[lIdx].mTargetRect = lNodeRect;
    }
}

Color RLTreeMap::computeNodeColor(const RLTreeNode& rNode, int aDepth) const {
    if (rNode.mUseColor) {
        return rNode.mColor;
    }

    if (mStyle.mUseDepthColors && !mStyle.mDepthPalette.empty()) {
        const size_t lPaletteIdx = (size_t)aDepth % mStyle.mDepthPalette.size();
        return mStyle.mDepthPalette[lPaletteIdx];
    }

    // Default fallback color
    return Color{80, 180, 255, 255};
}

void RLTreeMap::update(float aDt) {
    if (mDataDirty) {
        computeLayout();
    }

    if (!mStyle.mSmoothAnimate) {
        for (auto& rRect : mRects) {
            rRect.mRect = rRect.mTargetRect;
            rRect.mColor = rRect.mTargetColor;
            rRect.mAlpha = rRect.mTargetAlpha;
        }
        return;
    }

    const float lSizeDt = mStyle.mAnimateSpeed * aDt;
    const float lColorDt = mStyle.mColorSpeed * aDt;

    for (auto& rRect : mRects) {
        rRect.mRect = lerpRect(rRect.mRect, rRect.mTargetRect, lSizeDt);
        rRect.mColor = lerpColor(rRect.mColor, rRect.mTargetColor, lColorDt);
        rRect.mAlpha = approach(rRect.mAlpha, rRect.mTargetAlpha, lSizeDt);
    }
}

void RLTreeMap::draw() const {
    // Background
    if (mStyle.mShowBackground) {
        DrawRectangleRec(mBounds, mStyle.mBackground);
    }

    // Draw nodes in order (parents first, then children on top)
    for (size_t i = 0; i < mRects.size(); ++i) {
        const RLTreeRect& rRect = mRects[i];

        // Skip if too small
        if (rRect.mRect.width < mStyle.mMinNodeSize || rRect.mRect.height < mStyle.mMinNodeSize) {
            continue;
        }

        // Skip internal nodes if not showing them (but still need to draw children)
        const bool lIsInternal = !rRect.mIsLeaf;
        if (lIsInternal && !mStyle.mShowInternalNodes) {
            continue;
        }

        // Compute draw color with alpha
        Color lDrawColor = rRect.mColor;
        lDrawColor.a = (unsigned char)((float)lDrawColor.a * rRect.mAlpha);

        // For internal nodes, use internal node color
        if (lIsInternal) {
            lDrawColor = mStyle.mInternalNodeColor;
            lDrawColor.a = (unsigned char)((float)lDrawColor.a * rRect.mAlpha);
        }

        // Draw filled rectangle
        const float lMinDim = rRect.mRect.width < rRect.mRect.height ? rRect.mRect.width : rRect.mRect.height;
        if (mStyle.mCornerRadius > 0.0f && lMinDim > 0.0f) {
            DrawRectangleRounded(rRect.mRect, mStyle.mCornerRadius / lMinDim, 8, lDrawColor);
        } else {
            DrawRectangleRec(rRect.mRect, lDrawColor);
        }

        // Highlight
        if ((int)i == mHighlightedIndex) {
            const Color lHighlight = {255, 255, 255, 60};
            if (mStyle.mCornerRadius > 0.0f && lMinDim > 0.0f) {
                DrawRectangleRounded(rRect.mRect, mStyle.mCornerRadius / lMinDim, 8, lHighlight);
            } else {
                DrawRectangleRec(rRect.mRect, lHighlight);
            }
        }

        // Border
        if (mStyle.mBorderThickness > 0.0f) {
            Color lBorderColor = mStyle.mBorderColor;
            lBorderColor.a = (unsigned char)((float)lBorderColor.a * rRect.mAlpha);
            if (mStyle.mCornerRadius > 0.0f && lMinDim > 0.0f) {
                DrawRectangleRoundedLinesEx(rRect.mRect, mStyle.mCornerRadius / lMinDim,
                                            8, mStyle.mBorderThickness, lBorderColor);
            } else {
                DrawRectangleLinesEx(rRect.mRect, mStyle.mBorderThickness, lBorderColor);
            }
        }

        // Label
        const bool lShowLabel = (rRect.mIsLeaf && mStyle.mShowLeafLabels) ||
                          (!rRect.mIsLeaf && mStyle.mShowInternalLabels);

        if (lShowLabel && !rRect.mLabel.empty()) {
            const int lFontSize = mStyle.mLabelFontSize;
            const Font& lFont = (mStyle.mLabelFont.baseSize > 0) ? mStyle.mLabelFont : GetFontDefault();
            const Vector2 lTextSize = MeasureTextEx(lFont, rRect.mLabel.c_str(), (float)lFontSize, 0);
            const int lTextWidth = (int)lTextSize.x;
            const int lTextHeight = (int)lTextSize.y;

            // Check if label fits
            bool lFits = true;
            if (mStyle.mLabelFitCheck) {
                const float lPadding = 4.0f;
                lFits = ((float)lTextWidth + 2.0f * lPadding <= rRect.mRect.width) &&
                        ((float)lTextHeight + 2.0f * lPadding <= rRect.mRect.height);
            }

            if (lFits) {
                // Determine label color
                Color lLabelColor = mStyle.mLabelColor;
                if (mStyle.mAutoLabelColor) {
                    // Calculate luminance
                    const float lLuma = 0.2126f * (float)lDrawColor.r + 0.7152f * (float)lDrawColor.g + 0.0722f * (float)lDrawColor.b;
                    lLabelColor = (lLuma > 140.0f) ? Color{20, 20, 20, 255} : Color{240, 240, 240, 255};
                }
                lLabelColor.a = (unsigned char)((float)lLabelColor.a * rRect.mAlpha);

                // Position: center for leaves, top-left for internal
                float lX, lY;
                if (rRect.mIsLeaf) {
                    lX = rRect.mRect.x + (rRect.mRect.width - (float)lTextWidth) * 0.5f;
                    lY = rRect.mRect.y + (rRect.mRect.height - (float)lTextHeight) * 0.5f;
                } else {
                    lX = rRect.mRect.x + 4.0f;
                    lY = rRect.mRect.y + 2.0f;
                }

                DrawTextEx(lFont, rRect.mLabel.c_str(), Vector2{lX, lY}, (float)lFontSize, 0, lLabelColor);
            }
        }
    }
}

int RLTreeMap::getNodeAtPoint(Vector2 aPoint) const {
    // Return deepest node containing the point
    int lResult = -1;
    for (size_t i = 0; i < mRects.size(); ++i) {
        const Rectangle& rR = mRects[i].mRect;
        if (aPoint.x >= rR.x && aPoint.x <= rR.x + rR.width &&
            aPoint.y >= rR.y && aPoint.y <= rR.y + rR.height) {
            lResult = (int)i;
        }
    }
    return lResult;
}

void RLTreeMap::setHighlightedNode(int aIndex) {
    mHighlightedIndex = aIndex;
}

float RLTreeMap::approach(float a, float b, float aSpeedDt) {
    const float lDiff = b - a;
    if (lDiff * lDiff < 1e-8f) {
        return b;
    }
    return a + lDiff * (aSpeedDt > 1.0f ? 1.0f : aSpeedDt);
}

Color RLTreeMap::lerpColor(const Color& a, const Color& b, float t) {
    if (t >= 1.0f) {
        return b;
    }
    if (t <= 0.0f) {
        return a;
    }
    Color lResult;
    lResult.r = (unsigned char)((float)a.r + ((float)b.r - (float)a.r) * t);
    lResult.g = (unsigned char)((float)a.g + ((float)b.g - (float)a.g) * t);
    lResult.b = (unsigned char)((float)a.b + ((float)b.b - (float)a.b) * t);
    lResult.a = (unsigned char)((float)a.a + ((float)b.a - (float)a.a) * t);
    return lResult;
}

Rectangle RLTreeMap::lerpRect(const Rectangle& a, const Rectangle& b, float t) {
    if (t >= 1.0f) {
        return b;
    }
    if (t <= 0.0f) {
        return a;
    }
    Rectangle lResult;
    lResult.x = a.x + (b.x - a.x) * t;
    lResult.y = a.y + (b.y - a.y) * t;
    lResult.width = a.width + (b.width - a.width) * t;
    lResult.height = a.height + (b.height - a.height) * t;
    return lResult;
}
