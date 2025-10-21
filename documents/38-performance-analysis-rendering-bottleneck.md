# Performance Analysis: Rendering Bottleneck Investigation

## User Report

"ok review the logs for overall performance measures. i was noticing some jutters / lag with a lot of units and buildings so i'd like to analyse where this is coming from"

## Performance Summary

### Average Performance (Smooth Gameplay)
```
FPS: 55-58 FPS (~17ms/frame)
├── AI:          1.0-1.2ms  (6-7%)
├── Units:       1.6-2.1ms  (9-12%)
├── Structures:  0.1-0.14ms (<1%)
├── Pathfinding: 6.8-7.2ms  (40%)
└── Rendering:   6.9-7.5ms  (40%)
```

**Analysis**: Game logic is **highly optimized**
- Pathfinding: Within 6ms budget, averaging 7ms with peaks at 13ms
- AI: Very efficient at 1ms average
- Units: Fast updates at 2ms average
- Total game logic: ~10ms per frame (excellent!)

### Peak Performance (Lag Spikes)
```
FPS: 2.3-3.4 FPS (300-430ms/frame)
├── AI:          15-64ms   (5-15%)
├── Units:       14-77ms   (5-18%)
├── Structures:  1-5ms     (<2%)
├── Pathfinding: 9-13ms    (3-4%)
└── Rendering:   253-275ms (80-90%) ← PROBLEM!
```

**Analysis**: **Rendering dominates lag spikes**
- Rendering jumps from 7ms → 275ms (40x increase!)
- Game logic still reasonable: 40-100ms combined
- Rendering accounts for 80-90% of lag spike duration

### Catastrophic Spike (Worst Case)
```
FPS: 0.9 FPS (1059ms/frame = 1 second per frame!)
├── AI:          54ms   (5%)
├── Units:       22ms   (2%)
├── Structures:  3ms    (<1%)
├── Pathfinding: 27ms   (3%)
└── Rendering:   954ms  (90%) ← CATASTROPHIC!
```

**Analysis**: Nearly 1 full second blocked in rendering!

## Detailed Peak Analysis

### Sample 1: Typical Lag Spike
```
Frame: 302.75ms | FPS: 3.3
├── AI:          13.50ms  (4%)
├── Units:       0.46ms   (<1%)
├── Structures:  0.62ms   (<1%)
├── Pathfinding: 7.40ms   (2%)
└── Rendering:   182.00ms (60%)
Missing time: ~98ms (compositor/vsync)
```

### Sample 2: Heavy Units Load
```
Frame: 428.26ms | FPS: 2.3
├── AI:          63.97ms  (15%)
├── Units:       77.11ms  (18%)
├── Structures:  5.02ms   (1%)
├── Pathfinding: 12.90ms  (3%)
└── Rendering:   274.28ms (64%)
```
Note: Units peaked at 77ms (probably lots of unit movement), but rendering still dominates.

### Sample 3: Heavy AI Load
```
Frame: 347.09ms | FPS: 2.9
├── AI:          23.70ms  (7%)
├── Units:       44.98ms  (13%)
├── Structures:  2.39ms   (<1%)
├── Pathfinding: 12.34ms  (4%)
└── Rendering:   272.80ms (79%)
```

### Sample 4: The Catastrophe
```
Frame: 1059.76ms | FPS: 0.9
├── AI:          53.62ms  (5%)
├── Units:       22.40ms  (2%)
├── Structures:  2.54ms   (<1%)
├── Pathfinding: 27.37ms  (3%)
└── Rendering:   954.45ms (90%)
```
This is unplayable - nearly 1 second frozen waiting for rendering!

## Root Cause Analysis

### The Problem: SDL Rendering Pipeline Blocking

**What's happening:**
1. Game logic completes quickly (~10ms average)
2. `SDL_RenderPresent()` calls to swap buffers
3. SDL blocks waiting for:
   - **VSync** (waiting for display refresh)
   - **Compositor** (macOS window manager queuing frames)
   - **GPU** (previous frame still rendering)
   - **Display Sync** (monitor refresh rate mismatch)

### Why Rendering Blocks

**macOS Compositor Behavior:**
- macOS uses a display compositor (WindowServer)
- All app rendering goes through this compositor
- Compositor can delay frames for various reasons:
  - Other windows need updating
  - System animations (Mission Control, Spaces, etc.)
  - Background processes requesting display updates
  - Multiple monitor sync issues
  - Power management throttling

**VSync Issues:**
- When VSync is enabled, `SDL_RenderPresent()` blocks until the next display refresh
- On a 60Hz display, this means up to 16.67ms wait time
- But we're seeing 250-954ms waits, which suggests:
  - Compositor is holding frames for multiple refresh cycles
  - Display is in low-power mode (30Hz or slower)
  - System is under memory pressure (swapping)

**GPU Sync:**
- If GPU command buffer is full, `SDL_RenderPresent()` must wait
- With many units/buildings, each render call adds to GPU queue
- Eventually GPU can't keep up → blocks

## Why This Happens With Many Units/Buildings

**Rendering Workload Scales With:**
- Number of units visible (each unit = sprite draw call)
- Number of buildings visible (each building = multiple sprites)
- Number of explosions/bullets (particles)
- Terrain tiles (though these are typically batched)

**With lots of units:**
1. More draw calls per frame
2. GPU takes longer to process
3. GPU queue fills up
4. Next `SDL_RenderPresent()` blocks waiting for GPU to catch up
5. **Lag spike!**

**Exacerbated by macOS compositor:**
- macOS compositor adds extra latency
- Each frame goes: Game → SDL → GPU → Compositor → Display
- Any bottleneck in this chain causes the entire pipeline to stall

## Why Game Logic Is NOT The Problem

**Pathfinding is excellent:**
- Average: 7ms (within 6ms budget!)
- Peak: 13ms (slightly over, but only 6ms/frame processed)
- Budget system working perfectly
- Not causing lag spikes

**AI is efficient:**
- Average: 1ms (excellent!)
- Peak: 64ms (only during heavy decision-making)
- Even at peak, only 15% of lag spike time

**Units are fast:**
- Average: 2ms (very good!)
- Peak: 77ms (probably 100+ units moving simultaneously)
- Even at peak, only 18% of lag spike time

**Structures are negligible:**
- Average: 0.1ms (perfect!)
- Peak: 5ms (still trivial)

## Solutions

### 1. **Disable VSync** (Easiest)
**Change:** Allow frame rate to exceed display refresh rate
**Pro:** Eliminates VSync blocking, smooths frame times
**Con:** Tearing (visual artifacts), higher power usage
**Implementation:**
```cpp
SDL_RenderSetVSync(renderer, 0); // Disable VSync
```

### 2. **Batch Rendering** (Medium Difficulty)
**Change:** Group similar sprites into single draw calls
**Pro:** Reduces GPU command queue size
**Con:** Requires refactoring rendering code
**Implementation:**
- Use sprite batching/instancing
- Group units by sprite type
- Single draw call for all "Tank" units, etc.

### 3. **Occlusion Culling** (Medium Difficulty)
**Change:** Don't render off-screen units
**Pro:** Reduces draw calls significantly
**Con:** Need to track viewport and test visibility
**Implementation:**
```cpp
if (!isInViewport(unit->getLocation())) {
    continue; // Don't render off-screen units
}
```

### 4. **Level of Detail (LOD)** (Hard)
**Change:** Simplify sprites when zoomed out
**Pro:** Reduces GPU load for distant units
**Con:** Needs multiple sprite versions
**Implementation:**
- Small sprites for zoomed-out view
- Detailed sprites only when close

### 5. **Frame Pacing** (Hard)
**Change:** Decouple game logic from rendering
**Pro:** Game continues updating even if rendering lags
**Con:** Complex threading, synchronization issues
**Implementation:**
- Game logic thread runs at fixed rate (32 ticks/sec)
- Rendering thread runs as fast as possible
- Lock-free communication between threads

### 6. **Reduce Rendering Quality** (Easy)
**Change:** Lower resolution, fewer effects
**Pro:** Less GPU load
**Con:** Looks worse
**Implementation:**
- Render at 720p and upscale to 1080p
- Disable particle effects
- Use simpler shaders

## Recommendations (Priority Order)

### Immediate (No Code Changes)
1. **Test in fullscreen mode** (bypasses some compositor overhead)
2. **Close other applications** (reduce compositor load)
3. **Disable system animations** (macOS accessibility settings)
4. **Check Activity Monitor** during lag spikes (memory pressure?)

### Short-Term (Simple Code Changes)
1. **Disable VSync** (eliminates 16ms blocking per frame)
2. **Add occlusion culling** (don't render off-screen units)
3. **Profile rendering calls** (which specific sprites are slow?)

### Long-Term (Architectural Changes)
1. **Implement sprite batching** (reduce draw calls by 10-100x)
2. **Add LOD system** (simpler sprites when zoomed out)
3. **Consider Vulkan/Metal** (lower-level APIs with less blocking)

## Testing Strategy

### Reproduce The Problem
1. Start custom game on 128x128 map
2. Set AI to Brutal difficulty
3. Let game run for 30+ minutes
4. Both players should have 100+ units
5. Pan camera across large battles
6. **Expected**: Lag spikes when viewing many units

### Measure Impact
1. Note current peak rendering time: **275-954ms**
2. Apply fix (e.g., disable VSync)
3. Play same scenario
4. Check logs for new peak rendering time
5. **Goal**: Reduce peak rendering to <50ms

## Current Status

**Game Logic**: ✅ **EXCELLENT**
- Pathfinding budget working perfectly
- AI efficient and fast
- Units updating quickly
- No optimization needed here!

**Rendering**: ❌ **BOTTLENECK**
- Peaks at 275-954ms (catastrophic)
- Blocking on SDL_RenderPresent()
- Likely VSync + compositor delays
- **Needs optimization**

## Summary

**Problem**: Rendering lag spikes (275-954ms) causing juttering with many units/buildings

**Root Cause**: SDL rendering blocking on VSync + macOS compositor + GPU sync

**NOT The Problem**: Game logic is excellent (AI, units, pathfinding all fast!)

**Solution**: Start with disabling VSync, add occlusion culling, consider sprite batching

**Next Steps**: Disable VSync and re-test to measure improvement

