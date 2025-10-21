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

