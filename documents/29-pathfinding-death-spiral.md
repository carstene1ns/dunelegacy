# Pathfinding Death Spiral - Critical Performance Issue

## The Problem

**Observed:** 1.7 FPS (588ms per frame), game is unplayable
**Root Cause:** Pathfinding consuming 534ms per frame (90% of frame time)

### Performance Breakdown (from logs):
```
Frame Time:   588.42ms total
├─ Pathfinding: 534.21ms (90.8%) 🔥
├─ Units:        26.49ms (4.5%)
├─ Rendering:     5.28ms (0.9%)
├─ Structures:    3.11ms (0.5%)
└─ AI:            1.03ms (0.2%)

Pathfinding Stats:
- 468.4 paths per frame
- 72.8 game cycles per frame (catching up!)
- 6.43 paths per cycle
- 7.34ms per cycle (exceeds 6ms budget)
- 1.14ms per path
```

## The Death Spiral

1. Pathfinding takes too long (534ms)
2. Frame exceeds target time (32ms for game speed)
3. Game runs 72 cycles to "catch up"
4. Each cycle generates ~6.4 path requests
5. 468 paths queued per frame
6. Pathfinding takes even longer
7. **REPEAT** → Game becomes unplayable

## Root Causes

### 1. Too Many Units Pathfinding Simultaneously
- Large battles with 100+ units
- Each unit requests paths frequently
- No prioritization or throttling

### 2. Time-Sliced Pathfinding Queue System
- Paths requested in one cycle are processed in the next
- When game is behind, path queue explodes
- No way to shed load or prioritize critical paths

### 3. Fixed Timestep Catch-Up Loop
```cpp
while((frameTime > getGameSpeed()) || (!finished && (gameCycleCount < skipToGameCycle))) {
    updateGameState();  // Generates path requests
    frameTiming.gameCyclesThisFrame++;
}
```
- Runs multiple game cycles per frame when behind
- Each cycle generates MORE path requests
- Creates positive feedback loop

### 4. No Path Caching or Reuse
- Units recalculate paths even for nearby destinations
- No path smoothing or waypoint reuse
- Attack-move generates continuous path requests

## Potential Solutions

### Option 1: Increase Pathfinding Budget (Quick Fix)
**Change:** Increase `PathBudgetMs` from 6ms to 12-15ms
**Pros:** Simple, allows more paths per cycle
**Cons:** Doesn't fix root cause, just delays the spiral

### Option 2: Limit Game Cycles Per Frame (Break the Spiral)
**Change:**
```cpp
const int MAX_CYCLES_PER_FRAME = 5;  // Cap catch-up
int cyclesThisFrame = 0;
while((frameTime > getGameSpeed()) && cyclesThisFrame < MAX_CYCLES_PER_FRAME) {
    updateGameState();
    cyclesThisFrame++;
}
```
**Pros:** Breaks death spiral, game slows down but stays playable
**Cons:** Game runs in "slow motion" when overloaded

### Option 3: Throttle Path Requests (Reduce Load)
**Change:** Limit units to 1 path request per N cycles
- Units wait longer between path requests
- Priority queue: combat units > harvesters > idle
**Pros:** Reduces pathfinding load
**Cons:** Units may appear less responsive

### Option 4: Path Caching (Smart Fix)
**Change:** Cache paths for similar start/end positions
- Reuse paths within small radius
- Smooth existing paths instead of recalculating
**Pros:** Dramatically reduces path requests
**Cons:** Requires significant code changes

### Option 5: Simplify A* for Distant Targets
**Change:** Use cheaper heuristics for long-distance paths
- Rough path first, refine when closer
- Hierarchical pathfinding
**Pros:** Faster path calculation
**Cons:** Complex to implement

## Recommended Immediate Actions

### 1. **IMMEDIATE: Limit Game Cycles Per Frame**
```cpp
// In Game::runMainLoop()
const int MAX_CYCLES_PER_FRAME = 5;
int cyclesThisFrame = 0;

while((frameTime > getGameSpeed()) && cyclesThisFrame < MAX_CYCLES_PER_FRAME) {
    // ... existing code ...
    cyclesThisFrame++;
}
```
This will make the game run in "slow motion" but at least it will be playable.

### 2. **SHORT TERM: Increase Path Budget**
Change `PathBudgetMs` from `6.0` to `10.0` or `12.0`
- Allows more paths to be processed per cycle
- Reduces queue buildup

### 3. **MEDIUM TERM: Implement Path Request Throttling**
- Units can only request path every 5-10 cycles
- Combat units get priority
- Stationary units don't request paths

### 4. **LONG TERM: Path Caching System**
- Cache recently calculated paths
- Reuse paths for nearby destinations
- Smooth/adjust existing paths

## Why This Wasn't Caught Earlier

1. **Small maps:** Early testing likely had fewer units
2. **Short games:** Death spiral takes time to develop
3. **Debug mode:** Performance characteristics differ
4. **Measurement blind spot:** Only added detailed pathfinding logging recently

## Evidence from Logs

**Game Start (54 FPS):**
- 0.2 paths/frame
- 2.3 cycles/frame
- 0.06ms pathfinding

**Mid Game (41 FPS):**
- 22.9 paths/frame
- 3.0 cycles/frame
- 14ms pathfinding

**Late Game (3.9 FPS):**
- 220.6 paths/frame
- 31.3 cycles/frame
- 219ms pathfinding

**Current (1.7 FPS):**
- **468.4 paths/frame**
- **72.8 cycles/frame**
- **534ms pathfinding**

The progression is clear: as more units are added and battle intensifies, pathfinding load increases exponentially until the game becomes unplayable.

## Next Steps

1. Implement MAX_CYCLES_PER_FRAME cap (immediate)
2. Increase pathfinding budget (quick win)
3. Profile specific pathfinding bottlenecks (why is each path taking 1.14ms?)
4. Consider throttling or caching (medium term)

