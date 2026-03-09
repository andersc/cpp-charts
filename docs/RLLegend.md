# RLLegend

Shared legend rendering utility for all chart types. Draws a vertical list of color indicators with labels, positioned in the top-right corner of the chart bounds.

## Features

- 📊 Vertical legend with colored indicator boxes
- 🎨 Customizable colors, font, padding, and indicator size
- 👁️ Per-entry visibility for fade animations
- 📐 Automatic size measurement via `measure()`
- 🖋️ Supports both default and custom fonts

## Data Structures

### RLLegendEntry

| Field        | Type          | Default                | Description                  |
|--------------|---------------|------------------------|------------------------------|
| mLabel       | std::string   |                        | Legend entry text             |
| mColor       | Color         | {200, 200, 210, 255}   | Indicator color              |
| mVisibility  | float         | 1.0                    | Opacity [0..1] for fade anim |

### RLLegendStyle

| Field           | Type  | Default                | Description                  |
|-----------------|-------|------------------------|------------------------------|
| mTextColor      | Color | {200, 200, 210, 255}   | Label text color             |
| mBackground     | Color | {0, 0, 0, 0}           | Background (transparent)     |
| mFontSize       | float | 12.0                   | Font size in pixels          |
| mPadding        | float | 8.0                    | Inner padding                |
| mSpacing        | float | 4.0                    | Spacing between entries      |
| mIndicatorSize  | float | 10.0                   | Color indicator box size     |
| mFont           | Font  | {}                     | Custom font (default if empty) |

## Usage

### Drawing a Legend

```cpp
#include "RLLegend.h"

std::vector<RLLegendEntry> lEntries;
lEntries.push_back({"Temperature", RED, 1.0f});
lEntries.push_back({"Humidity", BLUE, 1.0f});

RLLegend::draw(lEntries, chartBounds);
```

### Measuring Legend Size

```cpp
auto lSize = RLLegend::measure(lEntries);
// lSize.x = width, lSize.y = height
```

### With Custom Style

```cpp
RLLegendStyle lStyle;
lStyle.mFontSize = 14.0f;
lStyle.mBackground = Color{20, 22, 28, 180};
lStyle.mIndicatorSize = 12.0f;

RLLegend::draw(lEntries, chartBounds, lStyle);
```

## Integrated Charts

The following charts have built-in legend support via their style struct:

| Chart          | Style Field       | Default |
|----------------|-------------------|---------|
| RLAreaChart    | mShowLegend       | true    |
| RLRadarChart   | mShowLegend       | true    |
| RLScatterPlot  | mShowLegend       | false   |
| RLTimeSeries   | mShowLegend       | false   |

Charts with `mShowLegend = false` by default need explicit enablement. Each chart's style also includes an `mLegendStyle` field of type `RLLegendStyle` for customization (where applicable).
