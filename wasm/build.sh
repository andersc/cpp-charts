#!/bin/bash

# cpp-charts WebAssembly Build Script
# This script builds all chart demos for WebAssembly using Emscripten

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║           cpp-charts WebAssembly Build                       ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Check if Emscripten is available
if ! command -v emcmake &> /dev/null; then
    echo "❌ Error: Emscripten not found!"
    echo ""
    echo "Please install and activate Emscripten SDK:"
    echo ""
    echo "  # Clone emsdk"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo ""
    echo "  # Install and activate latest version"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo ""
    echo "  # Activate PATH and environment variables"
    echo "  source ./emsdk_env.sh"
    echo ""
    exit 1
fi

echo "✅ Emscripten found: $(emcc --version | head -n1)"
echo ""

# Create build directory
echo "📁 Creating build directory..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Run CMake with Emscripten
echo ""
echo "🔧 Configuring with CMake..."
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "🔨 Building all demos..."
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
emmake make -j${NPROC}

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                    Build Complete! 🎉                        ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "📂 Output directory: ${BUILD_DIR}"
echo ""
echo "To run the demos locally:"
echo ""
echo "  cd ${BUILD_DIR}"
echo "  python3 -m http.server 8080"
echo ""
echo "Then open http://localhost:8080 in your browser"
echo ""

