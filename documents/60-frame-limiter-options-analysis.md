# Frame Limiting & Pathfinding Options Analysis
**Dune Legacy Performance Optimization**

---

## Executive Summary

The current 32ms frame limiter was specifically designed to accommodate the pathfinding budget (12ms/frame = 37.5% of frame time). Moving to 60 FPS requires addressing pathfinding bottleneck, which currently averages 14.5ms with peaks at 29ms. This paper analyzes 5 options for improving visual smoothness while maintaining multiplayer stability.

**Key Finding:** Frame limiter only affects rendering rate, NOT game cycle rate. Pathfinding acts as natural FPS limiter when uncapped.

---

## Current State Analysis

### Frame Loop Architecture
```
OUTER LOOP (Frame):
  1. Render scene                    → ~5-6ms
  2. Accumulate real time            → frameTime += actualTime
  3. Apply frame limiter (optional)  → delay to 32ms
  
INNER LOOP (Game Cycles):
  While (frameTime > gameSpeed):     → Usually 1-2 cycles per frame
    - Network sync
    - Process input
    - Update game state (AI, units, structures, pathfinding)
    - frameTime -= gameSpeed
```

### Critical Measurements (From Logs)
| Component | Average | Peak | % of 32ms Frame |
|-----------|---------|------|-----------------|
| **Pathfinding** | 14.5ms | 29.29ms | 45% avg, 91% peak |
| Rendering | 6ms | 27ms | 19% |
| Units | 1.5ms | 18.5ms | 5% |
| AI | 1.1ms | 17ms | 3% |
| Structures | 0.9ms | 10ms | 3% |
| **TOTAL** | ~24ms | 71.7ms | 75% avg |

### Key Insights
1. **Pathfinding budget (12ms) already exceeded** - avg 14.5ms actual
2. **Frame limiter is decoupled from game cycles** - removing it doesn't speed up game logic
3. **Multiplayer uses lock-step** - game speed is synced across all players
4. **Pathfinding budget is per-frame** - designed specifically for 32ms frames

---

## Option 1: Status Quo (32ms Software Limiter)

### Configuration
- Frame limiter: ON (32ms = 31 FPS)
- Pathfinding budget: 12ms per frame
- VSync: OFF

### Pros
✅ **Proven stable** - current production configuration  
✅ **Pathfinding has breathing room** - 12/32 = 37.5% of frame budget  
✅ **Handles spikes well** - 9ms buffer for pathfinding overruns  
✅ **Multiplayer stable** - consistent frame timing across different hardware  
✅ **Power efficient** - GPU not rendering unnecessarily  
✅ **No code changes needed**

### Cons
❌ **Choppy visuals** - 31 FPS noticeable on 60Hz+ monitors  
❌ **Appears dated** - users expect 60 FPS minimum  
❌ **Wasted GPU capability** - fast GPUs render in 5ms then wait 27ms

### Multiplayer Impact
🟢 **Excellent** - Original design intent, smooth synchronized timing

### Performance Ceiling
**Graphics:** 31 FPS (fixed)  
**Game Logic:** ~60 cycles/sec (at 16ms game speed)  
**Pathfinding:** Can use full 12ms budget, handles ~62 paths/sec

---

## Option 2: Remove Frame Limiter (Unlimited FPS)

### Configuration
- Frame limiter: OFF
- Pathfinding budget: 12ms per frame (same)
- VSync: OFF

### Pros
✅ **Smooth on fast hardware** - could hit 200+ FPS on fast GPUs  
✅ **No artificial restrictions** - lets hardware perform  
✅ **Still multiplayer-safe** - game cycles remain deterministic  
✅ **Pathfinding acts as natural limiter** - frames slow down during heavy pathfinding  
✅ **Simple implementation** - just remove limiter check  
✅ **User choice** - via checkbox in menu

### Cons
❌ **Inconsistent frame rate** - varies wildly (30-200 FPS) based on pathfinding load  
❌ **Stuttering** - visible frame time variance when pathfinding spikes  
❌ **Power inefficient** - GPU renders 200 FPS when only 30-60 needed  
❌ **Screen tearing** - no VSync synchronization  
❌ **Variable multiplayer timing** - different frame execution patterns per player

### Reality Check: Pathfinding as Natural Limiter
```
Fast GPU render (5ms):
  Frame with no pathfinding: 5ms → 200 FPS
  Frame with avg pathfinding (14.5ms): 19.5ms → 51 FPS
  Frame with peak pathfinding (29ms): 34ms → 29 FPS
  
Result: FPS fluctuates 29-200 depending on pathfinding activity
```

### Multiplayer Impact
🟡 **Acceptable but inconsistent** - Game cycles stay synced, but frame-to-frame timing varies. Could cause micro-stuttering when one player has pathfinding spike.

### Performance Ceiling
**Graphics:** 29-200 FPS (variable, pathfinding-limited)  
**Game Logic:** ~60 cycles/sec (unchanged)  
**Pathfinding:** Same 12ms/frame budget, but frames are variable duration

---

## Option 3: Enable VSync (Adaptive FPS)

### Configuration
- Frame limiter: Replaced by VSync
- SDL flags: `SDL_RENDERER_PRESENTVSYNC`
- Pathfinding budget: 12ms per frame
- Adapts to monitor refresh rate (60Hz/144Hz/etc)

### Pros
✅ **No screen tearing** - hardware-level synchronization  
✅ **Smooth visuals** - if can maintain monitor refresh rate  
✅ **Automatic adaptation** - works with any monitor (60/75/144 Hz)  
✅ **Cleaner code** - remove manual delay logic  
✅ **Standard practice** - industry-standard approach  
✅ **Works on working hardware** - fast GPUs maintain 60 FPS between pathfinding spikes

### Cons
❌ **Frame rate halving** - VSync drops to 30 FPS when can't hit 60 FPS  
❌ **Pathfinding causes drops** - peak 29ms pathfinding forces 30 FPS  
❌ **Stuttering during battles** - FPS oscillates 30↔60 based on unit count  
❌ **Feels worse than consistent 31 FPS** - variable 30-60 more jarring than stable 31  
❌ **Multiplayer timing variance** - 60Hz vs 144Hz monitors create different frame patterns

### Reality Check: VSync Behavior
```
60Hz Monitor (16.67ms per frame):
  Light pathfinding (0-6ms): Hits 60 FPS ✓
  Average pathfinding (14.5ms): Total ~20ms → MISS → drops to 30 FPS ✗
  Heavy pathfinding (29ms): Total ~34ms → MISS → drops to 30 FPS ✗
  
Result: Constantly oscillates between 30-60 FPS during gameplay
```

### Multiplayer Impact
🟡 **Risky** - Different monitor refresh rates create timing asymmetry. 60Hz player at 30 FPS while 144Hz player at 72 FPS could cause sync issues.

### Performance Ceiling
**Graphics:** 30-60 FPS (on 60Hz monitor), stuttery transitions  
**Game Logic:** ~60 cycles/sec (unchanged)  
**Pathfinding:** Same 12ms budget, but failures more visible (frame drops)

---

## Option 4: 60 FPS with Reduced Pathfinding Budget

### Configuration
- Frame limiter: 16ms (60 FPS)
- Pathfinding budget: **6ms per frame** (halved)
- VSync: OFF

### Pros
✅ **Smooth 60 FPS** - consistent frame rate  
✅ **Pathfinding proportional** - 6/16 = 37.5% (same ratio as current)  
✅ **More responsive UI** - doubled render rate  
✅ **Handles rendering spikes** - 10ms buffer  

### Cons
❌ **Pathfinding severely limited** - only 6ms vs current 14.5ms average  
❌ **Units slower to respond** - pathfinding queue backs up  
❌ **Large battles suffer** - 100+ units requesting paths can't be processed fast enough  
❌ **Requires pathfinding retuning** - may need algorithm optimization  
❌ **"Feels slow" complaint risk** - units appear dumber with halved pathfinding

### Implementation Impact
**Code changes required:**
```cpp
static constexpr double PathBudgetMs = 6.0;  // Halved for 16ms frames
```

### Multiplayer Impact
🟢 **Good** - Consistent timing, just slower pathfinding for all players equally

### Performance Ceiling
**Graphics:** 60 FPS (fixed)  
**Game Logic:** ~60 cycles/sec (unchanged)  
**Pathfinding:** Halved capacity - ~31 paths/sec instead of ~62

---

## Option 5: 60 FPS with Multi-Frame Pathfinding (RECOMMENDED)

### Configuration
- Frame limiter: 16ms (60 FPS) OR VSync
- Pathfinding budget: **3ms per frame** (¼ current)
- Pathfinding **spread across 4 frames**
- VSync: Optional

### Concept
Instead of trying to process all pathfinding in one frame, spread it across multiple frames:

```
Current (32ms frames):
  Frame 1: Process 12ms of pathfinding → 20 paths
  Frame 2: Process 12ms of pathfinding → 20 paths
  Total: 40 paths every 2 frames (64ms)

Proposed (16ms frames):  
  Frame 1: Process 3ms of pathfinding → 5 paths
  Frame 2: Process 3ms of pathfinding → 5 paths
  Frame 3: Process 3ms of pathfinding → 5 paths
  Frame 4: Process 3ms of pathfinding → 5 paths
  Total: 20 paths every 4 frames (64ms)
```

**Same throughput, smoother delivery!**

### Pros
✅ **Smooth 60 FPS graphics** - consistent frame times  
✅ **Same pathfinding capacity** - total throughput unchanged  
✅ **Better frame time consistency** - 3ms is 18% of frame (vs 45% currently)  
✅ **Huge buffer for spikes** - 13ms remaining per frame for rendering/other work  
✅ **More predictable performance** - smaller per-frame variance  
✅ **VSync compatible** - reliable 16ms frames allow stable VSync  
✅ **Scales to higher refresh rates** - can spread across 8 frames for 144Hz

### Cons
❌ **Implementation complexity** - requires pathfinding queue refactoring  
❌ **Slightly slower response** - individual unit paths take up to 4 frames (~64ms) instead of 1-2 frames  
❌ **Testing required** - need to verify no race conditions  
❌ **Latency increase** - worst-case path resolution: 16ms × 4 = 64ms vs current 32ms × 2 = 64ms (actually same!)

### Implementation Approach
```cpp
// Instead of: "process until budget exhausted"
// Use: "process fixed time slice, queue remainder for next frame"

static constexpr double PathBudgetMs = 3.0;  // Small budget per frame
static constexpr int PathFrameSpread = 4;    // Spread across 4 frames

void Game::processPathRequests() {
    if(pathRequestQueue.empty()) return;
    
    const double budgetSeconds = PathBudgetMs / 1000.0;
    const Uint64 start = SDL_GetPerformanceCounter();
    
    // Process for exactly 3ms, then stop
    while(!pathRequestQueue.empty()) {
        if(getElapsedMs(start, SDL_GetPerformanceCounter()) >= PathBudgetMs) {
            break;  // Time's up, defer rest to next frame
        }
        // Process one path request
        PathRequest request = pathRequestQueue.front();
        pathRequestQueue.pop_front();
        // ... resolve path ...
    }
}
```

### Multiplayer Impact
🟢 **Excellent** - Smoother frame timing helps multiplayer consistency. Path resolution slightly delayed but still deterministic.

### Performance Ceiling
**Graphics:** 60 FPS (smooth, consistent)  
**Game Logic:** ~60 cycles/sec (unchanged)  
**Pathfinding:** Same total capacity (~62 paths/sec), spread over more frames

---

## Option 6: Hybrid Approach (User Configurable)

### Configuration
Provide multiple modes in settings:

**"Performance Mode"** (Default)
- 32ms limiter, 12ms pathfinding budget
- Most stable, works on all hardware

**"Balanced Mode"**
- VSync enabled, 6ms pathfinding budget
- Smooth when possible, acceptable fallback

**"Quality Mode"**
- VSync enabled, 3ms pathfinding with multi-frame spread
- Best experience on capable hardware

### Pros
✅ **User choice** - players pick their preference  
✅ **Accommodates hardware variance** - old PCs use Performance, new PCs use Quality  
✅ **Multiplayer safe** - game cycles still synced regardless of choice  
✅ **Marketing value** - "multiple quality settings" sounds professional

### Cons
❌ **Complex implementation** - need all variants working  
❌ **Testing burden** - must verify all combinations  
❌ **User confusion** - requires understanding of options  
❌ **Increased maintenance** - more code paths to support

---

## Multiplayer Deep Dive

### Why Frame Limiter Doesn't Break Multiplayer

**Critical insight:** Game cycles accumulate **real time**, not frames:

```
Player A (Fast GPU, no limiter):
  Renders at 200 FPS, but game cycles still run at 60/sec
  
Player B (Slow GPU, 32ms limiter):
  Renders at 31 FPS, but game cycles still run at 60/sec
  
Result: Both execute cycle #12345 at same real-world time
```

**Lock-step networking ensures:**
1. All players use **same game speed** (16ms/cycle in multiplayer)
2. Commands are queued ahead of time (network buffer)
3. Cycle N doesn't execute until **all players** have submitted commands for cycle N
4. Rendering rate doesn't affect game logic rate

**What CAN break multiplayer:**
- ❌ Frame-dependent game logic (we don't have this)
- ❌ Timing-dependent randomness (we use synced RNG with seed)
- ❌ Float precision differences (careful with FixedPoint math)
- ✅ Different frame limiters: **SAFE** (game cycles remain synced)

### Smoothness Factor

The frame limiter affects **perceived smoothness** but not **game fairness**:

**30 FPS player vs 60 FPS player:**
- 30 FPS sees game state updating every 33ms (choppier)
- 60 FPS sees game state updating every 16ms (smoother)
- Both players' units move at identical speeds
- Both players execute commands at identical times
- Advantage: 60 FPS player has smoother aiming/clicking, but same game logic

**Recommendation:** Allow different frame limiters in multiplayer, but recommend 32ms mode for competitive fairness.

---

## Recommendations

### Short-Term (Low Risk)
**Option 2: Remove Frame Limiter (make it truly optional)**

**Rationale:**
- Minimal code changes (already implemented checkbox)
- Lets fast hardware shine
- Pathfinding naturally limits FPS (29-200 FPS range)
- Multiplayer-safe (game cycles unchanged)
- User choice via existing UI

**Implementation:** Already done - just verify multiplayer testing

### Medium-Term (Moderate Risk, High Reward)
**Option 5: Multi-Frame Pathfinding Spread + VSync**

**Rationale:**
- Best of all worlds: smooth 60 FPS, stable frame times, same pathfinding capacity
- VSync becomes viable with consistent 16ms frames
- Scales to future (144Hz monitors)
- Professional solution

**Implementation effort:** ~1-2 weeks
1. Refactor pathfinding queue to process fixed time slice
2. Test with various unit counts (10, 50, 100, 200 units)
3. Verify multiplayer behavior
4. Tune budget values (3ms may need adjustment)

### Long-Term (Optional Polish)
**Option 6: Hybrid with multiple quality presets**

**Rationale:**
- Maximum flexibility
- Professional polish
- Marketing differentiator

**Implementation effort:** ~1 week after Option 5
- Add quality dropdown in settings
- Implement presets
- Add tooltips explaining each mode

---

## Recommendation Matrix

| Priority | Use Case | Recommendation |
|----------|----------|----------------|
| 🚀 **NOW** | Single-player power users | **Option 2** (Remove limiter) |
| ⭐ **BEST** | General users | **Option 5** (Multi-frame pathfinding + VSync) |
| 🏆 **IDEAL** | Professional polish | **Option 6** (Hybrid modes) |
| 🛡️ **SAFE** | Multiplayer competitive | Keep **Option 1** as default |

---

## Conclusion

**Frame limiter only affects rendering smoothness, not game logic speed.** The real bottleneck is pathfinding budget per frame. 

**Immediate action:** Make frame limiter truly optional (already done) - lets users with fast GPUs enjoy smoother visuals at their own risk.

**Best solution:** Implement multi-frame pathfinding spread (Option 5) - achieves 60 FPS with no pathfinding capacity loss, making VSync viable.

**Multiplayer safety:** All options are multiplayer-safe because game cycles accumulate real time regardless of rendering rate. Different players can use different frame limiters without breaking synchronization.

---

## Appendix: Technical Details

### Current Code Structure
**Frame limiter location:** `src/Game.cpp` lines 1091-1098  
**Pathfinding budget:** `include/Game.h` line 702  
**Menu limiter:** `src/Menu/MenuBase.cpp` line 82-85  
**Map editor limiter:** `src/MapEditor/MapEditor.cpp` line 135-138

### Files That Need Changes for Option 5

**High-level changes:**
1. `include/Game.h` - Update `PathBudgetMs` constant
2. `src/Game.cpp` - Modify `processPathRequests()` to enforce strict time limit
3. `src/Game.cpp` - Update frame limiter to 16ms
4. `src/Menu/MenuBase.cpp` - Update menu limiter to 16ms
5. `src/MapEditor/MapEditor.cpp` - Update editor limiter to 16ms

**Testing checklist:**
- [ ] Single-player with 10 units
- [ ] Single-player with 100 units
- [ ] Single-player with 200+ units (large battle)
- [ ] Multiplayer 2 players (different hardware)
- [ ] Multiplayer 4 players (stress test)
- [ ] Performance profiling (confirm consistent frame times)
- [ ] VSync enabled/disabled comparison

---

**Document Version:** 1.0  
**Date:** 2025-10-25  
**Author:** Development Team  
**Status:** Analysis Complete - Awaiting Implementation Decision

