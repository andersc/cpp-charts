#include <iostream>
#include "raylib.h"

int main() {
    constexpr int lScreenWidth = 1240;
    constexpr int lScreenHeight = 800;

    InitWindow(lScreenWidth, lScreenHeight, "Some test");
    SetTargetFPS(60);

    float x_skew = 0;
    float y_skew = 0;
    float z_skew = 0;
    float x_curr_start = 0;
    float y_curr_start = 0;
    float z_curr_start = 0;
    float max_circle_size = 50;
    float min_circle_size = 10;

    while (!WindowShouldClose()) {

        x_curr_start += 0.009422;
        y_curr_start += 0.008294;
        z_curr_start += 0.042155;

        BeginDrawing();
        ClearBackground(BLACK);

        x_skew = x_curr_start;
        y_skew = y_curr_start;
        z_skew = z_curr_start;

        for (int i = 0; i < 500 ; i++) {
            double x_pos = (sin(x_skew) + 1) / 2;
            double y_pos = (cos(y_skew) + 1) / 2;
            float z_size = (cos(z_skew) + 1) / 2;
            float circle_size = ((max_circle_size - min_circle_size) * z_size) + min_circle_size;
            Color circle_color;
            circle_color.r = (uint8_t)(z_size * 255);
            circle_color.g = (uint8_t)(x_pos * 255);
            circle_color.b = (uint8_t)(y_pos * 255);
            circle_color.a = 255;
            x_skew += 0.1103039;
            y_skew += -0.1294393;
            z_skew += 0.123994;
            DrawCircle(
                (int)(x_pos * (lScreenWidth-(max_circle_size * 2)) + max_circle_size),
                (int)(y_pos * (lScreenHeight-(max_circle_size * 2)) + max_circle_size),
                circle_size,
                circle_color
                );
        }

        DrawFPS(10,10);
        EndDrawing();
    }

    return EXIT_SUCCESS;
}