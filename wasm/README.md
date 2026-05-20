# 🌐 cpp-charts WebAssembly Build

This directory contains everything needed to build cpp-charts demos for WebAssembly, allowing them to run directly in web browsers.

## 🚀 Live Demo

**👉 [https://andersc.github.io/cpp-charts/](https://andersc.github.io/cpp-charts/)**

The demos are automatically built and deployed to GitHub Pages on every push to `main`. No manual deployment needed!

## Prerequisites

### Emscripten SDK

You need the Emscripten SDK installed and activated. Follow these steps:

```bash
# Clone the Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install the latest version
./emsdk install latest

# Activate it
./emsdk activate latest

# Set up environment variables (run this in each new terminal session)
source ./emsdk_env.sh
```

> **Note:** On Windows, use `emsdk.bat` instead of `./emsdk` and `emsdk_env.bat` instead of `source ./emsdk_env.sh`.

### CMake

CMake 3.20 or higher is required. Install via your package manager:

```bash
# macOS
brew install cmake

# Ubuntu/Debian
sudo apt install cmake

# Windows (with Chocolatey)
choco install cmake
```

## Building

### Quick Build (Recommended)

Run the build script:

```bash
cd wasm
./build.sh
```

### Manual Build

If you prefer to run commands manually:

```bash
cd wasm
mkdir -p build
cd build

# Configure with Emscripten
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# Build all demos
emmake make -j$(nproc)
```

## Running the Demos

After building, serve the `build` directory with any HTTP server:

```bash
cd wasm/build
python3 -m http.server 8080
```

Then open **http://localhost:8080** in your web browser.

> **Important:** WebAssembly files must be served over HTTP(S). Opening the HTML files directly (via `file://`) will not work due to browser security restrictions.

## Available Demos

| Demo | Description | File |
|------|-------------|------|
| 📊 Bar Chart | Vertical/horizontal bars with animations | `barchart.html` |
| 🥧 Pie Chart | Pie and donut charts | `piechart.html` |
| 📈 Scatter Plot | Multi-series scatter/line plots | `scatter.html` |
| 🫧 Bubble Chart | Gravity-simulated bubbles | `bubble.html` |
| 🕯️ Candlestick | OHLCV financial chart (JPM data) | `candlestick.html` |
| 📉 Candlestick Live | Real-time candle formation | `candlestick2.html` |
| 📊 Time Series | Streaming multi-trace plots | `timeseries.html` |
| 📊 Order Book | DOM visualization (2D/3D) | `orderbook.html` |
| 🗺️ Heat Map | 2D matrix visualization | `heatmap.html` |
| 🏔️ 3D Heat Map | Interactive 3D surface | `heatmap3d.html` |
| 📉 Log-Log Plot | Allan variance analysis | `logplot.html` |
| 🌳 Tree Map | Hierarchical data | `treemap.html` |
| 🌡️ Gauges | Circular gauge displays | `gauges.html` |

## Directory Structure

```
wasm/
├── CMakeLists.txt    # CMake build configuration
├── build.sh          # Build script
├── shell.html        # Custom HTML template for demos
├── index.html        # Landing page (copied to build/)
├── JPM_sample.csv    # Smaller stock data sample (100 rows) for web demos
├── README.md         # This file
└── build/            # Build output (created by build.sh)
    ├── index.html    # Landing page
    ├── *.html        # Demo pages
    ├── *.js          # Generated JavaScript
    ├── *.wasm        # WebAssembly binaries
    └── *.data        # Preloaded assets
```

## How It Works

The build system:

1. **Fetches raylib 6.0** via CMake's `FetchContent` and configures it for WebAssembly (`PLATFORM=Web`)
2. **Compiles each demo** to WebAssembly using Emscripten
3. **Embeds assets** (fonts, CSV data) into `.data` files using `--preload-file`
4. **Uses a custom shell** (`shell.html`) for beautiful, consistent styling across all demos

### Emscripten Flags Used

- `-sUSE_GLFW=3` - Use GLFW 3 for window/input handling
- `-sASYNCIFY` - Enable async operations (required for raylib)
- `-sALLOW_MEMORY_GROWTH=1` - Allow dynamic memory growth
- `-sTOTAL_MEMORY=67108864` - Initial memory (64 MB)
- `--shell-file shell.html` - Custom HTML template
- `--preload-file` - Embed assets into the build

## Troubleshooting

### "emcmake: command not found"

Make sure you've activated the Emscripten environment:

```bash
source /path/to/emsdk/emsdk_env.sh
```

### Build fails with memory errors

Try increasing the memory limit in `CMakeLists.txt`:

```cmake
set(COMMON_LINK_FLAGS
    ...
    "-sTOTAL_MEMORY=134217728"  # 128 MB
    ...
)
```

### Demos don't load / white screen

- Make sure you're serving via HTTP, not `file://`
- Check the browser console (F12) for errors
- Try a different browser (Chrome, Firefox, Edge recommended)

### Performance issues

- WebGL performance varies by browser and GPU
- Try closing other tabs/applications
- Some demos (3D heatmap, order book) are more demanding

## Browser Compatibility

Tested and working on:

- ✅ Chrome 90+
- ✅ Firefox 90+
- ✅ Edge 90+
- ✅ Safari 15+

WebAssembly and WebGL 2.0 support is required.

## License

Same as the main cpp-charts project - MIT License.

