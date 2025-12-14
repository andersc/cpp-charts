// test_main.cpp
// Doctest entry point with optional headless raylib initialization
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include "raylib.h"
#include <cstdlib>
#include <cstdio>

// Global flag to indicate if raylib is available
bool gRaylibAvailable = false;

int main(int aArgc, char** apArgv) {
    // Check if we should skip raylib initialization (CI environment without display)
    const char* lpSkipRaylib = std::getenv("CPP_CHARTS_SKIP_RAYLIB");

    if (lpSkipRaylib == nullptr) {
        // Try to initialize raylib in headless mode
        SetTraceLogLevel(LOG_WARNING); // Reduce noise
        SetConfigFlags(FLAG_WINDOW_HIDDEN);
        InitWindow(800, 600, "cpp-charts tests");
        gRaylibAvailable = IsWindowReady();

        if (!gRaylibAvailable) {
            std::fprintf(stderr, "TEST: Window creation failed, raylib-dependent tests will be skipped\n");
        }
    } else {
        std::fprintf(stderr, "TEST: CPP_CHARTS_SKIP_RAYLIB set, skipping raylib initialization\n");
        std::fprintf(stderr, "TEST: Only RLCommon math tests will run\n");
    }

    // Run doctest
    doctest::Context lContext;
    lContext.applyCommandLine(aArgc, apArgv);
    int lResult = lContext.run();

    // Cleanup raylib if it was initialized
    if (gRaylibAvailable) {
        CloseWindow();
    }

    return lResult;
}

