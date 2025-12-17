// RLHeatMap3D.cpp
#include "RLHeatMap3D.h"
#include "RLCommon.h"
#include "rlgl.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cstdio>

namespace {
    const int PERFORMANCE_WARNING_THRESHOLD = 65536; // 256x256
    const int LUT_SIZE = 256;
    const float BOX_SIZE = 1.0f; // Normalized box size (scaled at draw time)
}

RLHeatMap3D::RLHeatMap3D() {
    // Default 4-color palette: blue -> cyan -> yellow -> red
    mPaletteStops.push_back(Color{0, 0, 180, 255});     // Blue
    mPaletteStops.push_back(Color{0, 220, 220, 255});   // Cyan
    mPaletteStops.push_back(Color{255, 255, 0, 255});   // Yellow
    mPaletteStops.push_back(Color{255, 50, 0, 255});    // Red
    rebuildLut();
}

RLHeatMap3D::RLHeatMap3D(int aWidth, int aHeight) : RLHeatMap3D() {
    setGridSize(aWidth, aHeight);
}

RLHeatMap3D::~RLHeatMap3D() {
    freeMesh();
    freeScatterMesh();
}

void RLHeatMap3D::setGridSize(int aWidth, int aHeight) {
    if (aWidth < 2 || aHeight < 2) {
        TraceLog(LOG_WARNING, "RLHeatMap3D: Grid size must be at least 2x2");
        return;
    }

    const int lTotalCells = aWidth * aHeight;
    if (lTotalCells > PERFORMANCE_WARNING_THRESHOLD) {
        TraceLog(LOG_WARNING, "RLHeatMap3D: Grid size %dx%d (%d cells) exceeds recommended maximum of %d cells. Performance may be degraded.",
                 aWidth, aHeight, lTotalCells, PERFORMANCE_WARNING_THRESHOLD);
    }

    if (mWidth == aWidth && mHeight == aHeight) {
        return;
    }

    mWidth = aWidth;
    mHeight = aHeight;

    const auto lSize = (size_t)mWidth * (size_t)mHeight;
    mCurrentValues.resize(lSize, 0.0f);
    mTargetValues.resize(lSize, 0.0f);

    std::fill(mCurrentValues.begin(), mCurrentValues.end(), 0.0f);
    std::fill(mTargetValues.begin(), mTargetValues.end(), 0.0f);

    // Update axis ranges to match grid
    mAxisMinX = 0.0f;
    mAxisMaxX = (float)(mWidth - 1);
    mAxisMinY = 0.0f;
    mAxisMaxY = (float)(mHeight - 1);

    freeMesh();
    freeScatterMesh();
    buildMesh();
}

void RLHeatMap3D::setValues(const float* pValues, int aCount) {
    if (pValues == nullptr || aCount <= 0) {
        return;
    }

    const int lCopyCount = aCount < (int)mTargetValues.size() ? aCount : (int)mTargetValues.size();
    std::memcpy(mTargetValues.data(), pValues, (size_t)lCopyCount * sizeof(float));

    if (mAutoRange && lCopyCount > 0) {
        float lMin = mTargetValues[0];
        float lMax = mTargetValues[0];
        for (int i = 1; i < lCopyCount; ++i) {
            lMin = std::min(lMin, mTargetValues[(size_t)i]);
            lMax = std::max(lMax, mTargetValues[(size_t)i]);
        }
        if (lMax - lMin < 1e-6f) {
            lMax = lMin + 1.0f;
        }
        mMinValue = lMin;
        mMaxValue = lMax;
        mAxisMinZ = lMin;
        mAxisMaxZ = lMax;
    }

    mMeshDirty = true;
}

void RLHeatMap3D::updatePartialValues(int aX, int aY, int aW, int aH, const float* pValues) {
    if (pValues == nullptr || aW <= 0 || aH <= 0) {
        return;
    }

    const int lX0 = aX < 0 ? 0 : aX;
    const int lY0 = aY < 0 ? 0 : aY;
    const int lX1 = (aX + aW) > mWidth ? mWidth : (aX + aW);
    const int lY1 = (aY + aH) > mHeight ? mHeight : (aY + aH);

    if (lX0 >= lX1 || lY0 >= lY1) {
        return;
    }

    for (int lY = lY0; lY < lY1; ++lY) {
        for (int lX = lX0; lX < lX1; ++lX) {
            const int lSrcX = lX - aX;
            const int lSrcY = lY - aY;
            const int lSrcIdx = lSrcY * aW + lSrcX;
            const int lDstIdx = lY * mWidth + lX;
            mTargetValues[(size_t)lDstIdx] = pValues[lSrcIdx];
        }
    }

    if (mAutoRange && !mTargetValues.empty()) {
        float lMin = mTargetValues[0];
        float lMax = mTargetValues[0];
        for (size_t i = 1; i < mTargetValues.size(); ++i) {
            lMin = std::min(lMin, mTargetValues[i]);
            lMax = std::max(lMax, mTargetValues[i]);
        }
        if (lMax - lMin < 1e-6f) {
            lMax = lMin + 1.0f;
        }
        mMinValue = lMin;
        mMaxValue = lMax;
        mAxisMinZ = lMin;
        mAxisMaxZ = lMax;
    }

    mMeshDirty = true;
}

void RLHeatMap3D::setPalette(Color aColorA, Color aColorB, Color aColorC) {
    mPaletteStops.clear();
    mPaletteStops.push_back(aColorA);
    mPaletteStops.push_back(aColorB);
    mPaletteStops.push_back(aColorC);
    mLutDirty = true;
}

void RLHeatMap3D::setPalette(Color aColorA, Color aColorB, Color aColorC, Color aColorD) {
    mPaletteStops.clear();
    mPaletteStops.push_back(aColorA);
    mPaletteStops.push_back(aColorB);
    mPaletteStops.push_back(aColorC);
    mPaletteStops.push_back(aColorD);
    mLutDirty = true;
}

void RLHeatMap3D::setValueRange(float aMinValue, float aMaxValue) {
    mAutoRange = false;
    mMinValue = aMinValue;
    mMaxValue = aMaxValue;
    mAxisMinZ = aMinValue;
    mAxisMaxZ = aMaxValue;
    if (mMaxValue - mMinValue < 1e-6f) {
        mMaxValue = mMinValue + 1.0f;
        mAxisMaxZ = mMaxValue;
    }
    mMeshDirty = true;
}

void RLHeatMap3D::setAutoRange(bool aEnabled) {
    mAutoRange = aEnabled;
    if (mAutoRange && !mTargetValues.empty()) {
        float lMin = mTargetValues[0];
        float lMax = mTargetValues[0];
        for (size_t i = 1; i < mTargetValues.size(); ++i) {
            lMin = std::min(lMin, mTargetValues[i]);
            lMax = std::max(lMax, mTargetValues[i]);
        }
        if (lMax - lMin < 1e-6f) {
            lMax = lMin + 1.0f;
        }
        mMinValue = lMin;
        mMaxValue = lMax;
        mAxisMinZ = lMin;
        mAxisMaxZ = lMax;
        mMeshDirty = true;
    }
}

void RLHeatMap3D::setAxisRangeX(float aMin, float aMax) {
    mAxisMinX = aMin;
    mAxisMaxX = aMax;
}

void RLHeatMap3D::setAxisRangeY(float aMin, float aMax) {
    mAxisMinY = aMin;
    mAxisMaxY = aMax;
}

void RLHeatMap3D::setAxisRangeZ(float aMin, float aMax) {
    mAxisMinZ = aMin;
    mAxisMaxZ = aMax;
}

void RLHeatMap3D::setAxisLabels(const char* pLabelX, const char* pLabelY, const char* pLabelZ) {
    mpLabelX = pLabelX;
    mpLabelY = pLabelY;
    mpLabelZ = pLabelZ;
}

void RLHeatMap3D::setMode(RLHeatMap3DMode aMode) {
    mStyle.mMode = aMode;
}

void RLHeatMap3D::setSmoothing(float aSpeed) {
    mStyle.mSmoothingSpeed = aSpeed > 0.0f ? aSpeed : 0.0f;
}

void RLHeatMap3D::setWireframe(bool aEnabled) {
    mStyle.mShowWireframe = aEnabled;
}

void RLHeatMap3D::setPointSize(float aSize) {
    mStyle.mPointSize = aSize > 0.0f ? aSize : 0.01f;
    mScatterMeshDirty = true;
}

void RLHeatMap3D::setStyle(const RLHeatMap3DStyle& rStyle) {
    mStyle = rStyle;
    mMeshDirty = true;
    mScatterMeshDirty = true;
}

void RLHeatMap3D::update(float aDt) {
    if (mLutDirty) {
        rebuildLut();
    }

    if (mWidth <= 0 || mHeight <= 0) {
        return;
    }

    const float lAlpha = 1.0f - expf(-mStyle.mSmoothingSpeed * aDt);
    bool lChanged = false;

    for (size_t i = 0; i < mCurrentValues.size(); ++i) {
        const float lDiff = mTargetValues[i] - mCurrentValues[i];
        if (fabsf(lDiff) > 1e-6f) {
            mCurrentValues[i] += lDiff * lAlpha;
            lChanged = true;
        }
    }

    if (mStyle.mMode == RLHeatMap3DMode::Surface) {
        if (lChanged || mMeshDirty) {
            updateMeshVertices();
            mMeshDirty = false;
        }
    } else {
        // Scatter mode - build mesh on demand if not yet created
        if (!mScatterMeshValid) {
            const_cast<RLHeatMap3D*>(this)->buildScatterMesh();
        }
        if (lChanged || mScatterMeshDirty) {
            const_cast<RLHeatMap3D*>(this)->updateScatterMeshVertices();
            mScatterMeshDirty = false;
        }
    }
}

void RLHeatMap3D::draw(Vector3 aPosition, float aScale, const Camera3D& rCamera) const {
    if (mWidth <= 0 || mHeight <= 0) {
        return;
    }

    // Draw the floor grid first (opaque)
    if (mStyle.mShowFloorGrid) {
        drawFloorGrid(aPosition, aScale);
    }

    // Draw data (surface or scatter) - this is the main content
    if (mStyle.mMode == RLHeatMap3DMode::Surface) {
        drawSurface(aPosition, aScale);
    } else {
        drawScatterPoints(aPosition, aScale);
    }

    // Draw axis box edges (lines, always visible)
    if (mStyle.mShowAxisBox) {
        drawAxisBox(aPosition, aScale, rCamera);
    }

    // Draw back walls LAST with transparency (so they don't occlude the data)
    if (mStyle.mShowAxisBox) {
        drawBackWalls(aPosition, aScale, rCamera);
    }

    // Draw labels and ticks (done in 2D after 3D, but we prepare here)
    // Note: Labels would be better drawn in 2D space after EndMode3D
}

void RLHeatMap3D::drawBackWalls(Vector3 aPosition, float aScale, const Camera3D& rCamera) const {
    const float lHalfSize = BOX_SIZE * 0.5f * aScale;
    const float lHeight = BOX_SIZE * aScale;

    // Disable depth writing and enable blending for transparent walls
    rlDisableDepthMask();
    rlEnableColorBlend();
    rlSetBlendMode(BLEND_ALPHA);

    // Calculate camera position relative to the box center
    const float lCamRelX = rCamera.position.x - aPosition.x;
    const float lCamRelZ = rCamera.position.z - aPosition.z;

    // Only draw walls that are on the OPPOSITE side from the camera
    // This makes them appear as background reference planes

    // Back wall (at Z = -halfSize) - only draw if camera is in front (Z > 0)
    if (lCamRelZ > 0) {
        Color lWallColor = mStyle.mBackWallColor;
        // Fade based on how much the camera is in front
        float lFade = lCamRelZ / (lHalfSize * 3.0f);
        lFade = std::min(lFade, 1.0f);
        lWallColor.a = (unsigned char)((float)lWallColor.a * lFade);

        const Vector3 lV1 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        const Vector3 lV2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        const Vector3 lV3 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
        const Vector3 lV4 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lWallColor);
        DrawTriangle3D(lV1, lV3, lV4, lWallColor);
    }

    // Front wall (at Z = +halfSize) - only draw if the camera is behind (Z < 0)
    if (lCamRelZ < 0) {
        Color lWallColor = mStyle.mBackWallColor;
        float lFade = -lCamRelZ / (lHalfSize * 3.0f);
        lFade = std::min(lFade, 1.0f);
        lWallColor.a = (unsigned char)((float)lWallColor.a * lFade);

        const Vector3 lV1 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        const Vector3 lV2 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        const Vector3 lV3 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};
        const Vector3 lV4 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lWallColor);
        DrawTriangle3D(lV1, lV3, lV4, lWallColor);
    }

    // Left wall (at X = -halfSize) - only draw if camera is to the right (X > 0)
    if (lCamRelX > 0) {
        Color lWallColor = mStyle.mBackWallColor;
        float lFade = lCamRelX / (lHalfSize * 3.0f);
        lFade = std::min(lFade, 1.0f);
        lWallColor.a = (unsigned char)((float)lWallColor.a * lFade);

        const Vector3 lV1 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        const Vector3 lV2 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        const Vector3 lV3 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};
        const Vector3 lV4 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lWallColor);
        DrawTriangle3D(lV1, lV3, lV4, lWallColor);
    }

    // Right wall (at X = +halfSize) - only draw if the camera is to the left (X < 0)
    if (lCamRelX < 0) {
        Color lWallColor = mStyle.mBackWallColor;
        float lFade = -lCamRelX / (lHalfSize * 3.0f);
        lFade = std::min(lFade, 1.0f);
        lWallColor.a = (unsigned char)((float)lWallColor.a * lFade);

        const Vector3 lV1 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        const Vector3 lV2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        const Vector3 lV3 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
        const Vector3 lV4 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lWallColor);
        DrawTriangle3D(lV1, lV3, lV4, lWallColor);
    }

    // Re-enable depth writing
    rlEnableDepthMask();
}

void RLHeatMap3D::drawFloorGrid(Vector3 aPosition, float aScale) const {
    const float lHalfSize = BOX_SIZE * 0.5f * aScale;
    const int lDivisions = mStyle.mGridDivisions;
    const float lStep = (lHalfSize * 2.0f) / (float)lDivisions;

    // Draw grid lines on floor (Y = 0 plane)
    for (int i = 0; i <= lDivisions; ++i) {
        const float lOffset = -lHalfSize + (float)i * lStep;

        // Lines parallel to Z axis
        const Vector3 lStart1 = {aPosition.x + lOffset, aPosition.y, aPosition.z - lHalfSize};
        const Vector3 lEnd1 = {aPosition.x + lOffset, aPosition.y, aPosition.z + lHalfSize};
        DrawLine3D(lStart1, lEnd1, mStyle.mFloorGridColor);

        // Lines parallel to X axis
        const Vector3 lStart2 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lOffset};
        const Vector3 lEnd2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lOffset};
        DrawLine3D(lStart2, lEnd2, mStyle.mFloorGridColor);
    }
}

void RLHeatMap3D::drawAxisBox(Vector3 aPosition, float aScale, const Camera3D& rCamera) const {
    (void)rCamera; // May be used for adaptive rendering

    const float lHalfSize = BOX_SIZE * 0.5f * aScale;
    const float lHeight = BOX_SIZE * aScale;

    // Bottom rectangle (floor edges)
    const Vector3 lB1 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z - lHalfSize};
    const Vector3 lB2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z - lHalfSize};
    const Vector3 lB3 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lHalfSize};
    const Vector3 lB4 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lHalfSize};

    // Top rectangle
    const Vector3 lT1 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
    const Vector3 lT2 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
    const Vector3 lT3 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};
    const Vector3 lT4 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};

    // Draw bottom edges
    DrawLine3D(lB1, lB2, mStyle.mAxisColor);
    DrawLine3D(lB2, lB3, mStyle.mAxisColor);
    DrawLine3D(lB3, lB4, mStyle.mAxisColor);
    DrawLine3D(lB4, lB1, mStyle.mAxisColor);

    // Draw top edges
    DrawLine3D(lT1, lT2, mStyle.mAxisColor);
    DrawLine3D(lT2, lT3, mStyle.mAxisColor);
    DrawLine3D(lT3, lT4, mStyle.mAxisColor);
    DrawLine3D(lT4, lT1, mStyle.mAxisColor);

    // Draw vertical edges
    DrawLine3D(lB1, lT1, mStyle.mAxisColor);
    DrawLine3D(lB2, lT2, mStyle.mAxisColor);
    DrawLine3D(lB3, lT3, mStyle.mAxisColor);
    DrawLine3D(lB4, lT4, mStyle.mAxisColor);

    // Draw tick marks on axes
    if (mStyle.mShowTicks) {
        const int lTickCount = mStyle.mTickCount;
        const float lTickLen = 0.02f * aScale;

        // Z-axis ticks (on the back-left vertical edge)
        for (int i = 0; i <= lTickCount; ++i) {
            const float lT = (float)i / (float)lTickCount;
            const float lY = aPosition.y + lT * lHeight;
            const Vector3 lTickStart = {lB1.x, lY, lB1.z};
            const Vector3 lTickEnd = {lB1.x - lTickLen, lY, lB1.z - lTickLen};
            DrawLine3D(lTickStart, lTickEnd, mStyle.mTickColor);
        }

        // X-axis ticks (on the front-bottom edge)
        for (int i = 0; i <= lTickCount; ++i) {
            const float lT = (float)i / (float)lTickCount;
            const float lX = aPosition.x - lHalfSize + lT * lHalfSize * 2.0f;
            const Vector3 lTickStart = {lX, aPosition.y, lB3.z};
            const Vector3 lTickEnd = {lX, aPosition.y - lTickLen, lB3.z + lTickLen};
            DrawLine3D(lTickStart, lTickEnd, mStyle.mTickColor);
        }

        // Y-axis ticks (on the right-bottom edge)
        for (int i = 0; i <= lTickCount; ++i) {
            const float lT = (float)i / (float)lTickCount;
            const float lZ = aPosition.z - lHalfSize + lT * lHalfSize * 2.0f;
            const Vector3 lTickStart = {lB2.x, aPosition.y, lZ};
            const Vector3 lTickEnd = {lB2.x + lTickLen, aPosition.y - lTickLen, lZ};
            DrawLine3D(lTickStart, lTickEnd, mStyle.mTickColor);
        }
    }
}

void RLHeatMap3D::drawSurface(Vector3 aPosition, float aScale) const {
    if (!mMeshValid) {
        return;
    }

    // Disable backface culling so the surface is visible from all angles
    rlDisableBackfaceCulling();

    DrawModelEx(mModel, aPosition, Vector3{0, 1, 0}, 0.0f,
                Vector3{aScale, aScale, aScale}, WHITE);

    if (mStyle.mShowWireframe) {
        DrawModelWiresEx(mModel, aPosition, Vector3{0, 1, 0}, 0.0f,
                         Vector3{aScale, aScale, aScale}, mStyle.mWireframeColor);
    }

    // Re-enable backface culling for other rendering
    rlEnableBackfaceCulling();
}

void RLHeatMap3D::drawScatterPoints(Vector3 aPosition, float aScale) const {
    if (!mScatterMeshValid) {
        return;
    }


    // Disable backface culling for the scatter cubes
    rlDisableBackfaceCulling();

    DrawModelEx(mScatterModel, aPosition, Vector3{0, 1, 0}, 0.0f,
                Vector3{aScale, aScale, aScale}, WHITE);

    rlEnableBackfaceCulling();
}

void RLHeatMap3D::drawAxisLabelsAndTicks(Vector3 aPosition, float aScale, const Camera3D& rCamera) const {
    (void)aPosition;
    (void)aScale;
    (void)rCamera;
    // Labels are best drawn in 2D after EndMode3D
    // This is a placeholder for future implementation
}

float RLHeatMap3D::calculateWallAlpha(Vector3 aWallNormal, const Camera3D& rCamera) const {
    // Calculate view direction
    Vector3 lViewDir = {
        rCamera.target.x - rCamera.position.x,
        rCamera.target.y - rCamera.position.y,
        rCamera.target.z - rCamera.position.z
    };

    // Normalize
    const float lLen = sqrtf(lViewDir.x * lViewDir.x + lViewDir.y * lViewDir.y + lViewDir.z * lViewDir.z);
    if (lLen > 0.0f) {
        lViewDir.x /= lLen;
        lViewDir.y /= lLen;
        lViewDir.z /= lLen;
    }

    // Dot product: wall is visible when the camera looks at it (dot > 0)
    const float lDot = aWallNormal.x * lViewDir.x + aWallNormal.y * lViewDir.y + aWallNormal.z * lViewDir.z;

    // Fade wall as it becomes more perpendicular to view
    // Wall fully visible when dot approaches 1, invisible when dot <= 0
    if (lDot <= 0.0f) {
        return 0.0f;
    }

    // Smooth fade
    return lDot * lDot; // Quadratic falloff for smoother transition
}

void RLHeatMap3D::rebuildLut() {
    if (mPaletteStops.size() < 2) {
        for (int i = 0; i < LUT_SIZE; ++i) {
            const auto lVal = (unsigned char)i;
            mLut[i] = Color{lVal, lVal, lVal, 255};
        }
        mLutDirty = false;
        return;
    }

    const int lNumStops = (int)mPaletteStops.size();
    for (int i = 0; i < LUT_SIZE; ++i) {
        const float lT = (float)i / (float)(LUT_SIZE - 1);
        const float lScaled = lT * (float)(lNumStops - 1);
        int lIdx0 = (int)lScaled;
        int lIdx1 = lIdx0 + 1;
        if (lIdx1 >= lNumStops) {
            lIdx1 = lNumStops - 1;
            lIdx0 = lIdx1 - 1;
        }
        lIdx0 = std::max(lIdx0, 0);

        const float lLocalT = lScaled - (float)lIdx0;
        mLut[i] = RLCharts::lerpColor(mPaletteStops[(size_t)lIdx0], mPaletteStops[(size_t)lIdx1], lLocalT);
    }

    mLutDirty = false;
}

void RLHeatMap3D::buildMesh() {
    if (mWidth < 2 || mHeight < 2) {
        return;
    }

    freeMesh();

    const int lCellsX = mWidth - 1;
    const int lCellsY = mHeight - 1;
    const int lTriangleCount = lCellsX * lCellsY * 2;
    const int lVertexCount = lTriangleCount * 3;

    mMesh.triangleCount = lTriangleCount;
    mMesh.vertexCount = lVertexCount;

    mMesh.vertices = (float*)MemAlloc((size_t)lVertexCount * 3 * sizeof(float));
    mMesh.normals = (float*)MemAlloc((size_t)lVertexCount * 3 * sizeof(float));
    mMesh.colors = (unsigned char*)MemAlloc((size_t)lVertexCount * 4 * sizeof(unsigned char));

    // Initialize with current values
    updateMeshVertices();

    // Calculate normals
    for (int i = 0; i < lVertexCount; ++i) {
        mMesh.normals[(size_t)i * 3 + 0] = 0.0f;
        mMesh.normals[(size_t)i * 3 + 1] = 1.0f;
        mMesh.normals[(size_t)i * 3 + 2] = 0.0f;
    }

    UploadMesh(&mMesh, true);
    mModel = LoadModelFromMesh(mMesh);

    mMeshValid = true;
    mMeshDirty = false;
}

void RLHeatMap3D::updateMeshVertices() {
    if (mWidth < 2 || mHeight < 2) {
        return;
    }

    // If mesh not built yet, just mark dirty
    if (!mMeshValid && mMesh.vertices == nullptr) {
        return;
    }

    const int lCellsX = mWidth - 1;
    const int lCellsY = mHeight - 1;
    const float lHalfSize = BOX_SIZE * 0.5f;
    const float lHeight = BOX_SIZE;

    int lVertIdx = 0;
    for (int lCy = 0; lCy < lCellsY; ++lCy) {
        for (int lCx = 0; lCx < lCellsX; ++lCx) {
            // Grid positions mapped to [-halfSize, +halfSize]
            const float lX0 = -lHalfSize + ((float)lCx / (float)lCellsX) * lHalfSize * 2.0f;
            const float lX1 = -lHalfSize + ((float)(lCx + 1) / (float)lCellsX) * lHalfSize * 2.0f;
            const float lZ0 = -lHalfSize + ((float)lCy / (float)lCellsY) * lHalfSize * 2.0f;
            const float lZ1 = -lHalfSize + ((float)(lCy + 1) / (float)lCellsY) * lHalfSize * 2.0f;

            const int lIdx00 = lCy * mWidth + lCx;
            const int lIdx10 = lCy * mWidth + (lCx + 1);
            const int lIdx01 = (lCy + 1) * mWidth + lCx;
            const int lIdx11 = (lCy + 1) * mWidth + (lCx + 1);

            const float lN00 = normalizeValue(mCurrentValues[(size_t)lIdx00]);
            const float lN10 = normalizeValue(mCurrentValues[(size_t)lIdx10]);
            const float lN01 = normalizeValue(mCurrentValues[(size_t)lIdx01]);
            const float lN11 = normalizeValue(mCurrentValues[(size_t)lIdx11]);

            const float lH00 = lN00 * lHeight;
            const float lH10 = lN10 * lHeight;
            const float lH01 = lN01 * lHeight;
            const float lH11 = lN11 * lHeight;

            Color lC00 = getColorForValue(lN00);
            Color lC10 = getColorForValue(lN10);
            Color lC01 = getColorForValue(lN01);
            Color lC11 = getColorForValue(lN11);

            // Apply surface opacity
            lC00.a = (unsigned char)(mStyle.mSurfaceOpacity * 255.0f);
            lC10.a = (unsigned char)(mStyle.mSurfaceOpacity * 255.0f);
            lC01.a = (unsigned char)(mStyle.mSurfaceOpacity * 255.0f);
            lC11.a = (unsigned char)(mStyle.mSurfaceOpacity * 255.0f);

            // Triangle 1: (0,0), (1,0), (0,1)
            mMesh.vertices[(size_t)lVertIdx * 3 + 0] = lX0;
            mMesh.vertices[(size_t)lVertIdx * 3 + 1] = lH00;
            mMesh.vertices[(size_t)lVertIdx * 3 + 2] = lZ0;
            mMesh.colors[(size_t)lVertIdx * 4 + 0] = lC00.r;
            mMesh.colors[(size_t)lVertIdx * 4 + 1] = lC00.g;
            mMesh.colors[(size_t)lVertIdx * 4 + 2] = lC00.b;
            mMesh.colors[(size_t)lVertIdx * 4 + 3] = lC00.a;
            lVertIdx++;

            mMesh.vertices[(size_t)lVertIdx * 3 + 0] = lX1;
            mMesh.vertices[(size_t)lVertIdx * 3 + 1] = lH10;
            mMesh.vertices[(size_t)lVertIdx * 3 + 2] = lZ0;
            mMesh.colors[(size_t)lVertIdx * 4 + 0] = lC10.r;
            mMesh.colors[(size_t)lVertIdx * 4 + 1] = lC10.g;
            mMesh.colors[(size_t)lVertIdx * 4 + 2] = lC10.b;
            mMesh.colors[(size_t)lVertIdx * 4 + 3] = lC10.a;
            lVertIdx++;

            mMesh.vertices[(size_t)lVertIdx * 3 + 0] = lX0;
            mMesh.vertices[(size_t)lVertIdx * 3 + 1] = lH01;
            mMesh.vertices[(size_t)lVertIdx * 3 + 2] = lZ1;
            mMesh.colors[(size_t)lVertIdx * 4 + 0] = lC01.r;
            mMesh.colors[(size_t)lVertIdx * 4 + 1] = lC01.g;
            mMesh.colors[(size_t)lVertIdx * 4 + 2] = lC01.b;
            mMesh.colors[(size_t)lVertIdx * 4 + 3] = lC01.a;
            lVertIdx++;

            // Triangle 2: (1,0), (1,1), (0,1)
            mMesh.vertices[(size_t)lVertIdx * 3 + 0] = lX1;
            mMesh.vertices[(size_t)lVertIdx * 3 + 1] = lH10;
            mMesh.vertices[(size_t)lVertIdx * 3 + 2] = lZ0;
            mMesh.colors[(size_t)lVertIdx * 4 + 0] = lC10.r;
            mMesh.colors[(size_t)lVertIdx * 4 + 1] = lC10.g;
            mMesh.colors[(size_t)lVertIdx * 4 + 2] = lC10.b;
            mMesh.colors[(size_t)lVertIdx * 4 + 3] = lC10.a;
            lVertIdx++;

            mMesh.vertices[(size_t)lVertIdx * 3 + 0] = lX1;
            mMesh.vertices[(size_t)lVertIdx * 3 + 1] = lH11;
            mMesh.vertices[(size_t)lVertIdx * 3 + 2] = lZ1;
            mMesh.colors[(size_t)lVertIdx * 4 + 0] = lC11.r;
            mMesh.colors[(size_t)lVertIdx * 4 + 1] = lC11.g;
            mMesh.colors[(size_t)lVertIdx * 4 + 2] = lC11.b;
            mMesh.colors[(size_t)lVertIdx * 4 + 3] = lC11.a;
            lVertIdx++;

            mMesh.vertices[(size_t)lVertIdx * 3 + 0] = lX0;
            mMesh.vertices[(size_t)lVertIdx * 3 + 1] = lH01;
            mMesh.vertices[(size_t)lVertIdx * 3 + 2] = lZ1;
            mMesh.colors[(size_t)lVertIdx * 4 + 0] = lC01.r;
            mMesh.colors[(size_t)lVertIdx * 4 + 1] = lC01.g;
            mMesh.colors[(size_t)lVertIdx * 4 + 2] = lC01.b;
            mMesh.colors[(size_t)lVertIdx * 4 + 3] = lC01.a;
            lVertIdx++;
        }
    }

    // Update GPU buffers if mesh is valid
    if (mMeshValid) {
        UpdateMeshBuffer(mMesh, 0, mMesh.vertices, mMesh.vertexCount * 3 * (int)sizeof(float), 0);
        UpdateMeshBuffer(mMesh, 3, mMesh.colors, mMesh.vertexCount * 4 * (int)sizeof(unsigned char), 0);
    }
}

void RLHeatMap3D::freeMesh() {
    if (mMeshValid) {
        UnloadModel(mModel);
        mMeshValid = false;
    }
    mMesh = Mesh{};
    mModel = Model{};
}

void RLHeatMap3D::buildScatterMesh() {
    if (mWidth < 2 || mHeight < 2) {
        return;
    }

    freeScatterMesh();

    const int lPointCount = mWidth * mHeight;
    const int lTrianglesPerPoint = 12; // 6 faces x 2 triangles per cube
    const int lTriangleCount = lPointCount * lTrianglesPerPoint;
    const int lVertexCount = lTriangleCount * 3;

    mScatterMesh.triangleCount = lTriangleCount;
    mScatterMesh.vertexCount = lVertexCount;

    mScatterMesh.vertices = (float*)MemAlloc((size_t)lVertexCount * 3 * sizeof(float));
    mScatterMesh.normals = (float*)MemAlloc((size_t)lVertexCount * 3 * sizeof(float));
    mScatterMesh.colors = (unsigned char*)MemAlloc((size_t)lVertexCount * 4 * sizeof(unsigned char));

    // Initialize normals (simple per-face normals pointing outward)
    for (int i = 0; i < lVertexCount; ++i) {
        mScatterMesh.normals[(size_t)i * 3 + 0] = 0.0f;
        mScatterMesh.normals[(size_t)i * 3 + 1] = 1.0f;
        mScatterMesh.normals[(size_t)i * 3 + 2] = 0.0f;
    }

    // Initialize with current values
    updateScatterMeshVertices();

    UploadMesh(&mScatterMesh, true); // dynamic = true for frequent updates
    mScatterModel = LoadModelFromMesh(mScatterMesh);

    mScatterMeshValid = true;
    mScatterMeshDirty = false;
}

void RLHeatMap3D::updateScatterMeshVertices() {
    if (mWidth < 2 || mHeight < 2) {
        return;
    }

    if (!mScatterMeshValid && mScatterMesh.vertices == nullptr) {
        return;
    }

    const float lHalfSize = BOX_SIZE * 0.5f;
    const float lHeight = BOX_SIZE;
    const float lS = mStyle.mPointSize * 0.5f; // Half-size of cube

    int lVertIdx = 0;
    for (int lY = 0; lY < mHeight; ++lY) {
        for (int lX = 0; lX < mWidth; ++lX) {
            const int lIdx = lY * mWidth + lX;
            const float lValue = mCurrentValues[(size_t)lIdx];
            const float lNorm = normalizeValue(lValue);

            // Map grid position to box coordinates
            const float lPx = -lHalfSize + ((float)lX / (float)(mWidth - 1)) * lHalfSize * 2.0f;
            const float lPz = -lHalfSize + ((float)lY / (float)(mHeight - 1)) * lHalfSize * 2.0f;
            const float lPy = lNorm * lHeight;

            const Color lC = getColorForValue(lNorm);

            // 36 vertices per cube (12 triangles x 3 vertices)
            // Each face has 2 triangles = 6 vertices

            // Front face (+Z) - 6 vertices
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            // Back face (-Z) - 6 vertices
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            // Top face (+Y) - 6 vertices
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            // Bottom face (-Y) - 6 vertices
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            // Right face (+X) - 6 vertices
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            // Left face (-X) - 6 vertices
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz + lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;

            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 0] = lPx - lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 1] = lPy + lS;
            mScatterMesh.vertices[(size_t)lVertIdx * 3 + 2] = lPz - lS;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 0] = lC.r;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 1] = lC.g;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 2] = lC.b;
            mScatterMesh.colors[(size_t)lVertIdx * 4 + 3] = lC.a;
            lVertIdx++;
        }
    }

    // Update GPU buffers if mesh is already uploaded
    if (mScatterMeshValid) {
        UpdateMeshBuffer(mScatterMesh, 0, mScatterMesh.vertices, mScatterMesh.vertexCount * 3 * (int)sizeof(float), 0);
        UpdateMeshBuffer(mScatterMesh, 3, mScatterMesh.colors, mScatterMesh.vertexCount * 4 * (int)sizeof(unsigned char), 0);
    }
}

void RLHeatMap3D::freeScatterMesh() {
    if (mScatterMeshValid) {
        UnloadModel(mScatterModel);
        mScatterMeshValid = false;
    }
    mScatterMesh = Mesh{};
    mScatterModel = Model{};
}

float RLHeatMap3D::normalizeValue(float aValue) const {
    const float lRange = mMaxValue - mMinValue;
    if (lRange < 1e-6f) {
        return 0.5f;
    }
    const float lNorm = (aValue - mMinValue) / lRange;
    return RLCharts::clamp01(lNorm);
}

Color RLHeatMap3D::getColorForValue(float aNormalizedValue) const {
    const float lClamped = RLCharts::clamp01(aNormalizedValue);
    int lIdx = (int)(lClamped * 255.0f);
    lIdx = std::max(lIdx, 0);
    lIdx = std::min(lIdx, 255);
    return mLut[lIdx];
}
