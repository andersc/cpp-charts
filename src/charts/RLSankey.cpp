// RLSankey.cpp
#include "RLSankey.h"
#include <cmath>
#include <algorithm>
#include <queue>
#include <set>

// ============================================================================
// Constructor
// ============================================================================

RLSankey::RLSankey(Rectangle aBounds, const RLSankeyStyle& rStyle)
    : mBounds(aBounds)
    , mStyle(rStyle)
{
    mLayoutDirty = true;
}

// ============================================================================
// Configuration
// ============================================================================

void RLSankey::setBounds(Rectangle aBounds) {
    mBounds = aBounds;
    mLayoutDirty = true;
}

void RLSankey::setStyle(const RLSankeyStyle& rStyle) {
    mStyle = rStyle;
    mLayoutDirty = true;
}

// ============================================================================
// Node Management
// ============================================================================

size_t RLSankey::addNode(const std::string& rLabel, Color aColor, int aColumn) {
    RLSankeyNode lNode;
    lNode.mLabel = rLabel;
    lNode.mColor = aColor;
    lNode.mColumn = aColumn;
    return addNode(lNode);
}

size_t RLSankey::addNode(const RLSankeyNode& rNode) {
    NodeDyn lDyn;
    lDyn.mLabel = rNode.mLabel;
    lDyn.mColor = rNode.mColor;
    lDyn.mColorTarget = rNode.mColor;
    lDyn.mColumn = rNode.mColumn;
    lDyn.mVisibility = 0.0f;
    lDyn.mVisibilityTarget = 1.0f;
    lDyn.mPendingRemoval = false;

    mNodes.push_back(lDyn);
    mLayoutDirty = true;
    return mNodes.size() - 1;
}

void RLSankey::setNodeColor(size_t aNodeId, Color aColor) {
    if (aNodeId >= mNodes.size()) {
        return;
    }
    mNodes[aNodeId].mColorTarget = aColor;
}

void RLSankey::setNodeColumn(size_t aNodeId, int aColumn) {
    if (aNodeId >= mNodes.size()) {
        return;
    }
    mNodes[aNodeId].mColumn = aColumn;
    mLayoutDirty = true;
}

void RLSankey::removeNode(size_t aNodeId) {
    if (aNodeId >= mNodes.size()) {
        return;
    }

    // Mark node for removal
    mNodes[aNodeId].mVisibilityTarget = 0.0f;
    mNodes[aNodeId].mPendingRemoval = true;
    mNodes[aNodeId].mHeightTarget = 0.0f;

    // Also remove all links connected to this node
    for (auto& rLink : mLinks) {
        if (rLink.mSourceId == aNodeId || rLink.mTargetId == aNodeId) {
            rLink.mVisibilityTarget = 0.0f;
            rLink.mPendingRemoval = true;
            rLink.mSourceThicknessTarget = 0.0f;
            rLink.mTargetThicknessTarget = 0.0f;
        }
    }
}

// ============================================================================
// Link Management
// ============================================================================

size_t RLSankey::addLink(size_t aSourceId, size_t aTargetId, float aValue, Color aColor) {
    RLSankeyLink lLink;
    lLink.mSourceId = aSourceId;
    lLink.mTargetId = aTargetId;
    lLink.mValue = aValue;
    lLink.mColor = aColor;
    return addLink(lLink);
}

size_t RLSankey::addLink(const RLSankeyLink& rLink) {
    LinkDyn lDyn;
    lDyn.mSourceId = rLink.mSourceId;
    lDyn.mTargetId = rLink.mTargetId;
    lDyn.mValue = 0.0f;             // Start at 0, animate to target
    lDyn.mValueTarget = rLink.mValue;
    lDyn.mColor = rLink.mColor;
    lDyn.mColorTarget = rLink.mColor;
    lDyn.mVisibility = 0.0f;
    lDyn.mVisibilityTarget = 1.0f;
    lDyn.mPendingRemoval = false;
    lDyn.mCacheDirty = true;

    mLinks.push_back(lDyn);
    mLayoutDirty = true;
    return mLinks.size() - 1;
}

void RLSankey::setLinkValue(size_t aLinkId, float aValue) {
    if (aLinkId >= mLinks.size()) {
        return;
    }
    mLinks[aLinkId].mValueTarget = aValue;
    mLinks[aLinkId].mCacheDirty = true;
    mLayoutDirty = true;
}

void RLSankey::setLinkColor(size_t aLinkId, Color aColor) {
    if (aLinkId >= mLinks.size()) {
        return;
    }
    mLinks[aLinkId].mColorTarget = aColor;
}

void RLSankey::removeLink(size_t aLinkId) {
    if (aLinkId >= mLinks.size()) {
        return;
    }

    mLinks[aLinkId].mVisibilityTarget = 0.0f;
    mLinks[aLinkId].mPendingRemoval = true;
    mLinks[aLinkId].mSourceThicknessTarget = 0.0f;
    mLinks[aLinkId].mTargetThicknessTarget = 0.0f;
    mLinks[aLinkId].mValueTarget = 0.0f;
}

// ============================================================================
// Batch Data
// ============================================================================

bool RLSankey::setData(const std::vector<RLSankeyNode>& rNodes, const std::vector<RLSankeyLink>& rLinks) {
    clear();

    for (const auto& rNode : rNodes) {
        addNode(rNode);
    }

    for (const auto& rLink : rLinks) {
        addLink(rLink);
    }

    // Validate flow conservation if the strict mode is enabled
    if (mStyle.mStrictFlowConservation) {
        return validateFlowConservation();
    }
    return true;
}

void RLSankey::clear() {
    mNodes.clear();
    mLinks.clear();
    mLayoutDirty = true;
    mColumnCount = 0;
}

// ============================================================================
// Getters
// ============================================================================

size_t RLSankey::getNodeCount() const {
    size_t lCount = 0;
    for (const auto& rNode : mNodes) {
        if (!rNode.mPendingRemoval) {
            lCount++;
        }
    }
    return lCount;
}

size_t RLSankey::getLinkCount() const {
    size_t lCount = 0;
    for (const auto& rLink : mLinks) {
        if (!rLink.mPendingRemoval) {
            lCount++;
        }
    }
    return lCount;
}

// ============================================================================
// Update (Animation)
// ============================================================================

void RLSankey::update(float aDt) {
    // Recompute layout if dirty
    if (mLayoutDirty) {
        computeLayout();
        mLayoutDirty = false;
    }

    if (!mStyle.mSmoothAnimate) {
        // Instant update
        for (auto& rNode : mNodes) {
            rNode.mY = rNode.mYTarget;
            rNode.mHeight = rNode.mHeightTarget;
            rNode.mColor = rNode.mColorTarget;
            rNode.mVisibility = rNode.mVisibilityTarget;
        }
        for (auto& rLink : mLinks) {
            rLink.mValue = rLink.mValueTarget;
            rLink.mSourceThickness = rLink.mSourceThicknessTarget;
            rLink.mTargetThickness = rLink.mTargetThicknessTarget;
            rLink.mSourceY = rLink.mSourceYTarget;
            rLink.mTargetY = rLink.mTargetYTarget;
            rLink.mColor = rLink.mColorTarget;
            rLink.mVisibility = rLink.mVisibilityTarget;
            rLink.mCacheDirty = true;
        }
    } else {
        const float lValueSpeed = mStyle.mAnimateSpeed * aDt;
        const float lFadeSpeed = mStyle.mFadeSpeed * aDt;

        // Animate nodes
        for (auto& rNode : mNodes) {
            rNode.mY = RLCharts::approach(rNode.mY, rNode.mYTarget, lValueSpeed);
            rNode.mHeight = RLCharts::approach(rNode.mHeight, rNode.mHeightTarget, lValueSpeed);
            rNode.mColor = RLCharts::lerpColor(rNode.mColor, rNode.mColorTarget, lValueSpeed);
            rNode.mVisibility = RLCharts::approach(rNode.mVisibility, rNode.mVisibilityTarget, lFadeSpeed);
        }

        // Animate links
        for (auto& rLink : mLinks) {
            const float lOldValue = rLink.mValue;
            const float lOldSourceThickness = rLink.mSourceThickness;
            const float lOldTargetThickness = rLink.mTargetThickness;
            rLink.mValue = RLCharts::approach(rLink.mValue, rLink.mValueTarget, lValueSpeed);
            rLink.mSourceThickness = RLCharts::approach(rLink.mSourceThickness, rLink.mSourceThicknessTarget, lValueSpeed);
            rLink.mTargetThickness = RLCharts::approach(rLink.mTargetThickness, rLink.mTargetThicknessTarget, lValueSpeed);
            rLink.mSourceY = RLCharts::approach(rLink.mSourceY, rLink.mSourceYTarget, lValueSpeed);
            rLink.mTargetY = RLCharts::approach(rLink.mTargetY, rLink.mTargetYTarget, lValueSpeed);
            rLink.mColor = RLCharts::lerpColor(rLink.mColor, rLink.mColorTarget, lValueSpeed);
            rLink.mVisibility = RLCharts::approach(rLink.mVisibility, rLink.mVisibilityTarget, lFadeSpeed);

            if (rLink.mValue != lOldValue ||
                rLink.mSourceThickness != lOldSourceThickness ||
                rLink.mTargetThickness != lOldTargetThickness) {
                rLink.mCacheDirty = true;
            }
        }
    }

    // Remove fully faded-out nodes
    mNodes.erase(
        std::remove_if(mNodes.begin(), mNodes.end(),
            [](const NodeDyn& rN) {
                return rN.mPendingRemoval && rN.mVisibility < 0.001f;
            }),
        mNodes.end()
    );

    // Remove fully faded-out links
    mLinks.erase(
        std::remove_if(mLinks.begin(), mLinks.end(),
            [](const LinkDyn& rL) {
                return rL.mPendingRemoval && rL.mVisibility < 0.001f;
            }),
        mLinks.end()
    );
}

// ============================================================================
// Draw
// ============================================================================

void RLSankey::draw() const {
    drawBackground();
    drawLinks();
    drawNodes();
    drawLabels();
}

// ============================================================================
// Layout Computation
// ============================================================================

void RLSankey::computeLayout() {
    if (mNodes.empty()) {
        return;
    }

    // Compute chart area
    mChartLeft = mBounds.x + mStyle.mPadding;
    mChartTop = mBounds.y + mStyle.mPadding;
    mChartWidth = mBounds.width - 2.0f * mStyle.mPadding;
    mChartHeight = mBounds.height - 2.0f * mStyle.mPadding;

    // Assign columns to nodes
    assignColumns();

    // Compute node positions
    computeNodePositions();

    // Compute link positions
    computeLinkPositions();
}

void RLSankey::assignColumns() {
    // First, check if all nodes have explicit columns
    bool lAllExplicit = true;
    int lMaxColumn = 0;

    for (const auto& rNode : mNodes) {
        if (rNode.mPendingRemoval) {
            continue;
        }
        if (rNode.mColumn < 0) {
            lAllExplicit = false;
        } else {
            lMaxColumn = (rNode.mColumn > lMaxColumn) ? rNode.mColumn : lMaxColumn;
        }
    }

    if (lAllExplicit) {
        mColumnCount = lMaxColumn + 1;
        return;
    }

    // Auto-assign columns using topological ordering based on links
    // Nodes with no incoming links go to column 0
    // Each node goes to max(source columns) + 1

    const size_t lNodeCount = mNodes.size();
    std::vector<int> lComputedColumn(lNodeCount, -1);
    std::vector<std::set<size_t>> lIncoming(lNodeCount);

    // Build incoming link sets
    for (const auto& rLink : mLinks) {
        if (rLink.mPendingRemoval) {
            continue;
        }
        if (rLink.mSourceId < lNodeCount && rLink.mTargetId < lNodeCount) {
            lIncoming[rLink.mTargetId].insert(rLink.mSourceId);
        }
    }

    // Process nodes iteratively
    bool lChanged = true;
    int lIterations = 0;
    const int lMaxIter = (int)lNodeCount + 10;

    while (lChanged && lIterations < lMaxIter) {
        lChanged = false;
        lIterations++;

        for (size_t i = 0; i < lNodeCount; ++i) {
            if (mNodes[i].mPendingRemoval) {
                continue;
            }

            // If node has explicit column, use it
            if (mNodes[i].mColumn >= 0) {
                if (lComputedColumn[i] != mNodes[i].mColumn) {
                    lComputedColumn[i] = mNodes[i].mColumn;
                    lChanged = true;
                }
                continue;
            }

            // If no incoming links, column 0
            if (lIncoming[i].empty()) {
                if (lComputedColumn[i] != 0) {
                    lComputedColumn[i] = 0;
                    lChanged = true;
                }
                continue;
            }

            // Find max column of all sources
            int lMaxSrcCol = -1;
            bool lAllSourcesAssigned = true;

            for (const size_t lSrcId : lIncoming[i]) {
                if (lComputedColumn[lSrcId] < 0) {
                    lAllSourcesAssigned = false;
                    break;
                }
                lMaxSrcCol = (lComputedColumn[lSrcId] > lMaxSrcCol) ? lComputedColumn[lSrcId] : lMaxSrcCol;
            }

            if (lAllSourcesAssigned) {
                const int lNewCol = lMaxSrcCol + 1;
                if (lComputedColumn[i] != lNewCol) {
                    lComputedColumn[i] = lNewCol;
                    lChanged = true;
                }
            }
        }
    }

    // Apply computed columns and find max
    mColumnCount = 1;
    for (size_t i = 0; i < lNodeCount; ++i) {
        if (mNodes[i].mPendingRemoval) {
            continue;
        }

        int lCol = lComputedColumn[i];
        lCol = std::max(lCol, 0); // Fallback for unassigned

        mNodes[i].mColumn = lCol;
        mColumnCount = std::max(lCol + 1, mColumnCount);
    }
}

void RLSankey::computeNodePositions() {
    if (mColumnCount == 0) {
        return;
    }

    // Group nodes by column
    std::vector<std::vector<size_t>> lColumnNodes(mColumnCount);
    for (size_t i = 0; i < mNodes.size(); ++i) {
        if (mNodes[i].mPendingRemoval) {
            continue;
        }
        const int lCol = mNodes[i].mColumn;
        if (lCol >= 0 && lCol < mColumnCount) {
            lColumnNodes[lCol].push_back(i);
        }
    }

    // Calculate total flow for height scaling
    float lMaxColumnFlow = 0.0f;
    for (int col = 0; col < mColumnCount; ++col) {
        float lColumnFlow = 0.0f;
        for (const size_t lNodeId : lColumnNodes[col]) {
            const float lIn = computeTotalFlow(lNodeId, false);
            const float lOut = computeTotalFlow(lNodeId, true);
            float lNodeFlow = (lIn > lOut) ? lIn : lOut;
            if (lNodeFlow < 0.001f) {
                lNodeFlow = 1.0f; // Minimum for isolated nodes
            }
            lColumnFlow += lNodeFlow;
        }
        // Add padding between nodes
        const float lPaddingTotal = mStyle.mNodePadding * (float)(lColumnNodes[col].size() > 0 ? lColumnNodes[col].size() - 1 : 0);
        lColumnFlow += lPaddingTotal;

        lMaxColumnFlow = std::max(lColumnFlow, lMaxColumnFlow);
    }

    // Compute scale factor
    if (lMaxColumnFlow > 0.001f) {
        mValueToPixelScale = mChartHeight / lMaxColumnFlow;
    } else {
        mValueToPixelScale = 1.0f;
    }

    // Position nodes in each column
    for (int col = 0; col < mColumnCount; ++col) {

        // Calculate the total height for this column to center it
        float lTotalHeight = 0.0f;
        for (const size_t lNodeId : lColumnNodes[col]) {
            const float lIn = computeTotalFlow(lNodeId, false);
            const float lOut = computeTotalFlow(lNodeId, true);
            float lNodeFlow = (lIn > lOut) ? lIn : lOut;
            if (lNodeFlow < 0.001f) {
                lNodeFlow = 1.0f;
            }
            lTotalHeight += lNodeFlow * mValueToPixelScale;
        }
        lTotalHeight += mStyle.mNodePadding * (float)(lColumnNodes[col].size() > 0 ? lColumnNodes[col].size() - 1 : 0);

        // Center column vertically
        float lY = mChartTop + (mChartHeight - lTotalHeight) * 0.5f;

        for (const size_t lNodeId : lColumnNodes[col]) {
            NodeDyn& rNode = mNodes[lNodeId];

            const float lIn = computeTotalFlow(lNodeId, false);
            const float lOut = computeTotalFlow(lNodeId, true);
            float lNodeFlow = (lIn > lOut) ? lIn : lOut;
            if (lNodeFlow < 0.001f) {
                lNodeFlow = 1.0f;
            }

            const float lHeight = lNodeFlow * mValueToPixelScale;

            rNode.mYTarget = lY;
            rNode.mHeightTarget = lHeight;

            // Initialize current values if new
            if (rNode.mVisibility < 0.01f && !rNode.mPendingRemoval) {
                rNode.mY = lY;
                rNode.mHeight = 0.0f; // Start with zero height, grow
            }

            lY += lHeight + mStyle.mNodePadding;
        }
    }
}

void RLSankey::computeLinkPositions() {
    // Reset node flow offsets
    for (auto& rNode : mNodes) {
        rNode.mOutflowOffset = 0.0f;
        rNode.mInflowOffset = 0.0f;
    }

    // Pre-compute per-node scale factors for NORMALIZED mode
    std::vector<float> lOutflowScale(mNodes.size(), 1.0f);
    std::vector<float> lInflowScale(mNodes.size(), 1.0f);

    if (mStyle.mFlowMode == RLSankeyFlowMode::NORMALIZED) {
        for (size_t i = 0; i < mNodes.size(); ++i) {
            if (mNodes[i].mPendingRemoval) {
                continue;
            }

            const float lInflow = computeTotalFlow(i, false);
            const float lOutflow = computeTotalFlow(i, true);
            const float lNodeHeight = mNodes[i].mHeightTarget;

            // Scale factors to make bands fill the node height
            if (lOutflow > 0.001f) {
                lOutflowScale[i] = lNodeHeight / (lOutflow * mValueToPixelScale);
            }
            if (lInflow > 0.001f) {
                lInflowScale[i] = lNodeHeight / (lInflow * mValueToPixelScale);
            }
        }
    }

    // Compute link positions
    for (size_t i = 0; i < mLinks.size(); ++i) {
        LinkDyn& rLink = mLinks[i];
        if (rLink.mPendingRemoval && rLink.mVisibility < 0.01f) {
            continue;
        }

        if (rLink.mSourceId >= mNodes.size() || rLink.mTargetId >= mNodes.size()) {
            continue;
        }

        NodeDyn& rSource = mNodes[rLink.mSourceId];
        NodeDyn& rTarget = mNodes[rLink.mTargetId];

        // Base link thickness from value
        float lBaseThickness = rLink.mValueTarget * mValueToPixelScale;
        lBaseThickness = std::max(lBaseThickness, mStyle.mMinLinkThickness);

        // Compute source and target thicknesses based on flow mode
        float lSourceThickness = lBaseThickness;
        float lTargetThickness = lBaseThickness;

        if (mStyle.mFlowMode == RLSankeyFlowMode::NORMALIZED) {
            lSourceThickness = lBaseThickness * lOutflowScale[rLink.mSourceId];
            lTargetThickness = lBaseThickness * lInflowScale[rLink.mTargetId];
        }

        rLink.mSourceThicknessTarget = lSourceThickness;
        rLink.mTargetThicknessTarget = lTargetThickness;

        // Source Y offset (within source node)
        rLink.mSourceYTarget = rSource.mOutflowOffset;
        rSource.mOutflowOffset += lSourceThickness;

        // Target Y offset (within target node)
        rLink.mTargetYTarget = rTarget.mInflowOffset;
        rTarget.mInflowOffset += lTargetThickness;

        // Initialize current values if new
        if (rLink.mVisibility < 0.01f && !rLink.mPendingRemoval) {
            rLink.mSourceY = rLink.mSourceYTarget;
            rLink.mTargetY = rLink.mTargetYTarget;
            rLink.mSourceThickness = 0.0f; // Start thin, grow
            rLink.mTargetThickness = 0.0f;
        }

        rLink.mCacheDirty = true;
    }
}

float RLSankey::computeTotalFlow(size_t aNodeId, bool aOutgoing) const {
    float lTotal = 0.0f;

    for (const auto& rLink : mLinks) {
        if (rLink.mPendingRemoval) {
            continue;
        }

        const bool lMatches = aOutgoing ? (rLink.mSourceId == aNodeId) : (rLink.mTargetId == aNodeId);
        if (lMatches) {
            lTotal += rLink.mValueTarget;
        }
    }

    return lTotal;
}

bool RLSankey::isIntermediateNode(size_t aNodeId) const {
    if (aNodeId >= mNodes.size()) {
        return false;
    }
    if (mNodes[aNodeId].mPendingRemoval) {
        return false;
    }

    bool lHasIncoming = false;
    bool lHasOutgoing = false;

    for (const auto& rLink : mLinks) {
        if (rLink.mPendingRemoval) {
            continue;
        }

        if (rLink.mSourceId == aNodeId) {
            lHasOutgoing = true;
        }
        if (rLink.mTargetId == aNodeId) {
            lHasIncoming = true;
        }

        if (lHasIncoming && lHasOutgoing) {
            return true;
        }
    }

    return false;
}

bool RLSankey::validateFlowConservation() const {
    bool lValid = true;

    for (size_t i = 0; i < mNodes.size(); ++i) {
        if (!isIntermediateNode(i)) {
            continue;
        }

        const float lInflow = computeTotalFlow(i, false);
        const float lOutflow = computeTotalFlow(i, true);
        const float lDifference = (lInflow > lOutflow) ? (lInflow - lOutflow) : (lOutflow - lInflow);

        if (lDifference > mStyle.mFlowTolerance) {
            TraceLog(LOG_WARNING, "RLSankey: Flow conservation violated at node '%s' (id=%zu): inflow=%.3f, outflow=%.3f, diff=%.3f",
                     mNodes[i].mLabel.c_str(), i, lInflow, lOutflow, lDifference);
            lValid = false;
        }
    }

    return lValid;
}

// ============================================================================
// Drawing Helpers
// ============================================================================

void RLSankey::drawBackground() const {
    if (!mStyle.mShowBackground) {
        return;
    }
    DrawRectangleRec(mBounds, mStyle.mBackground);
}

void RLSankey::drawLinks() const {
    for (const auto& rLink : mLinks) {
        if (rLink.mVisibility > 0.001f) {
            drawLink(rLink);
        }
    }
}

void RLSankey::drawNodes() const {
    for (size_t i = 0; i < mNodes.size(); ++i) {
        if (mNodes[i].mVisibility > 0.001f) {
            drawNode(mNodes[i], i);
        }
    }
}

void RLSankey::drawLabels() const {
    if (!mStyle.mShowLabels) {
        return;
    }

    const Font lFont = mStyle.mLabelFont;
    const int lFontSize = mStyle.mLabelFontSize;
    const Color lColor = mStyle.mLabelColor;
    const bool lUseDefaultFont = (lFont.texture.id == 0);

    for (size_t i = 0; i < mNodes.size(); ++i) {
        const NodeDyn& rNode = mNodes[i];
        if (rNode.mVisibility < 0.01f) {
            continue;
        }
        if (rNode.mLabel.empty()) {
            continue;
        }

        const float lX = getNodeX(rNode.mColumn);
        const float lNodeCenterY = rNode.mY + rNode.mHeight * 0.5f;

        // Measure text
        Vector2 lTextSize;
        if (lUseDefaultFont) {
            lTextSize.x = (float)MeasureText(rNode.mLabel.c_str(), lFontSize);
            lTextSize.y = (float)lFontSize;
        } else {
            lTextSize = MeasureTextEx(lFont, rNode.mLabel.c_str(), (float)lFontSize, 1.0f);
        }

        // Position: left of node for first column, right of node otherwise
        Vector2 lPos;
        if (rNode.mColumn == 0) {
            // Left side label
            lPos.x = lX - mStyle.mLabelPadding - lTextSize.x;
        } else {
            // Right side label (for both middle columns and last column)
            lPos.x = lX + mStyle.mNodeWidth + mStyle.mLabelPadding;
        }
        lPos.y = lNodeCenterY - lTextSize.y * 0.5f;

        // Apply visibility alpha
        Color lDrawColor = lColor;
        lDrawColor.a = static_cast<unsigned char>(static_cast<float>(lDrawColor.a) * rNode.mVisibility);

        // Highlight effect
        if ((int)i == mHighlightedNode) {
            lDrawColor.r = static_cast<unsigned char>(fminf(255.0f, static_cast<float>(lDrawColor.r) * 1.3f));
            lDrawColor.g = static_cast<unsigned char>(fminf(255.0f, static_cast<float>(lDrawColor.g) * 1.3f));
            lDrawColor.b = static_cast<unsigned char>(fminf(255.0f, static_cast<float>(lDrawColor.b) * 1.3f));
        }

        if (lUseDefaultFont) {
            DrawText(rNode.mLabel.c_str(), (int)lPos.x, (int)lPos.y, lFontSize, lDrawColor);
        } else {
            DrawTextEx(lFont, rNode.mLabel.c_str(), lPos, (float)lFontSize, 1.0f, lDrawColor);
        }
    }
}

void RLSankey::drawNode(const NodeDyn& rNode, size_t aNodeId) const {
    const float lX = getNodeX(rNode.mColumn);
    const float lY = rNode.mY;
    const float lWidth = mStyle.mNodeWidth;
    const float lHeight = rNode.mHeight;

    if (lHeight < 1.0f) {
        return;
    }

    // Apply visibility alpha
    Color lColor = rNode.mColor;
    lColor.a = static_cast<unsigned char>(static_cast<float>(lColor.a) * rNode.mVisibility);

    // Highlight effect
    const bool lHighlighted = ((int)aNodeId == mHighlightedNode);
    if (lHighlighted) {
        lColor.r = static_cast<unsigned char>(fminf(255.0f, static_cast<float>(lColor.r) * 1.2f));
        lColor.g = static_cast<unsigned char>(fminf(255.0f, static_cast<float>(lColor.g) * 1.2f));
        lColor.b = static_cast<unsigned char>(fminf(255.0f, static_cast<float>(lColor.b) * 1.2f));
    }

    const Rectangle lRect = {lX, lY, lWidth, lHeight};

    // Draw node rectangle (with optional rounded corners)
    if (mStyle.mNodeCornerRadius > 0.0f) {
        DrawRectangleRounded(lRect, mStyle.mNodeCornerRadius / fminf(lWidth, lHeight), 8, lColor);
    } else {
        DrawRectangleRec(lRect, lColor);
    }

    // Draw border
    if (mStyle.mShowNodeBorder) {
        Color lBorderColor = mStyle.mNodeBorderColor;
        lBorderColor.a = static_cast<unsigned char>(static_cast<float>(lBorderColor.a) * rNode.mVisibility);
        if (lHighlighted) {
            lBorderColor.a = static_cast<unsigned char>(fminf(255.0f, static_cast<float>(lBorderColor.a) * 1.5f));
        }

        if (mStyle.mNodeCornerRadius > 0.0f) {
            DrawRectangleRoundedLines(lRect, mStyle.mNodeCornerRadius / fminf(lWidth, lHeight), 8, lBorderColor);
        } else {
            DrawRectangleLinesEx(lRect, mStyle.mNodeBorderThickness, lBorderColor);
        }
    }
}

void RLSankey::drawLink(const LinkDyn& rLink) const {
    if (rLink.mSourceId >= mNodes.size() || rLink.mTargetId >= mNodes.size()) {
        return;
    }

    const NodeDyn& rSource = mNodes[rLink.mSourceId];
    const NodeDyn& rTarget = mNodes[rLink.mTargetId];

    // Check if link has sufficient thickness to draw
    const float lMaxThickness = (rLink.mSourceThickness > rLink.mTargetThickness)
                          ? rLink.mSourceThickness : rLink.mTargetThickness;
    if (lMaxThickness < 0.5f) {
        return;
    }

    // Compute a curve if dirty
    computeLinkCurve(rLink);

    if (rLink.mCachedTopCurve.size() < 2) {
        return;
    }

    // Determine colors based on mode
    Color lColorStart, lColorEnd;
    float lAlpha = mStyle.mLinkAlpha * rLink.mVisibility;

    switch (mStyle.mLinkColorMode) {
        case RLSankeyLinkColorMode::SOURCE:
            lColorStart = rSource.mColor;
            lColorEnd = rSource.mColor;
            break;
        case RLSankeyLinkColorMode::TARGET:
            lColorStart = rTarget.mColor;
            lColorEnd = rTarget.mColor;
            break;
        case RLSankeyLinkColorMode::CUSTOM:
            lColorStart = rLink.mColor;
            lColorEnd = rLink.mColor;
            break;
        case RLSankeyLinkColorMode::GRADIENT:
        default:
            lColorStart = rSource.mColor;
            lColorEnd = rTarget.mColor;
            break;
    }

    // Check if this link is highlighted
    bool lHighlighted = false;
    for (size_t i = 0; i < mLinks.size(); ++i) {
        if (&mLinks[i] == &rLink && (int)i == mHighlightedLink) {
            lHighlighted = true;
            break;
        }
    }

    if (lHighlighted) {
        lAlpha = fminf(1.0f, lAlpha * 1.5f);
    }

    // Draw ribbon as triangle strip with gradient
    const size_t lSegCount = rLink.mCachedTopCurve.size();

    for (size_t i = 0; i < lSegCount - 1; ++i) {
        const float lT1 = (float)i / (float)(lSegCount - 1);
        const float lT2 = (float)(i + 1) / (float)(lSegCount - 1);

        Color lC1 = RLCharts::lerpColor(lColorStart, lColorEnd, lT1);
        Color lC2 = RLCharts::lerpColor(lColorStart, lColorEnd, lT2);

        lC1.a = static_cast<unsigned char>(static_cast<float>(lC1.a) * lAlpha);
        lC2.a = static_cast<unsigned char>(static_cast<float>(lC2.a) * lAlpha);

        const Vector2 lTop1 = rLink.mCachedTopCurve[i];
        const Vector2 lTop2 = rLink.mCachedTopCurve[i + 1];
        const Vector2 lBot1 = rLink.mCachedBottomCurve[i];
        const Vector2 lBot2 = rLink.mCachedBottomCurve[i + 1];

        // Draw two triangles to form quad
        // Triangle 1: top1, bot1, top2
        DrawTriangle(lTop1, lBot1, lTop2, lC1);
        // Triangle 2: top2, bot1, bot2
        DrawTriangle(lTop2, lBot1, lBot2, lC2);
    }
}

void RLSankey::computeLinkCurve(const LinkDyn& rLink) const {
    if (!rLink.mCacheDirty) {
        return;
    }

    const NodeDyn& rSource = mNodes[rLink.mSourceId];
    const NodeDyn& rTarget = mNodes[rLink.mTargetId];

    const float lSourceX = getNodeX(rSource.mColumn) + mStyle.mNodeWidth;
    const float lTargetX = getNodeX(rTarget.mColumn);

    // Source and target Y positions (top edge of link within node)
    const float lSourceYTop = rSource.mY + rLink.mSourceY;
    const float lTargetYTop = rTarget.mY + rLink.mTargetY;

    // Control points for cubic Bezier (S-curve) - centerline
    const float lMidX = (lSourceX + lTargetX) * 0.5f;

    // Centerline control points (we'll offset by interpolated half-thickness)
    const Vector2 lP0 = {lSourceX, lSourceYTop + rLink.mSourceThickness * 0.5f};
    const Vector2 lP1 = {lMidX, lSourceYTop + rLink.mSourceThickness * 0.5f};
    const Vector2 lP2 = {lMidX, lTargetYTop + rLink.mTargetThickness * 0.5f};
    const Vector2 lP3 = {lTargetX, lTargetYTop + rLink.mTargetThickness * 0.5f};

    // Generate curve points with interpolated thickness
    const int lSegments = mStyle.mLinkSegments;
    rLink.mCachedTopCurve.resize(lSegments + 1);
    rLink.mCachedBottomCurve.resize(lSegments + 1);

    for (int i = 0; i <= lSegments; ++i) {
        const float lT = (float)i / (float)lSegments;

        // Get centerline point
        const Vector2 lCenter = cubicBezier(lP0, lP1, lP2, lP3, lT);

        // Interpolate thickness along the curve
        const float lThickness = rLink.mSourceThickness + (rLink.mTargetThickness - rLink.mSourceThickness) * lT;
        const float lHalfThickness = lThickness * 0.5f;

        // Offset top and bottom from the centerline
        rLink.mCachedTopCurve[i] = {lCenter.x, lCenter.y - lHalfThickness};
        rLink.mCachedBottomCurve[i] = {lCenter.x, lCenter.y + lHalfThickness};
    }

    rLink.mCacheDirty = false;
}

Vector2 RLSankey::cubicBezier(Vector2 aP0, Vector2 aP1, Vector2 aP2, Vector2 aP3, float aT) const {
    const float lU = 1.0f - aT;
    const float lU2 = lU * lU;
    const float lU3 = lU2 * lU;
    const float lT2 = aT * aT;
    const float lT3 = lT2 * aT;

    Vector2 lResult;
    lResult.x = lU3 * aP0.x + 3.0f * lU2 * aT * aP1.x + 3.0f * lU * lT2 * aP2.x + lT3 * aP3.x;
    lResult.y = lU3 * aP0.y + 3.0f * lU2 * aT * aP1.y + 3.0f * lU * lT2 * aP2.y + lT3 * aP3.y;
    return lResult;
}

float RLSankey::getNodeX(int aColumn) const {
    if (mColumnCount <= 1) {
        return mChartLeft + mChartWidth * 0.5f - mStyle.mNodeWidth * 0.5f;
    }

    const float lSpacing = (mChartWidth - mStyle.mNodeWidth) / (float)(mColumnCount - 1);
    return mChartLeft + lSpacing * (float)aColumn;
}

// ============================================================================
// Interaction
// ============================================================================

int RLSankey::getHoveredNode(Vector2 aMousePos) const {
    for (size_t i = 0; i < mNodes.size(); ++i) {
        const NodeDyn& rNode = mNodes[i];
        if (rNode.mPendingRemoval || rNode.mVisibility < 0.1f) {
            continue;
        }

        const float lX = getNodeX(rNode.mColumn);
        const Rectangle lRect = {lX, rNode.mY, mStyle.mNodeWidth, rNode.mHeight};

        if (CheckCollisionPointRec(aMousePos, lRect)) {
            return (int)i;
        }
    }
    return -1;
}

int RLSankey::getHoveredLink(Vector2 aMousePos) const {
    // Simple proximity check to link centerline
    for (size_t i = 0; i < mLinks.size(); ++i) {
        const LinkDyn& rLink = mLinks[i];
        if (rLink.mPendingRemoval || rLink.mVisibility < 0.1f) {
            continue;
        }
        if (rLink.mCachedTopCurve.empty()) {
            continue;
        }

        // Check if point is near the link ribbon
        for (size_t j = 0; j < rLink.mCachedTopCurve.size(); ++j) {
            const Vector2 lTop = rLink.mCachedTopCurve[j];
            const Vector2 lBot = rLink.mCachedBottomCurve[j];
            const float lCenterY = (lTop.y + lBot.y) * 0.5f;
            const float lHalfThick = (lBot.y - lTop.y) * 0.5f + 5.0f; // Add some tolerance

            const float lDx = aMousePos.x - lTop.x;
            const float lDy = aMousePos.y - lCenterY;

            if (lDx * lDx < 100.0f && lDy * lDy < lHalfThick * lHalfThick) {
                return (int)i;
            }
        }
    }
    return -1;
}

void RLSankey::setHighlightedNode(int aNodeId) {
    mHighlightedNode = aNodeId;
}

void RLSankey::setHighlightedLink(int aLinkId) {
    mHighlightedLink = aLinkId;
}

