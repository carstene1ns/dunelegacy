# Network Wait Timing & Pathfinding Budget Increase

**Date**: 2025-10-21  
**Branch**: `release-0.98.6`  
**Status**: ✅ Complete

## Problem

Performance logs were showing frame times that didn't add up. For example:
- Frame Time: 17.92ms
- AI: 0.48ms
- Units: 0.14ms  
- Structures: 0.11ms
- Pathfinding: 0.51ms
- Rendering: 3.09ms
- **Measured Total: ~4.3ms**
- **MISSING: ~13.6ms**

The missing time was network wait time when the game was waiting for other players in multiplayer, but this wasn't being measured or logged.

## Root Cause

In `Game::runMainLoop()`, when `handleNetworkUpdates()` returns `bWaitForNetwork = true`:
- The game **skips** `updateGameState()` 
- The main loop **waits** for network synchronization
- This wait time was **NOT tracked** in performance metrics

This made it impossible to diagnose multiplayer performance issues or understand where frame time was going.

## Changes Made

### 1. Added Network Wait Tracking to Performance System

**File**: `include/Game.h`

Added to `FrameTiming` struct:
```cpp
double networkWaitMs = 0.0;              // Accumulated network wait time
double networkWaitMsThisFrame = 0.0;     // Per-frame network wait
double maxNetworkWaitMs = 0.0;           // Peak network wait
double minNetworkWaitMs = 999999.0;      // Minimum network wait
```

### 2. Increased Pathfinding Budget

**File**: `include/Game.h`

```cpp
// Before:
static constexpr double PathBudgetMs = 6.0;

// After:
static constexpr double PathBudgetMs = 12.0;  // Increased for 32ms frames
```

**Rationale**: With 32ms frame time for multiplayer (31 FPS):
- Network wait can consume 10-15ms
- Pathfinding budget of 6ms was leaving 11ms unused
- Increasing to 12ms allows better pathfinding without impacting frame rate
- Previous testing showed max pathfinding was 8.49ms, so 12ms provides headroom

### 3. Implemented Network Wait Timing

**File**: `src/Game.cpp`

**In `runMainLoop()`** (lines 1113-1141):
```cpp
while((frameTime > getGameSpeed()) || (!finished && (gameCycleCount < skipToGameCycle))) {
    Uint64 networkWaitStart = SDL_GetPerformanceCounter();
    bool bWaitForNetwork = false;
    if(pNetworkManager != nullptr) {
        bWaitForNetwork = handleNetworkUpdates();
    }

    processInput();
    // ... other updates ...

    if(!bWaitForNetwork && !bPause) {
        updateGameState();
        frameTiming.gameCyclesThisFrame++;
    } else if(bWaitForNetwork) {
        // Measure time spent waiting for network
        Uint64 networkWaitEnd = SDL_GetPerformanceCounter();
        const double networkWaitMs = getElapsedMs(networkWaitStart, networkWaitEnd);
        frameTiming.networkWaitMs += networkWaitMs;
        frameTiming.networkWaitMsThisFrame += networkWaitMs;
        if(networkWaitMs > frameTiming.maxNetworkWaitMs) 
            frameTiming.maxNetworkWaitMs = networkWaitMs;
    }
}
```

**Reset per-frame accumulator** (line 1076):
```cpp
frameTiming.networkWaitMsThisFrame = 0.0;
```

**Track min/max values** (lines 1178, 1185):
```cpp
if(frameTiming.networkWaitMsThisFrame < frameTiming.minNetworkWaitMs) 
    frameTiming.minNetworkWaitMs = frameTiming.networkWaitMsThisFrame;
if(frameTiming.networkWaitMsThisFrame > frameTiming.maxNetworkWaitMs) 
    frameTiming.maxNetworkWaitMs = frameTiming.networkWaitMsThisFrame;
```

### 4. Updated Performance Logging

**File**: `src/Game.cpp` (`logFrameTiming()`)

**Added network wait calculation** (line 1393):
```cpp
const double avgNetworkWait = frameTiming.networkWaitMs / frameTiming.frameCount;
```

**Added to log output** (line 1422-1423):
```cpp
SDL_Log("[Performance] NetworkWait: min=%.2fms avg=%.2fms max=%.2fms",
    frameTiming.minNetworkWaitMs, avgNetworkWait, frameTiming.maxNetworkWaitMs);
```

**Added to PEAKS line** (line 1429):
```cpp
SDL_Log("[Performance] FPS: %.1f | Frame: %.2fms | AI: %.2fms | Units: %.2fms | Structures: %.2fms | Pathfinding: %.2fms | NetworkWait: %.2fms | Rendering: %.2fms",
    maxFps, frameTiming.maxTotalMs,
    frameTiming.maxAiMs, frameTiming.maxUnitsMs, frameTiming.maxStructuresMs, 
    frameTiming.maxPathfindingMs, frameTiming.maxNetworkWaitMs, frameTiming.maxRenderingMs);
```

**Reset counters** (lines 1441, 1452, 1463):
```cpp
frameTiming.networkWaitMs = 0.0;
frameTiming.maxNetworkWaitMs = 0.0;
frameTiming.minNetworkWaitMs = 999999.0;
```

## Expected Results

### New Log Output

Performance logs will now show network wait time:

```
[Performance] === AVERAGES over 2855 frames ===
[Performance] FPS: 95.1 | Frame: 10.51ms
[Performance] GameCycles/Frame: min=0 avg=1.3 max=23
[Performance] AI:         min=0.00ms avg=0.23ms max=5.23ms
[Performance] Units:      min=0.00ms avg=0.16ms max=2.59ms
[Performance] Structures: min=0.00ms avg=0.22ms max=17.03ms
[Performance] Pathfinding: min=0.00ms avg=0.49ms max=8.80ms
[Performance] NetworkWait: min=0.00ms avg=0.00ms max=0.00ms  ← NEW!
[Performance] Rendering:  min=1.85ms avg=2.82ms max=170.46ms
[Performance] Pathfinding Detail: 0.8 paths/frame | 0.59 paths/cycle | 0.17ms/cycle | 0.28ms/path
[Performance] === PEAKS (worst case) ===
[Performance] FPS: 8.1 | Frame: 123.98ms | AI: 3.14ms | Units: 1.78ms | Structures: 18.23ms | Pathfinding: 8.49ms | NetworkWait: 0.00ms | Rendering: 7.69ms  ← NEW!
```

In multiplayer, `NetworkWait` will show the actual time spent waiting for other players.

### Pathfinding Improvements

With 12ms pathfinding budget:
- More pathfinding work per frame
- Better unit responsiveness
- Smoother movement in large battles
- Can process ~2x more paths before hitting budget cap

## Testing Recommendations

1. **Multiplayer Test**:
   - Join a 2+ player game
   - Check logs for `NetworkWait` values
   - Verify: `AI + Units + Structures + Pathfinding + NetworkWait + Rendering ≈ Total Frame Time`

2. **Pathfinding Test**:
   - Large map with 100+ units
   - Check `Pathfinding Detail` logs
   - Expected: More paths/frame, smoother unit movement
   - Max pathfinding should stay under 12ms cap

3. **Performance Regression Test**:
   - Single player should show `NetworkWait: min=0.00ms avg=0.00ms max=0.00ms`
   - Frame times should remain stable or improve

## Files Changed

- `include/Game.h` - Added network wait tracking fields, increased pathfinding budget
- `src/Game.cpp` - Implemented network wait timing and logging

## Next Steps

1. ✅ **Build and test** - Verify compilation
2. ⏳ **Multiplayer testing** - Capture logs with network wait data
3. ⏳ **Performance analysis** - Determine optimal pathfinding budget for 32ms frames
4. ⏳ **Documentation** - Update performance tuning guide with new metrics

## Notes

- Network wait only occurs in multiplayer games
- Single player games will always show 0ms network wait
- The 12ms pathfinding budget is conservative; could be increased further based on testing
- With 32ms frame time: ~10ms network, ~12ms pathfinding, ~3ms rendering, ~7ms other systems

---

**Build Status**: ✅ Compiled successfully (Debug configuration)  
**Commit**: Pending testing

