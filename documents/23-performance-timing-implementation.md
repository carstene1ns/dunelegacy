# Performance Timing System Implementation

## Goal
Add diagnostic logging to show how much time (in milliseconds) each major game system takes per frame:
- Unit updates
- Structure updates
- Pathfinding
- Rendering
- Total frame time

## Implementation Plan

### 1. Add timing structure to Game.h
```cpp
struct FrameTiming {
    double unitsMs = 0.0;
    double structuresMs = 0.0;
    double pathfindingMs = 0.0;
    double renderingMs = 0.0;
    double totalMs = 0.0;
    int frameCount = 0;
};
```

### 2. Add helper functions to Game class
- `double getTimeMs(Uint64 start, Uint64 end)` - Convert SDL performance counter to ms
- `void logFrameTiming()` - Log timing stats periodically

### 3. Instrument Game::update()
- Wrap each major section with `SDL_GetPerformanceCounter()` calls
- Accumulate timing over ~60 frames
- Log average every second

### 4. Example Output
```
[Performance] Avg frame: 12.3ms | Units: 4.5ms | Structures: 2.1ms | Pathfinding: 1.2ms | Rendering: 4.5ms
```

## Benefits
- Pure diagnostic feature (no game logic changes)
- Helps identify bottlenecks
- Can guide optimization decisions (e.g., pathfinding budget)
- Low overhead (just timing, no complex tracking)

---

## Implementation Status: ✅ COMPLETE

### Files Modified
1. **include/Game.h**
   - Added `FrameTiming` struct (lines 647-654)
   - Added `frameTiming` and `lastTimingLogMs` fields (lines 656-657)
   - Added `getElapsedMs()` helper function (lines 659-662)
   - Added `logFrameTiming()` declaration (line 664)

2. **src/Game.cpp**
   - Implemented `logFrameTiming()` (lines 1253-1274)
   - Added timing to `updateGameState()` (lines 1168, 1204-1213)
   - Added timing to `processObjects()` for pathfinding (lines 259-262), structures (lines 272-277), units (lines 284-289)
   - Added timing to `renderFrame()` (lines 1131, 1144-1145)

### How It Works
1. **Frame Start**: `updateGameState()` records the start time
2. **Subsystems**: Each major subsystem (pathfinding, structures, units, rendering) is wrapped with timing calls
3. **Accumulation**: Times are accumulated in `frameTiming` struct
4. **Logging**: Every 2 seconds, averages are logged and counters reset
5. **Output Format**: `[Performance] Avg frame: 12.3ms | Units: 4.5ms | Structures: 2.1ms | Pathfinding: 1.2ms | Rendering: 4.5ms | Frames: 120`

### Testing
- ✅ Compiles successfully (Release configuration)
- ✅ No linter errors
- ⏳ Runtime testing pending (user will test next)

### Next Steps
User should:
1. Run the game
2. Observe `[Performance]` logs in console every 2 seconds
3. Use timing data to identify bottlenecks
4. Can adjust pathfinding budget if needed based on results

