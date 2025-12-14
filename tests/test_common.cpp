// test_common.cpp
// Unit tests for RLCommon.h utility functions
#include "doctest/doctest.h"
#include "RLCommon.h"
#include <cmath>

TEST_SUITE("RLCommon") {

    TEST_CASE("clamp01") {
        CHECK(RLCharts::clamp01(0.5f) == doctest::Approx(0.5f));
        CHECK(RLCharts::clamp01(-0.5f) == doctest::Approx(0.0f));
        CHECK(RLCharts::clamp01(1.5f) == doctest::Approx(1.0f));
        CHECK(RLCharts::clamp01(0.0f) == doctest::Approx(0.0f));
        CHECK(RLCharts::clamp01(1.0f) == doctest::Approx(1.0f));
    }

    TEST_CASE("clamp") {
        CHECK(RLCharts::clamp(5.0f, 0.0f, 10.0f) == doctest::Approx(5.0f));
        CHECK(RLCharts::clamp(-5.0f, 0.0f, 10.0f) == doctest::Approx(0.0f));
        CHECK(RLCharts::clamp(15.0f, 0.0f, 10.0f) == doctest::Approx(10.0f));
        CHECK(RLCharts::clamp(0, 0, 100) == 0);
        CHECK(RLCharts::clamp(150, 0, 100) == 100);
    }

    TEST_CASE("clampIdx") {
        CHECK(RLCharts::clampIdx(5, 10) == 5);
        CHECK(RLCharts::clampIdx(-1, 10) == 0);
        CHECK(RLCharts::clampIdx(10, 10) == 9);
        CHECK(RLCharts::clampIdx(15, 10) == 9);
    }

    TEST_CASE("lerp") {
        CHECK(RLCharts::lerp(0.0f, 10.0f, 0.5f) == doctest::Approx(5.0f));
        CHECK(RLCharts::lerp(0.0f, 10.0f, 0.0f) == doctest::Approx(0.0f));
        CHECK(RLCharts::lerp(0.0f, 10.0f, 1.0f) == doctest::Approx(10.0f));
        CHECK(RLCharts::lerp(-10.0f, 10.0f, 0.5f) == doctest::Approx(0.0f));
    }

    TEST_CASE("lerpF") {
        CHECK(RLCharts::lerpF(0.0f, 100.0f, 0.25f) == doctest::Approx(25.0f));
        CHECK(RLCharts::lerpF(100.0f, 0.0f, 0.5f) == doctest::Approx(50.0f));
    }

    TEST_CASE("lerpColor") {
        Color lBlack = {0, 0, 0, 255};
        Color lWhite = {255, 255, 255, 255};

        Color lMid = RLCharts::lerpColor(lBlack, lWhite, 0.5f);
        CHECK(lMid.r == 127);
        CHECK(lMid.g == 127);
        CHECK(lMid.b == 127);
        CHECK(lMid.a == 255);

        // Clamps t to [0,1]
        Color lClamped = RLCharts::lerpColor(lBlack, lWhite, 2.0f);
        CHECK(lClamped.r == 255);
        CHECK(lClamped.g == 255);
        CHECK(lClamped.b == 255);
    }

    TEST_CASE("degToRad") {
        CHECK(RLCharts::degToRad(0.0f) == doctest::Approx(0.0f));
        CHECK(RLCharts::degToRad(180.0f) == doctest::Approx(PI));
        CHECK(RLCharts::degToRad(90.0f) == doctest::Approx(PI / 2.0f));
        CHECK(RLCharts::degToRad(360.0f) == doctest::Approx(2.0f * PI));
    }

    TEST_CASE("radToDeg") {
        CHECK(RLCharts::radToDeg(0.0f) == doctest::Approx(0.0f));
        CHECK(RLCharts::radToDeg(PI) == doctest::Approx(180.0f));
        CHECK(RLCharts::radToDeg(PI / 2.0f) == doctest::Approx(90.0f));
    }

    TEST_CASE("colorLuma") {
        Color lBlack = {0, 0, 0, 255};
        Color lWhite = {255, 255, 255, 255};
        Color lRed = {255, 0, 0, 255};

        CHECK(RLCharts::colorLuma(lBlack) == doctest::Approx(0.0f));
        CHECK(RLCharts::colorLuma(lWhite) == doctest::Approx(255.0f).epsilon(0.01));
        CHECK(RLCharts::colorLuma(lRed) == doctest::Approx(54.213f).epsilon(0.01));
    }

    TEST_CASE("minVal and maxVal") {
        CHECK(RLCharts::minVal(5, 10) == 5);
        CHECK(RLCharts::minVal(10, 5) == 5);
        CHECK(RLCharts::maxVal(5, 10) == 10);
        CHECK(RLCharts::maxVal(10, 5) == 10);
        CHECK(RLCharts::minVal(-5.0f, 5.0f) == doctest::Approx(-5.0f));
    }

    TEST_CASE("approach") {
        // Approach should move value towards target
        float lResult = RLCharts::approach(0.0f, 10.0f, 0.5f);
        CHECK(lResult > 0.0f);
        CHECK(lResult < 10.0f);

        // Full approach (speedDt >= 1)
        lResult = RLCharts::approach(0.0f, 10.0f, 1.0f);
        CHECK(lResult == doctest::Approx(10.0f));
    }

    TEST_CASE("mulAlpha") {
        CHECK(RLCharts::mulAlpha(255, 0.5f) == 128);
        CHECK(RLCharts::mulAlpha(255, 0.0f) == 0);
        CHECK(RLCharts::mulAlpha(255, 1.0f) == 255);
        CHECK(RLCharts::mulAlpha(100, 2.0f) == 200); // 100*2 = 200
        CHECK(RLCharts::mulAlpha(200, 2.0f) == 255); // 200*2 = 400, clamped to 255
    }

    TEST_CASE("lerpVector2") {
        Vector2 lA = {0.0f, 0.0f};
        Vector2 lB = {10.0f, 20.0f};

        Vector2 lMid = RLCharts::lerpVector2(lA, lB, 0.5f);
        CHECK(lMid.x == doctest::Approx(5.0f));
        CHECK(lMid.y == doctest::Approx(10.0f));
    }

    TEST_CASE("distance") {
        Vector2 lA = {0.0f, 0.0f};
        Vector2 lB = {3.0f, 4.0f};

        CHECK(RLCharts::distance(lA, lB) == doctest::Approx(5.0f));
        CHECK(RLCharts::distance(lA, lA) == doctest::Approx(0.0f));
    }

    TEST_CASE("catmullRom") {
        Vector2 lP0 = {0.0f, 0.0f};
        Vector2 lP1 = {1.0f, 1.0f};
        Vector2 lP2 = {2.0f, 1.0f};
        Vector2 lP3 = {3.0f, 0.0f};

        // At t=0, should be at P1
        Vector2 lAt0 = RLCharts::catmullRom(lP0, lP1, lP2, lP3, 0.0f);
        CHECK(lAt0.x == doctest::Approx(1.0f));
        CHECK(lAt0.y == doctest::Approx(1.0f));

        // At t=1, should be at P2
        Vector2 lAt1 = RLCharts::catmullRom(lP0, lP1, lP2, lP3, 1.0f);
        CHECK(lAt1.x == doctest::Approx(2.0f));
        CHECK(lAt1.y == doctest::Approx(1.0f));
    }

}

