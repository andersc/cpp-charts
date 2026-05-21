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
# PNG export
./headless_charts                  # outputs chart_output.png
./headless_charts my_chart.png     # custom output path

# Raw pixel buffer export
./pixelbuffer_charts               # outputs chart_output.rgba
./pixelbuffer_charts frame.rgba    # custom output path
```

## How It Works

1. `InitWindow()` creates a memory framebuffer (no window appears)
2. Charts render into the framebuffer using the software renderer
3. `LoadImageFromScreen()` captures the framebuffer contents
4. `ExportImage()` writes the result as PNG

## Output Formats

| Method | Output | Use Case |
|--------|--------|----------|
| `ExportImage(img, "f.png")` | PNG file | Reports, web display |
| `ExportImage(img, "f.jpg")` | JPEG file | Smaller file size |
| `ExportImage(img, "f.bmp")` | BMP file | Uncompressed |
| `(uint8_t*)img.data` | Raw RGBA buffer | Network streaming, IPC, custom pipelines |
| `ExportImageToMemory(img, ".png", &size)` | In-memory encoded bytes | HTTP responses |

## Raw Pixel Buffer Example

The `pixelbuffer_charts` example renders a chart and writes the raw RGBA
pixel data to a file. This is useful for sending frames over a socket,
piping to FFmpeg, or integrating with custom rendering systems.

```cpp
#include "RLBarChart.h"
#include <cstdio>
#include <cstdint>

void sendPixelBuffer(const std::vector<RLBarData>& aData) {
    InitWindow(800, 600, "pixelbuffer");

    Rectangle lBounds{20.0f, 20.0f, 760.0f, 560.0f};
    RLBarChart lChart(lBounds, RLBarOrientation::VERTICAL);
    lChart.setData(aData);

    BeginDrawing();
    ClearBackground(WHITE);
    lChart.draw();
    EndDrawing();

    Image lImg = LoadImageFromScreen();
    uint8_t* lPixels = (uint8_t*)lImg.data;
    int32_t lSize = lImg.width * lImg.height * 4;

    // Send raw RGBA bytes (e.g., over socket, pipe, shared memory)
    fwrite(lPixels, 1, (size_t)lSize, stdout);

    UnloadImage(lImg);
    CloseWindow();
}
```

Pipe to FFmpeg to create a video frame:
```bash
./pixelbuffer_charts /dev/stdout | ffmpeg -f rawvideo -pixel_format rgba \
    -video_size 800x600 -i - -frames:v 1 frame.png
```

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

    Rectangle lBounds{20.0f, 20.0f, 760.0f, 560.0f};
    RLBarChart lChart(lBounds, RLBarOrientation::VERTICAL);
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
