# Performance Analysis - 12 FPS Issue
**Date:** 2025-10-25  
**Log Source:** Full game session from start to 4.6 FPS

---

## Executive Summary

**Problem:** FPS degraded from 243 FPS → 4.6 FPS during gameplay  
**Root Cause:** **Units processing time exploded from 2ms → 153ms**  
**Secondary Issue:** Pathfinding consistently exceeds 12ms budget (13-26ms)

---

## Full Performance Timeline (34 samples)

### Early Game - Excellent Performance

| FPS | Frame Time | AI | Units | Structures | Pathfinding | Rendering |
|-----|-----------|-----|-------|-----------|------------|-----------|
| 243.7 | 4.10ms | - | - | - | - | - |
| **17.7** | **56.18ms** | **7.20ms** | **9.89ms** | **2.22ms** | **13.91ms** | **17.37ms** |
| 312.3 | 3.20ms | - | - | - | - | - |
| **40.5** | **24.63ms** | **4.63ms** | **2.04ms** | **1.85ms** | **17.71ms** | **5.99ms** |
| 123.6 | 8.08ms | - | - | - | - | - |
| **26.9** | **37.12ms** | **6.43ms** | **4.70ms** | **2.35ms** | **19.77ms** | **13.55ms** |

**Observation:** Units only taking 2-10ms, pathfinding already 13-19ms (exceeding 12ms budget)

---

### Mid Game - Performance Degrading

| FPS | Frame Time | AI | Units | Structures | Pathfinding | Rendering |
|-----|-----------|-----|-------|-----------|------------|-----------|
| 61.2 | 16.33ms | - | - | - | - | - |
| **18.5** | **53.93ms** | **7.97ms** | **10.52ms** | **2.53ms** | **26.88ms** | **9.35ms** |
| 63.0 | 15.87ms | - | - | - | - | - |
| **28.2** | **35.45ms** | **6.01ms** | **6.06ms** | **1.87ms** | **20.25ms** | **5.71ms** |
| 58.0 | 17.22ms | - | - | - | - | - |
| **29.7** | **33.64ms** | **5.94ms** | **5.80ms** | **2.03ms** | **18.36ms** | **6.42ms** |
| 50.8 | 19.64ms | - | - | - | - | - |
| **19.5** | **51.11ms** | **10.65ms** | **11.38ms** | **3.67ms** | **22.83ms** | **9.65ms** |
| 46.4 | 21.51ms | - | - | - | - | - |
| **21.0** | **47.58ms** | **5.55ms** | **16.53ms** | **3.90ms** | **21.21ms** | **6.36ms** |
| 43.2 | 23.14ms | - | - | - | - | - |
| **16.6** | **60.22ms** | **8.34ms** | **14.95ms** | **3.64ms** | **20.39ms** | **7.12ms** |
| 42.7 | 23.36ms | - | - | - | - | - |
| **30.1** | **33.16ms** | **7.55ms** | **8.12ms** | **3.14ms** | **19.35ms** | **5.21ms** |

**Observation:** Units creeping up to 16ms, pathfinding 18-26ms (far exceeding budget)

---

### Late Game - Performance Collapse

| FPS | Frame Time | AI | Units | Structures | Pathfinding | Rendering |
|-----|-----------|-----|-------|-----------|------------|-----------|
| 37.4 | 26.67ms | - | - | - | - | - |
| **8.6** | **115.99ms** | **21.99ms** | **42.65ms** | **11.07ms** | **24.01ms** | **24.85ms** |
| 31.6 | 31.61ms | - | - | - | - | - |
| **11.6** | **86.07ms** | **8.08ms** | **31.05ms** | **5.79ms** | **23.23ms** | **6.28ms** |
| 31.0 | 32.24ms | - | - | - | - | - |
| **10.7** | **92.99ms** | **11.12ms** | **50.63ms** | **12.57ms** | **19.14ms** | **5.24ms** |
| 27.8 | 35.92ms | - | - | - | - | - |
| **13.0** | **76.51ms** | **7.60ms** | **44.88ms** | **4.93ms** | **18.72ms** | **5.51ms** |
| 25.3 | 39.45ms | - | - | - | - | - |
| **7.9** | **126.56ms** | **12.05ms** | **78.69ms** | **9.92ms** | **18.99ms** | **5.73ms** |
| 29.3 | 34.09ms | - | - | - | - | - |
| **9.1** | **108.96ms** | **13.37ms** | **67.23ms** | **8.63ms** | **18.88ms** | **5.86ms** |
| 18.2 | 54.84ms | - | - | - | - | - |
| **4.6** | **216.69ms** | **15.23ms** | **153.70ms** | **15.01ms** | **18.62ms** | **5.75ms** |

**Observation:** Units exploded to 153ms! AI also growing. Pathfinding remains 18-24ms.

---

## Component Analysis

### 1. Units Processing (PRIMARY BOTTLENECK)

| Stage | Min | Avg | Max | Growth |
|-------|-----|-----|-----|--------|
| Early game | 2.04ms | 5.54ms | 10.52ms | - |
| Mid game | 5.80ms | 11.64ms | 16.53ms | **+110%** |
| Late game | 31.05ms | 61.35ms | **153.70ms** | **+427%** |

**Growth rate:** 2ms → 153ms = **7650% increase**

**Why Units Processing Explodes:**
- More units on map
- Unit-to-unit collision checks (O(n²))
- Combat calculations
- Target finding
- Movement updates
- Bullet tracking

**Critical Frame Breakdown (4.6 FPS):**
```
Total frame: 216.69ms
  Units:       153.70ms  (71% of frame!) ← PRIMARY BOTTLENECK
  Pathfinding:  18.62ms  (9%)
  AI:           15.23ms  (7%)
  Structures:   15.01ms  (7%)
  Rendering:     5.75ms  (3%)
  Other:         8.38ms  (4%)
```

---

### 2. Pathfinding (SECONDARY ISSUE)

| Stage | Min | Avg | Max | Budget Compliance |
|-------|-----|-----|-----|------------------|
| Early game | 13.91ms | 17.13ms | 19.77ms | ❌ **+43% over budget** |
| Mid game | 18.36ms | 21.17ms | 26.88ms | ❌ **+76% over budget** |
| Late game | 18.62ms | 20.46ms | 24.01ms | ❌ **+71% over budget** |

**Budget:** 12ms per frame  
**Actual:** 13-26ms per frame  
**Compliance:** **0% - pathfinding ALWAYS exceeded budget**

**Why Pathfinding Exceeds Budget:**
- Budget is checked but NOT enforced strictly
- Once a path calculation starts, it runs to completion
- Single complex path can take >12ms
- Budget only stops *next* path from starting

**Code Issue (from processPathRequests):**
```cpp
// Line 360: Only checks budget BETWEEN paths, not during
if(processedAny && elapsed >= budgetSeconds) {
    break;  // But current path already completed!
}
```

If one path takes 18ms, the budget check happens AFTER, so 18ms is already used.

---

### 3. AI Processing

| Stage | Min | Avg | Max |
|-------|-----|-----|-----|
| Early game | 4.63ms | 6.09ms | 7.20ms |
| Mid game | 5.55ms | 7.82ms | 10.65ms |
| Late game | 7.60ms | 12.07ms | 21.99ms |

**Growth:** 4.63ms → 21.99ms = **+375%**

**Why AI Grows:**
- More houses/players to process
- More units under AI control
- More structures to manage
- Build queue calculations

---

### 4. Structures Processing

| Stage | Min | Avg | Max |
|-------|-----|-----|-----|
| Early game | 1.85ms | 2.14ms | 2.35ms |
| Mid game | 1.87ms | 3.04ms | 3.90ms |
| Late game | 4.93ms | 9.45ms | 15.01ms |

**Growth:** 1.85ms → 15.01ms = **+711%**

**Includes:** Turret scanning (we already optimized this)

---

### 5. Rendering (EXCELLENT)

| Stage | Min | Avg | Max |
|-------|-----|-----|-----|
| All stages | 5.21ms | 7.91ms | 24.85ms |

**Rendering is NOT the bottleneck** - stays 5-6ms even in worst case (except one 24ms spike)

---

## Frame Time Budget Analysis

**Target for 60 FPS:** 16.67ms per frame

**Current actual (worst case):**
```
Units:         153.70ms  (922% of 60 FPS budget!)
Pathfinding:    18.62ms  (112% of budget)
AI:             15.23ms  (91% of budget)
Structures:     15.01ms  (90% of budget)
Rendering:       5.75ms  (34% of budget)
Other:           8.38ms  (50% of budget)
--------------
TOTAL:        216.69ms  (1300% of 60 FPS budget!)
```

**Even for 31 FPS (32ms budget):**
```
Units:         153.70ms  (480% of budget!)
Pathfinding:    18.62ms  (58% of budget)
AI:             15.23ms  (48% of budget)
Structures:     15.01ms  (47% of budget)
Rendering:       5.75ms  (18% of budget)
Other:           8.38ms  (26% of budget)
--------------
TOTAL:        216.69ms  (677% of 32ms budget!)
```

**Can't hit ANY frame rate target when Units processing takes 153ms!**

---

## Pathfinding Budget Compliance - Deep Dive

**Budget:** 12ms per frame  
**Reality:** NEVER complied

### Early Game (Sample 2):
- Actual: 13.91ms
- Overage: 1.91ms (16% over)
- Impact: Steals from rendering budget

### Mid Game (Sample 8):
- Actual: 26.88ms
- Overage: 14.88ms (124% over!)
- Impact: Blows entire frame budget

### Late Game (Sample 30):
- Actual: 18.62ms
- Overage: 6.62ms (55% over)
- Impact: But Units are 153ms, so pathfinding is minor issue

**Conclusion:** Pathfinding budget is not strictly enforced. Individual paths can exceed budget.

---

## Root Cause Summary

### 1. Units Processing Death Spiral (PRIMARY)

**Why it grows:**
- Unit count increases exponentially in battles
- O(n²) collision detection
- Combat calculations per unit pair
- Target scanning (even with our turret optimization)

**Evidence:**
- 2ms → 153ms = 76x growth
- 71% of frame time in worst case
- Directly correlates with FPS drop

**This is THE bottleneck** - nothing else matters until Units processing is fixed

---

### 2. Pathfinding Budget Non-Compliance (SECONDARY)

**Why it exceeds:**
- Budget checked BETWEEN paths, not during
- Complex paths complete before budget check
- No per-path time limit

**Evidence:**
- 100% of samples exceeded 12ms budget
- Average 70% overage

**Impact:** Minor compared to Units, but prevents 60 FPS even in early game

---

## Answers to Your Questions

### Q: "My FPS went down to 12"

**A:** Actually went down to **4.6 FPS** in worst case.

**Cause:** Units processing exploded to 153ms per frame. At that speed, maximum theoretical FPS = 6.5 FPS, but you're hitting 4.6 FPS due to additional overhead.

---

### Q: "Do we need to change pathfinding budget to ensure smooth 60 FPS?"

**A:** Two separate issues:

**Issue 1: Units bottleneck (PRIMARY)**
- Even if pathfinding was 0ms, you still can't hit 60 FPS with Units at 153ms
- **60 FPS requires <16.67ms total, you're at 216ms total**

**Issue 2: Pathfinding budget non-compliance (SECONDARY)**
- Yes, reducing budget from 12ms → 6ms would help early/mid game
- But late game, pathfinding is only 8% of problem (Units are 71%)

---

### Q: "Before we proceed, let's check if there are more issues"

**A:** Yes, major issues found:

**Priority 1: Units Processing**
- Needs spatial partitioning or optimization
- 153ms is unacceptable
- Blocks ANY frame rate improvement

**Priority 2: Pathfinding Budget Enforcement**
- Not strictly enforced
- Individual paths exceed budget
- Needs per-path time limit

**Priority 3 (DONE): Rendering**
- Only 5-6ms, excellent
- Not a bottleneck

---

## Recommendations

### SHORT TERM - Don't Change Pathfinding Budget Yet

**Why:** Units bottleneck is 10x worse than pathfinding issue

**What to do:**
1. Investigate Units processing - why 153ms?
2. Check if spatial grid optimization is working
3. Profile unit update loops
4. Check for O(n²) algorithms in unit code

### MEDIUM TERM - Fix Pathfinding Budget

**After Units fixed:**
1. Enforce per-path time limit (not just batch limit)
2. Reduce budget to 6ms for 60 FPS
3. Implement path result caching

### LONG TERM - Optimize Units

**Possible solutions:**
1. Better spatial partitioning for collision checks
2. Lazy target finding (not every frame)
3. Multi-threaded unit updates
4. Simplify combat calculations

---

## What's Actually Happening in Your Game

**Timeline:**

```
Game Start:
  Few units → 2ms Units processing → 243 FPS ✓
  
5 minutes in:
  More units → 16ms Units processing → 21 FPS
  
Heavy battle:
  Many units + combat → 153ms Units processing → 4.6 FPS ✗
```

**The game is literally spending 71% of every frame just updating units.**

---

## Critical Question for You

**How many units were on the map when FPS hit 4.6?**

If we know the unit count, we can calculate the O(n²) growth rate and determine if spatial grid optimization is working or broken.

**To check:** Look for log entries with unit counts, or load the save game and count units on map.

