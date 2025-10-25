# Unit Performance Breakdown Analysis
**Date:** 2025-10-25  
**Scenario:** 1,616-1,628 units on map at 16ms game speed (default)

---

## Executive Summary

**FOUND IT! Targeting is the bottleneck, consuming 83% of Units processing time.**

With 1,616 units on map:
- **Total Units time:** 3.45ms average, 12.34ms peak
- **Targeting time:** 2.45ms (82.9%) ← **THE BOTTLENECK**
- **Other operations:** 1.00ms (17.1%)

---

## Detailed Unit Breakdown (Latest Session)

### Session 1: 1,628 units
```
FPS: 41.7 | Frame: 23.92ms

Units: min=1.88ms avg=3.52ms max=7.96ms (1628 units)
  ↳ Targeting:   2.51ms (83.2%)  ← 83% of time!
  ↳ Navigate:    0.07ms (2.3%)
  ↳ Move:        0.25ms (8.5%)
  ↳ Turn:        0.07ms (2.5%)
  ↳ Visibility:  0.09ms (3.2%)
```

### Session 2: 1,616 units
```
FPS: 42.6 | Frame: 23.44ms

Units: min=1.98ms avg=3.45ms max=12.34ms (1616 units)
  ↳ Targeting:   2.45ms (82.9%)  ← Still 83%!
  ↳ Navigate:    0.07ms (2.3%)
  ↳ Move:        0.25ms (8.7%)
  ↳ Turn:        0.07ms (2.6%)
  ↳ Visibility:  0.09ms (3.2%)
```

**Targeting is consistently the dominant cost.**

---

## Component Analysis

### 1. Targeting (82.9% of Units time)

**What it does:** `UnitBase::targeting()` - Find enemies to attack

**Cost per unit:**
```
2.45ms total / 1616 units = 0.00152ms per unit average
```

**But with 1,616 units:**
- Each unit searches for targets
- Likely using spatial grid
- Still adds up to 2.45ms total

**Optimization potential:** 
- Throttle target searches (already has `findTargetTimer`)
- Increase timer frequency
- Use better spatial queries
- Cache target lists

---

### 2. Move (8.7% of Units time)

**What it does:** `UnitBase::move()` - Update unit positions

**Cost:** 0.25ms for 1,616 units = 0.00015ms per unit

**Status:** ✓ Efficient, not a problem

---

### 3. Navigate (2.3% of Units time)

**What it does:** `UnitBase::navigate()` - Follow paths

**Cost:** 0.07ms for 1,616 units = 0.000043ms per unit

**Status:** ✓ Very efficient

---

### 4. Turn (2.6% of Units time)

**What it does:** `UnitBase::turn()` - Rotate units

**Cost:** 0.07ms for 1,616 units = 0.000043ms per unit

**Status:** ✓ Very efficient

---

### 5. Visibility (3.2% of Units time)

**What it does:** `UnitBase::updateVisibleUnits()` - House contact checks

**Cost:** 0.09ms for 1,616 units = 0.000056ms per unit

**Status:** ✓ Efficient

---

## Comparison: Current Session vs Previous 153ms Spike

### Current Performance (Good)
```
1,616 units
Units total: 3.45ms
  Targeting: 2.45ms (83%)
```

### Previous Spike (Bad) - From logs analysis
```
Unknown unit count (estimated 200-300?)
Units total: 153ms
  Unknown breakdown
```

**Questions:**
1. Why was Units 153ms before but only 3.45ms now?
2. Was the earlier measurement during a huge battle spike?
3. Was there a pathfinding death spiral inflating the numbers?

---

## Current Frame Budget Analysis

**Target: 60 FPS = 16.67ms per frame**

**Current breakdown (42.6 FPS = 23.44ms):**
```
Rendering:     2.89ms  (12%) ✓ Great
Units:         3.45ms  (15%) ✓ Acceptable
Structures:    0.31ms  (1%)  ✓ Great
AI:            0.40ms  (2%)  ✓ Great
Pathfinding:  15.68ms (67%) ← BOTTLENECK NOW
Other:         0.71ms  (3%)
--------------
TOTAL:        23.44ms (vs 16.67ms target)
```

**Overage: 23.44ms - 16.67ms = 6.77ms (29% over budget)**

---

## The Real Bottleneck Now: Pathfinding!

**Pathfinding is now the #1 problem:**
- Budget: 12ms
- Actual: 15.68ms average, 21.88ms peak
- **Exceeding budget by 31% (average) to 82% (peak)**

**Why:**
- Budget enforced per frame, but individual paths can exceed budget
- 6.9 paths/frame processed
- 2.25ms per path average
- Some paths take much longer (causing 21.88ms peak)

---

## Recommendations

### Priority 1: Reduce Pathfinding Budget for 60 FPS

**Change from 12ms → 6ms:**

```cpp
// include/Game.h
static constexpr double PathBudgetMs = 6.0;  // Reduced for 60 FPS
```

**Expected impact:**
```
Frame time: 23.44ms - 9.68ms (saved from pathfinding) = 13.76ms
FPS: 72 FPS (if achievable)
```

**Trade-off:** Fewer paths processed per frame (3-4 instead of 6-7)
- Path queue may grow during heavy combat
- Units may wait 1-2 extra frames for paths
- Acceptable for strategy game

---

### Priority 2: Enforce Strict Pathfinding Budget

**Current issue:** Individual paths can blow the budget

**Solution:** Implement per-path time limit

```cpp
void Game::processPathRequests() {
    const double maxPathTime = 2.0; // 2ms max per path
    
    while(!pathRequestQueue.empty()) {
        Uint64 pathStart = SDL_GetPerformanceCounter();
        
        // Process one path...
        
        double pathTime = getElapsedMs(pathStart, SDL_GetPerformanceCounter());
        
        // If this path took too long, break
        if(pathTime > maxPathTime) {
            SDL_Log("Warning: Path took %.2fms (over 2ms limit)", pathTime);
            break; // Defer remaining paths to next frame
        }
        
        // Check frame budget
        if(/* budget exceeded */) break;
    }
}
```

---

### Priority 3: Optimize Targeting (Future)

**Currently:** 2.45ms for 1,616 units = 10.5% of 60 FPS budget

**Potential optimizations:**
1. Increase `findTargetTimer` intervals
2. Batch target finding (process N units per frame)
3. Cache nearby enemy lists
4. Better spatial grid queries

**But:** Targeting is already pretty efficient at 0.00152ms per unit. Not urgent.

---

## Can We Hit 60 FPS?

**Current frame budget:**
```
Rendering:     2.89ms
Units:         3.45ms
Structures:    0.31ms
AI:            0.40ms
Pathfinding:   6.00ms (if we reduce budget)
Other:         0.71ms
--------------
TOTAL:        13.76ms < 16.67ms ✓ YES!
```

**Answer: YES, if we reduce pathfinding budget to 6ms!**

**Trade-offs:**
- Path processing rate: 6.9 paths/frame → ~3-4 paths/frame
- During heavy combat with many path requests, queue will grow
- Units may wait 2-3 frames for paths instead of 1 frame
- **Acceptable for RTS game**

---

## Mystery: What Happened to the 153ms Units Time?

**Earlier logs showed:** Units = 153ms

**Current logs show:** Units = 3.45ms (44x less!)

**Possible explanations:**

1. **Different scenarios:**
   - Earlier: Huge battle with concentrated units (O(n²) interactions?)
   - Current: Units spread out across map

2. **Measurement error:**
   - Earlier measurement included multiple cycles per frame?
   - Frame timing bug?

3. **Code changes:**
   - Something we changed inadvertently fixed it?

4. **Pathfinding death spiral:**
   - Earlier: 72 cycles per frame × units processing = inflated number
   - Current: 1.1 cycles per frame (normal)

**Need more data:** Run a huge battle scenario and check if Units spikes to 153ms again.

---

## Action Items

**Immediate (for 60 FPS):**
1. ✅ Change `PathBudgetMs = 12.0` → `PathBudgetMs = 6.0`
2. Test with heavy combat scenarios
3. Monitor path queue size

**Short-term (robustness):**
1. Implement per-path time limit (2ms max)
2. Add path queue size logging
3. Test worst-case scenarios (200+ units concentrated in one area)

**Medium-term (polish):**
1. Optimize targeting if it becomes an issue
2. Consider spreading target finding across frames
3. Add configurable pathfinding budget to INI file

---

## Bottom Line

**FOUND THE BOTTLENECK: Pathfinding (15.68ms) is now the limiter, not Units (3.45ms).**

**Targeting takes 83% of Units time, but Units is only 15% of frame time.**

**Reduce pathfinding budget from 12ms → 6ms to hit 60 FPS.**

**The 153ms Units spike is a mystery - may have been a death spiral scenario.**

