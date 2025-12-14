// test_main.cpp
// Doctest entry point with headless raylib initialization
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include "raylib.h"

int main(int aArgc, char** apArgv) {
    // Initialize raylib in headless mode (no visible window)
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(800, 600, "cpp-charts tests");

    // Run doctest
    doctest::Context lContext;
    lContext.applyCommandLine(aArgc, apArgv);
    int lResult = lContext.run();

    // Cleanup raylib
    CloseWindow();

    return lResult;
}

