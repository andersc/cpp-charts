#!/bin/bash
# create_gifs.sh
# Generates animated GIFs for charts that are missing them.
# Builds the capture tool, runs it for each chart, and uses ffmpeg to create GIFs.
#
# Usage: cd tools && ./create_gifs.sh
# Requirements: cmake, c++ compiler, raylib, ffmpeg

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
GIFS_DIR="$SCRIPT_DIR/../docs/gifs"
FRAMES_DIR="$BUILD_DIR/frames"

# Charts to generate GIFs for and their output filenames
CHARTS=("areachart:RLAreaChart" "lineargauge:RLLinearGauge" "radar:RLRadarChart" "sankey:RLSankey")

echo "=== cpp-charts GIF Generator ==="
echo ""

# Build the capture tool
echo "Building gif_capture tool..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1
cmake --build . --config Release > /dev/null 2>&1
echo "Build complete."
echo ""

# Generate GIFs for each chart
for ENTRY in "${CHARTS[@]}"; do
    CHART_NAME="${ENTRY%%:*}"
    GIF_NAME="${ENTRY##*:}"
    CHART_FRAMES="$FRAMES_DIR/$CHART_NAME"
    OUTPUT_GIF="$GIFS_DIR/$GIF_NAME.gif"

    echo "--- Generating $GIF_NAME.gif ---"

    # Create frames directory
    rm -rf "$CHART_FRAMES"
    mkdir -p "$CHART_FRAMES"

    # Capture frames
    echo "  Capturing frames for $CHART_NAME..."
    cd "$BUILD_DIR"
    ./gif_capture "$CHART_NAME" "$CHART_FRAMES"

    # Count frames
    FRAME_COUNT=$(ls "$CHART_FRAMES"/frame_*.png 2>/dev/null | wc -l | tr -d ' ')
    echo "  Captured $FRAME_COUNT frames."

    if [ "$FRAME_COUNT" -eq 0 ]; then
        echo "  ERROR: No frames captured for $CHART_NAME!"
        continue
    fi

    # Create GIF using ffmpeg 2-pass palette method (matching convert.txt)
    echo "  Creating optimized GIF..."
    PALETTE="$CHART_FRAMES/palette.png"

    # Pass 1: Generate palette
    ffmpeg -y -framerate 15 -i "$CHART_FRAMES/frame_%04d.png" \
        -vf "scale=640:-1:flags=lanczos,palettegen" \
        "$PALETTE" -loglevel error

    # Pass 2: Create GIF with palette
    ffmpeg -y -framerate 15 -i "$CHART_FRAMES/frame_%04d.png" -i "$PALETTE" \
        -lavfi "scale=640:-1:flags=lanczos[x];[x][1:v]paletteuse" \
        -t 4 "$OUTPUT_GIF" -loglevel error

    # Report file size
    GIF_SIZE=$(ls -lh "$OUTPUT_GIF" | awk '{print $5}')
    echo "  Created: $OUTPUT_GIF ($GIF_SIZE)"
    echo ""
done

# Clean up frames
echo "Cleaning up temporary frames..."
rm -rf "$FRAMES_DIR"

echo ""
echo "=== Done! Generated GIFs in $GIFS_DIR ==="
ls -lh "$GIFS_DIR"/*.gif
