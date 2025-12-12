// RLHeatMap3D.cpp
#include "RLHeatMap3D.h"
#include "RLCommon.h"
#include "rlgl.h"
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
}

void RLHeatMap3D::setGridSize(int aWidth, int aHeight) {
    if (aWidth < 2 || aHeight < 2) {
        TraceLog(LOG_WARNING, "RLHeatMap3D: Grid size must be at least 2x2");
        return;
    }

    int lTotalCells = aWidth * aHeight;
    if (lTotalCells > PERFORMANCE_WARNING_THRESHOLD) {
        TraceLog(LOG_WARNING, "RLHeatMap3D: Grid size %dx%d (%d cells) exceeds recommended maximum of %d cells. Performance may be degraded.",
                 aWidth, aHeight, lTotalCells, PERFORMANCE_WARNING_THRESHOLD);
    }

    if (mWidth == aWidth && mHeight == aHeight) {
        return;
    }

    mWidth = aWidth;
    mHeight = aHeight;

    size_t lSize = (size_t)(mWidth * mHeight);
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
    buildMesh();
}

void RLHeatMap3D::setValues(const float* pValues, int aCount) {
    if (pValues == nullptr || aCount <= 0) {
        return;
    }

    int lCopyCount = aCount < (int)mTargetValues.size() ? aCount : (int)mTargetValues.size();
    std::memcpy(mTargetValues.data(), pValues, (size_t)lCopyCount * sizeof(float));

    if (mAutoRange && lCopyCount > 0) {
        float lMin = mTargetValues[0];
        float lMax = mTargetValues[0];
        for (int i = 1; i < lCopyCount; ++i) {
            if (mTargetValues[(size_t)i] < lMin) lMin = mTargetValues[(size_t)i];
            if (mTargetValues[(size_t)i] > lMax) lMax = mTargetValues[(size_t)i];
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

    int lX0 = aX < 0 ? 0 : aX;
    int lY0 = aY < 0 ? 0 : aY;
    int lX1 = (aX + aW) > mWidth ? mWidth : (aX + aW);
    int lY1 = (aY + aH) > mHeight ? mHeight : (aY + aH);

    if (lX0 >= lX1 || lY0 >= lY1) {
        return;
    }

    for (int lY = lY0; lY < lY1; ++lY) {
        for (int lX = lX0; lX < lX1; ++lX) {
            int lSrcX = lX - aX;
            int lSrcY = lY - aY;
            int lSrcIdx = lSrcY * aW + lSrcX;
            int lDstIdx = lY * mWidth + lX;
            mTargetValues[(size_t)lDstIdx] = pValues[lSrcIdx];
        }
    }

    if (mAutoRange && !mTargetValues.empty()) {
        float lMin = mTargetValues[0];
        float lMax = mTargetValues[0];
        for (size_t i = 1; i < mTargetValues.size(); ++i) {
            if (mTargetValues[i] < lMin) lMin = mTargetValues[i];
            if (mTargetValues[i] > lMax) lMax = mTargetValues[i];
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
            if (mTargetValues[i] < lMin) lMin = mTargetValues[i];
            if (mTargetValues[i] > lMax) lMax = mTargetValues[i];
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
}

void RLHeatMap3D::setStyle(const RLHeatMap3DStyle& rStyle) {
    mStyle = rStyle;
    mMeshDirty = true;
}

void RLHeatMap3D::update(float aDt) {
    if (mLutDirty) {
        rebuildLut();
    }

    if (mWidth <= 0 || mHeight <= 0) {
        return;
    }

    float lAlpha = 1.0f - expf(-mStyle.mSmoothingSpeed * aDt);
    bool lChanged = false;

    for (size_t i = 0; i < mCurrentValues.size(); ++i) {
        float lDiff = mTargetValues[i] - mCurrentValues[i];
        if (fabsf(lDiff) > 1e-6f) {
            mCurrentValues[i] += lDiff * lAlpha;
            lChanged = true;
        }
    }

    if ((lChanged || mMeshDirty) && mStyle.mMode == RLHeatMap3DMode::Surface) {
        updateMeshVertices();
        mMeshDirty = false;
    }
}

void RLHeatMap3D::draw(Vector3 aPosition, float aScale, const Camera3D& rCamera) const {
    if (mWidth <= 0 || mHeight <= 0) {
        return;
    }

    // Draw back walls first (with transparency based on view angle)
    if (mStyle.mShowAxisBox) {
        drawBackWalls(aPosition, aScale, rCamera);
    }

    // Draw floor grid
    if (mStyle.mShowFloorGrid) {
        drawFloorGrid(aPosition, aScale);
    }

    // Draw data (surface or scatter)
    if (mStyle.mMode == RLHeatMap3DMode::Surface) {
        drawSurface(aPosition, aScale);
    } else {
        drawScatterPoints(aPosition, aScale);
    }

    // Draw axis box edges
    if (mStyle.mShowAxisBox) {
        drawAxisBox(aPosition, aScale, rCamera);
    }

    // Draw labels and ticks (done in 2D after 3D, but we prepare here)
    // Note: Labels would be better drawn in 2D space after EndMode3D
}

void RLHeatMap3D::drawBackWalls(Vector3 aPosition, float aScale, const Camera3D& rCamera) const {
    float lHalfSize = BOX_SIZE * 0.5f * aScale;

    // Back wall (XZ plane at Y = -halfSize)
    Vector3 lBackNormal = {0.0f, 1.0f, 0.0f};
    float lBackAlpha = calculateWallAlpha(lBackNormal, rCamera);
    if (lBackAlpha > 0.01f) {
        Color lBackColor = mStyle.mBackWallColor;
        lBackColor.a = (unsigned char)(lBackColor.a * lBackAlpha);

        Vector3 lV1 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        Vector3 lV2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        Vector3 lV3 = {aPosition.x + lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z - lHalfSize};
        Vector3 lV4 = {aPosition.x - lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z - lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lBackColor);
        DrawTriangle3D(lV1, lV3, lV4, lBackColor);
    }

    // Left wall (YZ plane at X = -halfSize)
    Vector3 lLeftNormal = {1.0f, 0.0f, 0.0f};
    float lLeftAlpha = calculateWallAlpha(lLeftNormal, rCamera);
    if (lLeftAlpha > 0.01f) {
        Color lLeftColor = mStyle.mBackWallColor;
        lLeftColor.a = (unsigned char)(lLeftColor.a * lLeftAlpha);

        Vector3 lV1 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        Vector3 lV2 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        Vector3 lV3 = {aPosition.x - lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z + lHalfSize};
        Vector3 lV4 = {aPosition.x - lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z - lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lLeftColor);
        DrawTriangle3D(lV1, lV3, lV4, lLeftColor);
    }

    // Right wall (YZ plane at X = +halfSize)
    Vector3 lRightNormal = {-1.0f, 0.0f, 0.0f};
    float lRightAlpha = calculateWallAlpha(lRightNormal, rCamera);
    if (lRightAlpha > 0.01f) {
        Color lRightColor = mStyle.mBackWallColor;
        lRightColor.a = (unsigned char)(lRightColor.a * lRightAlpha);

        Vector3 lV1 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        Vector3 lV2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z - lHalfSize};
        Vector3 lV3 = {aPosition.x + lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z - lHalfSize};
        Vector3 lV4 = {aPosition.x + lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z + lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lRightColor);
        DrawTriangle3D(lV1, lV3, lV4, lRightColor);
    }

    // Front wall (XZ plane at Y = +halfSize) - usually not drawn as it's facing viewer
    Vector3 lFrontNormal = {0.0f, -1.0f, 0.0f};
    float lFrontAlpha = calculateWallAlpha(lFrontNormal, rCamera);
    if (lFrontAlpha > 0.01f) {
        Color lFrontColor = mStyle.mBackWallColor;
        lFrontColor.a = (unsigned char)(lFrontColor.a * lFrontAlpha * 0.3f); // Even more transparent

        Vector3 lV1 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        Vector3 lV2 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lHalfSize};
        Vector3 lV3 = {aPosition.x - lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z + lHalfSize};
        Vector3 lV4 = {aPosition.x + lHalfSize, aPosition.y + lHalfSize * 2.0f, aPosition.z + lHalfSize};
        DrawTriangle3D(lV1, lV2, lV3, lFrontColor);
        DrawTriangle3D(lV1, lV3, lV4, lFrontColor);
    }
}

void RLHeatMap3D::drawFloorGrid(Vector3 aPosition, float aScale) const {
    float lHalfSize = BOX_SIZE * 0.5f * aScale;
    int lDivisions = mStyle.mGridDivisions;
    float lStep = (lHalfSize * 2.0f) / (float)lDivisions;

    // Draw grid lines on floor (Y = 0 plane)
    for (int i = 0; i <= lDivisions; ++i) {
        float lOffset = -lHalfSize + (float)i * lStep;

        // Lines parallel to Z axis
        Vector3 lStart1 = {aPosition.x + lOffset, aPosition.y, aPosition.z - lHalfSize};
        Vector3 lEnd1 = {aPosition.x + lOffset, aPosition.y, aPosition.z + lHalfSize};
        DrawLine3D(lStart1, lEnd1, mStyle.mFloorGridColor);

        // Lines parallel to X axis
        Vector3 lStart2 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lOffset};
        Vector3 lEnd2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lOffset};
        DrawLine3D(lStart2, lEnd2, mStyle.mFloorGridColor);
    }
}

void RLHeatMap3D::drawAxisBox(Vector3 aPosition, float aScale, const Camera3D& rCamera) const {
    (void)rCamera; // May be used for adaptive rendering

    float lHalfSize = BOX_SIZE * 0.5f * aScale;
    float lHeight = BOX_SIZE * aScale;

    // Bottom rectangle (floor edges)
    Vector3 lB1 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z - lHalfSize};
    Vector3 lB2 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z - lHalfSize};
    Vector3 lB3 = {aPosition.x + lHalfSize, aPosition.y, aPosition.z + lHalfSize};
    Vector3 lB4 = {aPosition.x - lHalfSize, aPosition.y, aPosition.z + lHalfSize};

    // Top rectangle
    Vector3 lT1 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
    Vector3 lT2 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z - lHalfSize};
    Vector3 lT3 = {aPosition.x + lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};
    Vector3 lT4 = {aPosition.x - lHalfSize, aPosition.y + lHeight, aPosition.z + lHalfSize};

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
        int lTickCount = mStyle.mTickCount;
        float lTickLen = 0.02f * aScale;

        // Z-axis ticks (on the back-left vertical edge)
        for (int i = 0; i <= lTickCount; ++i) {
            float lT = (float)i / (float)lTickCount;
            float lY = aPosition.y + lT * lHeight;
            Vector3 lTickStart = {lB1.x, lY, lB1.z};
            Vector3 lTickEnd = {lB1.x - lTickLen, lY, lB1.z - lTickLen};
            DrawLine3D(lTickStart, lTickEnd, mStyle.mTickColor);
        }

        // X-axis ticks (on the front-bottom edge)
        for (int i = 0; i <= lTickCount; ++i) {
            float lT = (float)i / (float)lTickCount;
            float lX = aPosition.x - lHalfSize + lT * lHalfSize * 2.0f;
            Vector3 lTickStart = {lX, aPosition.y, lB3.z};
            Vector3 lTickEnd = {lX, aPosition.y - lTickLen, lB3.z + lTickLen};
            DrawLine3D(lTickStart, lTickEnd, mStyle.mTickColor);
        }

        // Y-axis ticks (on the right-bottom edge)
        for (int i = 0; i <= lTickCount; ++i) {
            float lT = (float)i / (float)lTickCount;
            float lZ = aPosition.z - lHalfSize + lT * lHalfSize * 2.0f;
            Vector3 lTickStart = {lB2.x, aPosition.y, lZ};
            Vector3 lTickEnd = {lB2.x + lTickLen, aPosition.y - lTickLen, lZ};
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
    float lHalfSize = BOX_SIZE * 0.5f;
    float lHeight = BOX_SIZE;

    for (int lY = 0; lY < mHeight; ++lY) {
        for (int lX = 0; lX < mWidth; ++lX) {
            int lIdx = lY * mWidth + lX;
            float lValue = mCurrentValues[(size_t)lIdx];
            float lNorm = normalizeValue(lValue);

            // Map grid position to box coordinates
            float lPx = -lHalfSize + ((float)lX / (float)(mWidth - 1)) * lHalfSize * 2.0f;
            float lPz = -lHalfSize + ((float)lY / (float)(mHeight - 1)) * lHalfSize * 2.0f;
            float lPy = lNorm * lHeight;

            Vector3 lPos = {
                aPosition.x + lPx * aScale,
                aPosition.y + lPy * aScale,
                aPosition.z + lPz * aScale
            };

            Color lColor = getColorForValue(lNorm);
            float lRadius = mStyle.mPointSize * aScale;

            DrawSphere(lPos, lRadius, lColor);

            if (mStyle.mShowPointOutline) {
                DrawSphereWires(lPos, lRadius * 1.05f, 4, 4, mStyle.mWireframeColor);
            }
        }
    }
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
    float lLen = sqrtf(lViewDir.x * lViewDir.x + lViewDir.y * lViewDir.y + lViewDir.z * lViewDir.z);
    if (lLen > 0.0f) {
        lViewDir.x /= lLen;
        lViewDir.y /= lLen;
        lViewDir.z /= lLen;
    }

    // Dot product: wall is visible when camera looks at it (dot > 0)
    float lDot = aWallNormal.x * lViewDir.x + aWallNormal.y * lViewDir.y + aWallNormal.z * lViewDir.z;

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
            unsigned char lVal = (unsigned char)i;
            mLut[i] = Color{lVal, lVal, lVal, 255};
        }
        mLutDirty = false;
        return;
    }

    int lNumStops = (int)mPaletteStops.size();
    for (int i = 0; i < LUT_SIZE; ++i) {
        float lT = (float)i / (float)(LUT_SIZE - 1);
        float lScaled = lT * (float)(lNumStops - 1);
        int lIdx0 = (int)lScaled;
        int lIdx1 = lIdx0 + 1;
        if (lIdx1 >= lNumStops) {
            lIdx1 = lNumStops - 1;
            lIdx0 = lIdx1 - 1;
        }
        if (lIdx0 < 0) lIdx0 = 0;

        float lLocalT = lScaled - (float)lIdx0;
        mLut[i] = RLCharts::lerpColor(mPaletteStops[(size_t)lIdx0], mPaletteStops[(size_t)lIdx1], lLocalT);
    }

    mLutDirty = false;
}

void RLHeatMap3D::buildMesh() {
    if (mWidth < 2 || mHeight < 2) {
        return;
    }

    freeMesh();

    int lCellsX = mWidth - 1;
    int lCellsY = mHeight - 1;
    int lTriangleCount = lCellsX * lCellsY * 2;
    int lVertexCount = lTriangleCount * 3;

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

    int lCellsX = mWidth - 1;
    int lCellsY = mHeight - 1;
    float lHalfSize = BOX_SIZE * 0.5f;
    float lHeight = BOX_SIZE;

    int lVertIdx = 0;
    for (int lCy = 0; lCy < lCellsY; ++lCy) {
        for (int lCx = 0; lCx < lCellsX; ++lCx) {
            // Grid positions mapped to [-halfSize, +halfSize]
            float lX0 = -lHalfSize + ((float)lCx / (float)lCellsX) * lHalfSize * 2.0f;
            float lX1 = -lHalfSize + ((float)(lCx + 1) / (float)lCellsX) * lHalfSize * 2.0f;
            float lZ0 = -lHalfSize + ((float)lCy / (float)lCellsY) * lHalfSize * 2.0f;
            float lZ1 = -lHalfSize + ((float)(lCy + 1) / (float)lCellsY) * lHalfSize * 2.0f;

            int lIdx00 = lCy * mWidth + lCx;
            int lIdx10 = lCy * mWidth + (lCx + 1);
            int lIdx01 = (lCy + 1) * mWidth + lCx;
            int lIdx11 = (lCy + 1) * mWidth + (lCx + 1);

            float lN00 = normalizeValue(mCurrentValues[(size_t)lIdx00]);
            float lN10 = normalizeValue(mCurrentValues[(size_t)lIdx10]);
            float lN01 = normalizeValue(mCurrentValues[(size_t)lIdx01]);
            float lN11 = normalizeValue(mCurrentValues[(size_t)lIdx11]);

            float lH00 = lN00 * lHeight;
            float lH10 = lN10 * lHeight;
            float lH01 = lN01 * lHeight;
            float lH11 = lN11 * lHeight;

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

float RLHeatMap3D::normalizeValue(float aValue) const {
    float lRange = mMaxValue - mMinValue;
    if (lRange < 1e-6f) {
        return 0.5f;
    }
    float lNorm = (aValue - mMinValue) / lRange;
    return RLCharts::clamp01(lNorm);
}

Color RLHeatMap3D::getColorForValue(float aNormalizedValue) const {
    float lClamped = RLCharts::clamp01(aNormalizedValue);
    int lIdx = (int)(lClamped * 255.0f);
    if (lIdx < 0) lIdx = 0;
    if (lIdx > 255) lIdx = 255;
    return mLut[lIdx];
}

