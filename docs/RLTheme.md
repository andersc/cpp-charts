# RLTheme — Global Theme System

Apply a consistent color palette across all charts with a single call.

## Theme Structure

`RLChartTheme` defines color roles that map to each chart's style fields:

| Role            | Description                        |
|-----------------|------------------------------------|
| `mBackground`   | Chart background fill              |
| `mForeground`   | Primary text and labels            |
| `mForegroundDim`| Secondary text, tick marks         |
| `mGrid`         | Grid lines                         |
| `mAxis`         | Axis lines                         |
| `mBorder`       | Cell / node borders                |
| `mPositive`     | Positive values (green)            |
| `mNegative`     | Negative values (red)              |
| `mWarning`      | Warning / mid-range (yellow)       |
| `mAccents[8]`   | Series color palette (8 colors)    |

## Built-in Presets

```cpp
auto lDark     = RLThemes::dark();      // Default dark theme
auto lLight    = RLThemes::light();     // Light background
auto lMidnight = RLThemes::midnight();  // Deep blue
auto lNeon     = RLThemes::neon();      // High-contrast neon
```

## Applying a Theme

Every chart has an `applyTheme()` method:

```cpp
auto lTheme = RLThemes::neon();
lBarChart.applyTheme(lTheme);
lPieChart.applyTheme(lTheme);
lTimeSeries.applyTheme(lTheme);
// ... all 16 chart types supported
```

## Custom Themes

Create your own by modifying a preset or building from scratch:

```cpp
RLChartTheme lCustom;
lCustom.mBackground = {30, 30, 40, 255};
lCustom.mForeground = {240, 240, 250, 255};
lCustom.mAccents[0] = {255, 100, 50, 255};
lBarChart.applyTheme(lCustom);
```

## Theme Selector Widget

`RLThemeSelector` is a built-in UI widget that draws clickable theme buttons:

```cpp
RLThemeSelector lSelector;

// In your draw loop:
if (lSelector.draw(x, y, font, fontSize)) {
    // Selection changed — apply to charts
    auto lTheme = lSelector.getTheme();
    lBarChart.applyTheme(lTheme);
}

// Use matching window background
ClearBackground(lSelector.getWindowBackground());
```

### RLThemeSelector API

| Method                 | Description                              |
|------------------------|------------------------------------------|
| `draw(x, y, font, sz)`| Draw buttons, returns true on change     |
| `getTheme()`           | Get the currently selected theme         |
| `getWindowBackground()`| Get complementary window background color|
| `mSelected`            | Current index (0=Dark, 1=Light, 2=Midnight, 3=Neon) |

All demos include the theme selector — click the buttons in the top-right corner to switch themes.
