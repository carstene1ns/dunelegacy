# Document 71: QuantBot Multiplayer Desync Analysis

**Version:** 0.98.6.3+  
**Date:** October 26, 2025  
**Type:** Critical Multiplayer Bug Analysis

## Problem

Multiplayer games desync when QuantBot AI is present. Players remain connected but see different game states.

## Investigation Results

### ✅ Network Synchronization (WORKING)

The game uses a **deterministic lockstep** network model:
- All clients execute commands in the same order at the same game cycle
- Game halts if any client hasn't sent their commands yet
- Commands are sent via `CommandManager` with `networkCycleBuffer` lookahead
- The network synchronization itself is working correctly

### ✅ Random Number Generation (WORKING)

- QuantBot uses `getRandomGen()` which returns `currentGame->randomGen`
- This is a shared deterministic Linear Congruential Generator
- All clients share the same random seed and call sequence
- Random number generation is correctly synchronized

### ✅ Object Iteration (SAFE)

- `getStructureList()` and `getUnitList()` return `RobustList` (linked list)
- Maintains insertion order, which should be deterministic
- All clients create objects in the same order via synchronized commands
- Iteration order is deterministic

---

## 🔴 CRITICAL ISSUE FOUND: Wall-Clock Time in Game Logic

### Location: `src/players/QuantBot.cpp` lines 480-510

```cpp
// Log military stats every 30 seconds
static Uint32 lastMilitaryLogTime = 0;
const Uint32 currentTime = SDL_GetTicks();
if(lastMilitaryLogTime == 0) {
    lastMilitaryLogTime = currentTime;
} else if(currentTime - lastMilitaryLogTime >= 30000) {
    SDL_Log("[QuantBot %s] ========== MILITARY STATUS ==========", ...);
    // ... extensive logging ...
    lastMilitaryLogTime = currentTime;
}

checkAllUnits();

if (buildTimer <= 0) {
    build(militaryValue);
}
```

### Why This Causes Desync

**SDL_GetTicks() returns wall-clock time, NOT game time!**

In multiplayer:
1. **Client A** runs at 60fps, smooth performance
2. **Client B** runs at 30fps, slower machine
3. Both clients are at game cycle 15000

**Client A (fast):**
- Real time elapsed: 10 minutes (30 seconds per 9000 cycles)
- `currentTime - lastMilitaryLogTime >= 30000` → **TRUE**
- Executes logging code
- Calls `checkAllUnits()` and `build()` after the logging path

**Client B (slow):**
- Real time elapsed: 15 minutes (45 seconds per 9000 cycles)
- `currentTime - lastMilitaryLogTime >= 30000` → **TRUE** (but at different game cycle!)
- Executes logging code at a **different game cycle** than Client A

**Result:**
- The conditional check evaluates differently based on **machine performance**
- Different clients execute the logging code on different game cycles
- Even though the logging itself doesn't affect game state, the timing difference can cause:
  - Different execution timing of subsequent code
  - Potential cache effects
  - Compiler optimization differences in code paths

### Actual Impact

While the logging itself is harmless, the problem is:

1. **Static variable `lastMilitaryLogTime`** is updated at different game cycles on different clients
2. The condition uses **wall-clock time** instead of **game cycles**
3. This creates **non-deterministic code paths** based on client performance

Even if this specific logging doesn't directly cause desync, it violates the fundamental rule:

> **ALL clients must execute IDENTICAL code paths in IDENTICAL order every game cycle**

### Why This Might Not Cause Immediate Desync

The logging code doesn't:
- ❌ Increment any game state counters
- ❌ Call the random number generator
- ❌ Make any game decisions
- ❌ Modify any game objects

So it might not cause desync... **unless**:
- ⚠️ Compiler optimizations differ based on code paths
- ⚠️ Cache behavior changes execution timing
- ⚠️ Future changes add state modifications inside the conditional

---

## 🟡 POTENTIAL ISSUE: Race Conditions (LOW RISK)

### QuantBot Random Number Usage

QuantBot calls `getRandomGen().rand()` in three places, all in the `build()` function:

```cpp
// Line 129 (constructor):
buildTimer = getRandomGen().rand(0, 3) * 50;

// Lines 1632, 1892 (build function):
buildTimer = getRandomGen().rand(0, 3) * 5;
```

These calls happen:
- After the SDL_GetTicks() conditional (safe for now)
- In deterministic code paths
- **UNLESS** future code adds random calls inside the conditional logging block

### Risk Assessment

**Current status: LOW RISK** ✅  
- Random calls are after the conditional
- All clients should call `rand()` in the same order

**Future risk: MEDIUM** ⚠️  
- If anyone adds a random call inside the logging conditional
- If the conditional affects control flow before random calls
- If performance differences cause frame-based timing issues

---

## The Fix

### Solution: Replace Wall-Clock Time with Game Cycles

```cpp
// BEFORE (BROKEN):
static Uint32 lastMilitaryLogTime = 0;
const Uint32 currentTime = SDL_GetTicks();
if(lastMilitaryLogTime == 0) {
    lastMilitaryLogTime = currentTime;
} else if(currentTime - lastMilitaryLogTime >= 30000) {
    // Log stuff
    lastMilitaryLogTime = currentTime;
}

// AFTER (FIXED):
static Uint32 lastMilitaryLogCycle = 0;
const Uint32 currentCycle = getGameCycleCount();
const Uint32 LOG_INTERVAL = MILLI2CYCLES(30000); // 30 seconds in game cycles

if(lastMilitaryLogCycle == 0) {
    lastMilitaryLogCycle = currentCycle;
} else if(currentCycle - lastMilitaryLogCycle >= LOG_INTERVAL) {
    // Log stuff
    lastMilitaryLogCycle = currentCycle;
}
```

### Benefits

✅ **Perfectly deterministic** - all clients log at same game cycle  
✅ **Frame-rate independent** - doesn't matter how fast clients run  
✅ **Consistent with existing code** - QuantBot already uses `MILLI2CYCLES()` elsewhere  
✅ **No performance impact** - same logic, just using game time instead of wall time  

---

## Prevention Rules (From Document 69)

**To prevent future desync bugs:**

### 🔴 NEVER
1. ❌ Use `SDL_GetTicks()` or `SDL_GetPerformanceCounter()` in game logic
2. ❌ Use wall-clock time for game decisions
3. ❌ Increment counters conditionally based on anything except game state
4. ❌ Branch on performance-dependent conditions

### 🟢 ALWAYS
1. ✅ Use `getGameCycleCount()` for all timing
2. ✅ Use `MILLI2CYCLES()` to convert time to game cycles
3. ✅ Ensure all game logic is frame-rate independent
4. ✅ Test multiplayer on machines with different performance

---

## Testing Checklist

**After applying the fix:**

### Basic Test (15+ minutes)
1. Start 2-player game with QuantBot AI
2. Ensure one client runs slower (background processes, lower specs)
3. Play for 15-20 minutes
4. Verify no desync (units in same positions on both clients)

### Stress Test (30+ minutes)
1. Start 4-player FFA with multiple QuantBot instances
2. Play for 30+ minutes
3. Verify game states remain synchronized

### Different Frame Rates
1. Force different frame rates on different clients
2. Client A: 60fps (normal)
3. Client B: 30fps (limited)
4. Verify synchronization maintained

---

## Related Issues

Similar issues fixed in:
- **Document 69**: Combat stats causing desync (conditional counters)
- **Document 70**: Multiplayer safety audit (SDL_GetPerformanceCounter usage)

All these bugs share the same root cause:
> **Non-deterministic code execution based on local machine state**

---

## Summary

**Root Cause:** QuantBot uses `SDL_GetTicks()` (wall-clock time) instead of `getGameCycleCount()` (game time) for logging interval  

**Impact:** Different clients execute conditional code at different game cycles based on machine performance  

**Severity:** CRITICAL (violates multiplayer synchronization requirements)  

**Fix Complexity:** TRIVIAL (3 line change)  

**Fix Risk:** ZERO (only changes timing of debug logging)  

**Status:** ⏳ **FIX NEEDED**

---

## Technical Notes

### Why Wall-Clock Time Breaks Multiplayer

In a lockstep multiplayer game:
- All clients must execute **identical instructions** on **identical game cycles**
- Wall-clock time is **local to each machine** (different for each client)
- Game cycles are **synchronized across all clients** (same for everyone)

Using wall-clock time means:
```
Game Cycle 1000:
  Client A: wallTime = 5000ms  → condition TRUE
  Client B: wallTime = 4500ms  → condition FALSE
```

Different branches = different code paths = **desync risk**

### Why Game Cycles Are Safe

Using game cycles means:
```
Game Cycle 1000:
  Client A: gameCycle = 1000  → condition TRUE
  Client B: gameCycle = 1000  → condition TRUE
```

Same branches = same code paths = **perfect sync**

---

## Implementation

See next commit for the fix.


