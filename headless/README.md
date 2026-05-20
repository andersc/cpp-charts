# Headless Chart Rendering

Render charts to PNG images without a GPU or display server, using the raylib 6.0 **Memory platform** backend with the software renderer (`rlsw`).

## Use Cases

- **CI/CD pipelines** — Generate chart images in automated builds
- **Docker containers** — No GPU or X11/Wayland required
- **Server-side rendering** — Generate chart PNGs from backend services
- **Automated reports** — Batch-render charts for PDF/HTML reports

## Build

```bash
cd headless
mkdir build && cd build
cmake ..
make
```

The key difference from the normal build is `PLATFORM=Memory` which tells raylib to use a framebuffer in RAM instead of a GPU-backed window.

## Run

```bash
./headless_charts                  # outputs chart_output.png
./headless_charts my_chart.png     # custom output path
```

## How It Works

1. `InitWindow()` creates a memory framebuffer (no window appears)
2. Charts render into the framebuffer using the software renderer
3. `LoadImageFromScreen()` captures the framebuffer contents
4. `ExportImage()` writes the result as PNG

## Limitations

- Rendering is CPU-only (slower than GPU for complex scenes)
- No input handling (keyboard/mouse) — render-and-exit only
- No anti-aliasing (MSAA requires GPU)
- Font rendering works but custom fonts must be in the working directory

## Integration Example

```cpp
#include "RLBarChart.h"

void exportChart(const std::vector<RLBarData>& aData, const char* aPath) {
    InitWindow(800, 600, "headless");

    RLBarChart lChart;
    lChart.setBounds({20, 20, 760, 560});
    lChart.setData(aData);

    BeginDrawing();
    ClearBackground(WHITE);
    lChart.draw();
    EndDrawing();

    Image lImg = LoadImageFromScreen();
    ExportImage(lImg, aPath);
    UnloadImage(lImg);

    CloseWindow();
}
```
