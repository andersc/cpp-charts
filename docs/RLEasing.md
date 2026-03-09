# RLEasing — Animation Easing Library

Control the feel of chart animations with different easing modes.

## Easing Modes

Set `mEaseMode` on any chart's style struct to change animation character:

| Mode       | Description                                  |
|------------|----------------------------------------------|
| `LINEAR`   | Constant speed toward target                 |
| `SMOOTH`   | Exponential decay (default) — natural settle |
| `SNAPPY`   | Fast start, gradual settle (doubled decay)   |
| `SPRINGY`  | Overshoots target, springs back              |
| `ELASTIC`  | Damped oscillation around target             |

## Usage

```cpp
#include "RLEasing.h"

// Set on any chart style
RLBarChartStyle lStyle;
lStyle.mEaseMode = RLEaseMode::SPRINGY;
lBarChart.setStyle(lStyle);
```

## Standalone Easing Functions

For custom animations, use the standalone functions (input/output in `[0, 1]`):

```cpp
float lT = 0.5f;
float lEased = RLEasing::easeOutCubic(lT);   // 0.875
```

| Function            | Curve Type      |
|---------------------|-----------------|
| `linear(t)`         | Linear          |
| `easeInQuad(t)`     | Quadratic in    |
| `easeOutQuad(t)`    | Quadratic out   |
| `easeInOutQuad(t)`  | Quadratic both  |
| `easeInCubic(t)`    | Cubic in        |
| `easeOutCubic(t)`   | Cubic out       |
| `easeInOutCubic(t)` | Cubic both      |
| `easeInExpo(t)`     | Exponential in  |
| `easeOutExpo(t)`    | Exponential out |
| `easeOutElastic(t)` | Elastic out     |
| `easeOutBounce(t)`  | Bounce out      |
| `easeOutBack(t)`    | Overshoot out   |

## Frame-Based Approach

`approachEased()` is a drop-in replacement for the per-frame approach pattern:

```cpp
float lValue = RLEasing::approachEased(lCurrent, lTarget, lSpeed * lDt, RLEaseMode::SNAPPY);
```

`approachColorEased()` works the same way for `Color` values.
