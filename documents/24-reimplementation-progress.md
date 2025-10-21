# Reimplementation Progress

This tracks features being reimplemented from the changes analysis (documents/18-changes-analysis.md).

## Completed

### ✅ Change 2.1: Performance Timing System
**Commits**: `332a6c2`, `c5df807`
**What**: Added performance monitoring that logs frame breakdown every 2 seconds
- Tracks time for: Units, Structures, Pathfinding, Rendering, Total
- Uses `SDL_GetPerformanceCounter` for precise timing
- Logs format: `[Performance] Avg frame: 12.34ms | Units: 4.50ms | Structures: 2.10ms | Pathfinding: 1.20ms | Rendering: 4.54ms | Frames: 120`

**Files Modified**:
- `include/Game.h` - Added `FrameTiming` struct, `getElapsedMs()`, `logFrameTiming()`
- `src/Game.cpp` - Added timing instrumentation to `updateGameState()`, `processObjects()`, `renderFrame()`

**Status**: ✅ Compiles and ready to test

---

### ✅ Change 2.2: Pathfinding Budget Increase
**Commit**: `b57716c`
**What**: Increased pathfinding time and node budgets
- `PathBudgetMs`: 3.0ms → 6.0ms (allows more pathfinding per frame)
- `kPathNodeBudget`: Added constant = 2048 nodes (for future use)

**Files Modified**:
- `include/Game.h` - Updated `PathBudgetMs`, added `kPathNodeBudget`

**Notes**: 
- `PathBudgetMs` is actively used by `processPathRequests()`
- `kPathNodeBudget` is defined but not yet wired to AStarSearch (would require Changes 4.1 & 4.2)

**Status**: ✅ Compiles and ready to test

---

## Not Yet Implemented

### ⏳ Change 4.1 & 4.2: AStarSearch Node Budget Support
**Priority**: Low (can use time budget alone)
**What**: 
- Add `nodeBudget` parameter to AStarSearch constructor
- Add `exhaustedNodeBudget()` method
- Use `kPathNodeBudget` constant

**Why Skipped**: More complex change requiring AStarSearch refactoring. Current time-based budget (PathBudgetMs) should be sufficient.

---

## To Discuss With User

Which features should be reimplemented next? Options from analysis doc:
1. **Game Logic Changes** (section: "SUMMARY BY CATEGORY")
   - Ornithopter smart attack system (complex)
   - Kiting logic for launchers/deviators (was buggy before)
   - Attack squad management

2. **Bug Fixes** (section: "Bug Fixes (Attempted)")
   - Save/load robustness improvements
   - Integer overflow prevention
   - Null pointer safety checks

3. **Infrastructure** (section: "Infrastructure")
   - Stream safety improvements
   - Header file additions

**Recommendation**: Start with **Bug Fixes** - they're low-risk, high-value improvements that won't introduce game logic complexity.

