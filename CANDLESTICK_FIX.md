# RLCandlestickChart Fix - Single Value Per Candle

## Problem Description

The RLCandlestickChart had an issue where candlesticks with only one value per candle would not display properly:

1. **First Candlestick Never Displayed**: When `valuesPerCandle=1`, the very first candlestick would not show because:
   - With only one value, open=close, resulting in zero height
   - A candlestick needs a visual difference between open and close to be visible

2. **Subsequent Candlesticks Had Same Issue**: Each new candlestick would start fresh with open=close from the same sample, causing the same visibility problem

## Solution

The fix implements a mechanism to carry forward the close price from one candle to be used as the open price for the next candle:

### Changes to `RLCandlestickChart.h`

Added two new private member variables:
```cpp
// Track last close price for single-value candles
float mLastClose{0.0f};
bool mHasLastClose{false};
```

### Changes to `RLCandlestickChart.cpp`

1. **In `addSample()` method**:
   - When starting a new working candle, check if `valuesPerCandle == 1` and we have a previous close price
   - If yes, use `mLastClose` as the open price for the new candle
   - Otherwise, use the sample's open price as normal

2. **In `finalizeWorkingCandle()` method**:
   - Store the current candle's close price in `mLastClose`
   - Set `mHasLastClose = true` to indicate we have a valid previous close

### How It Works

- **First Candle**: Still uses open=close from the first sample (no previous candle exists yet)
- **Second and Subsequent Candles**: Use the previous candle's close as their open, and the current sample's close as their close
- **Result**: Each candle now shows the price movement from the previous close to the current close
- **Special Handling**: Only activates when `valuesPerCandle == 1`, so multi-value candles work as before

## New Demo File

Created `candlestick2.cpp` - a demonstration specifically for single-value candles:
- Location: `/Users/anderscedronius/git/cpp-charts/src/examples/candlestick2.cpp`
- Shows three charts all with `valuesPerCandle=1`
- Different visible candle counts (20, 30, 50)
- Feeds data every 0.5 seconds for faster visualization

## CMakeLists.txt Update

Added new build target:
```cmake
add_executable(raylib_candlestick2
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/candlestick2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/charts/RLCandlestickChart.cpp
)
target_link_libraries(raylib_candlestick2
        raylib
        Threads::Threads
)
```

## Testing

To build and run the new demo:
```bash
cd cmake-build-debug
cmake ..
cmake --build . --target raylib_candlestick2
./raylib_candlestick2
```

## Expected Behavior

With this fix:
1. ✅ First candlestick appears (though with open=close, may be a thin line)
2. ✅ Second and subsequent candlesticks show proper price movement (previous close → current close)
3. ✅ Smooth sliding animation as new candles are added
4. ✅ Multi-value candles continue to work as before (no regression)

