# Recent Changes Analysis - Finding the Targeting Bug

## Last 6 Commits

### 6ac9d82 - Software FPS Limiter
**Files**: `src/Game.cpp`
**Change**: Added `SDL_Delay()` to cap at 60 FPS
**Impact**: Timing only, NO effect on targeting

### 743f2d7 - Tile::squash() Iterator Fix (REVERTED)
**Files**: `src/Tile.cpp`
**Change**: Copy infantry IDs before squashing
**Impact**: Infantry squashing only, NO effect on targeting
**Status**: **REVERTED for testing**

### 1a6149c - VSync Disable
**Files**: `src/main.cpp`
**Change**: `SDL_HINT_RENDER_VSYNC` "1" → "0"
**Impact**: Rendering only, NO effect on targeting

### e528a60 - Carryall Crash + Turret Placement
**Files**: `src/units/UnitBase.cpp`, `src/players/QuantBot.cpp`
**Changes**:
1. **UnitBase.cpp**: Added null check before `unBook()` on repair yard
   - Line 1149: `if(goingToRepairYard && target.getObjPointer() != nullptr)`
   - NO effect on targeting
2. **QuantBot.cpp**: Revised `findTurretPlaceLocation()` logic
   - Building placement only, NO effect on targeting

### b8b1a23 - Remove Attack Squad Limit
**Files**: `src/Game.cpp`, `src/players/QuantBot.cpp`
**Changes**:
1. **Game.cpp**: Fixed FPS display calculation
   - NO effect on targeting
2. **QuantBot.cpp**: Removed `maxAttackSquadSize = 70` limit
   - Line 1997: Removed `if (attackSquadSize >= maxAttackSquadSize)` check
   - MORE units sent to HUNT mode
   - **POTENTIAL CULPRIT**: More units = more targeting = exposes bugs?

### 734c635 - Path Optimization (ONE BEFORE RECENT)
**Files**: `src/units/UnitBase.cpp`, `src/players/QuantBot.cpp`, Xcode project
**Changes**:
1. **UnitBase.cpp** (lines 476-497): Path recalculation threshold
   ```cpp
   if(movementDistance > 3) {
       clearPath(); // Stop and wait
   } else if(movementDistance > 1) {
       // Keep moving, queue new path
       destination = targetLocation;
       enqueuePathRequest();
   } else {
       // Minor movement, just update
       destination = targetLocation;
   }
   ```
   - Changed from "> 1 tile" to "> 3 tiles" for path clear
   - Units now keep following old path when target moves 1-3 tiles
   - **POTENTIAL CULPRIT**: Could cause units to path to old target location?

2. **QuantBot.cpp**: Turret placement + ornithopter counter
   - NO effect on unit targeting

3. **Xcode project**: Debug optimization `-O0` → `-O1`
   - NO effect on targeting

## Most Likely Culprits

### 1. Path Recalculation Change (734c635) ⭐⭐⭐
**File**: `src/units/UnitBase.cpp:476-497`

**What changed**:
- BEFORE: Clear path if target moves > 1 tile
- AFTER: Clear path only if target moves > 3 tiles
- For 1-3 tile movement: keep old path, update destination, queue new path

**How this could cause "firing at nothing"**:
1. Unit targets enemy infantry at position A
2. Infantry dies/destroyed
3. `destination` gets updated to last known position
4. Unit keeps moving toward old destination (now empty)
5. Unit arrives, fires at empty tile where infantry used to be

**BUT**: This doesn't explain "off-screen" firing or "long distance" issues

### 2. Attack Squad Limit Removal (b8b1a23) ⭐⭐
**File**: `src/players/QuantBot.cpp:1997`

**What changed**:
- BEFORE: Only 70 units in HUNT mode
- AFTER: ALL units in HUNT mode (unlimited)

**How this could expose bugs**:
- More units = more targeting calculations
- More simultaneous target acquisitions
- Could expose race conditions or stale target bugs
- More load on targeting system reveals weaknesses

**BUT**: Doesn't change targeting logic itself

### 3. Something Else ⭐
**Possibility**: Bug exists in remote branch (`origin/release-0.98.4`)
- Never noticed before
- More visible now with increased unit counts
- Recent changes expose pre-existing issue

## What We Know

**Symptoms**:
- Units firing at nothing
- Missiles flying off the map
- Firing at long distances
- Firing at buildings/empty locations

**When it started**:
- "Only in the last 3 commits or so"
- After commit 734c635 or later

**What we've ruled out**:
- ✅ Tile::squash() (reverted, still broken)
- ✅ VSync changes (rendering only)
- ✅ FPS limiter (timing only)
- ✅ Carryall null check (unrelated)
- ✅ Turret placement (building placement)

## Testing Plan

### Test 1: Revert Path Recalculation Change
**Target**: Commit 734c635, file `src/units/UnitBase.cpp`
**Revert**: Path optimization (lines 476-497)
**Expected**: If targeting bug goes away, this is the culprit

### Test 2: Revert Attack Squad Limit Removal
**Target**: Commit b8b1a23, file `src/players/QuantBot.cpp`
**Revert**: Restore `maxAttackSquadSize = 70` limit
**Expected**: If targeting bug goes away, it's a load/scaling issue

### Test 3: Full Revert to Before 734c635
**Target**: Reset HEAD to 734c635~1
**Expected**: Confirm if bug existed before all recent changes

## Hypothesis: Path Recalculation Is The Culprit

**The bug mechanism**:
1. Unit locks onto target (e.g., infantry, building)
2. Target moves or is destroyed
3. `destination` updated to last known position
4. Path recalculation logic keeps unit moving toward old destination
5. Unit tries to fire at old destination even though target is gone

**Evidence**:
- Only code that touches movement toward targets
- Changed behavior of how units track moving/destroyed targets
- Could cause units to path to invalid locations

**Next step**: Revert the path recalculation change and test

## Code to Revert (Test 1)

**File**: `src/units/UnitBase.cpp:476-497`

**Revert this:**
```cpp
if(movementDistance > 3) {
    clearPath();
} else if(movementDistance > 1) {
    destination = targetLocation;
    if(!pathRequestQueued) {
        enqueuePathRequest();
    }
} else {
    destination = targetLocation;
}
```

**Back to this:**
```cpp
if(movementDistance > 1) {
    clearPath();
} else {
    destination = targetLocation;
}
```

## Recommendation

**First**: Revert the path recalculation change (734c635)
**Then**: Test if targeting bug is fixed
**If not**: Revert attack squad limit removal (b8b1a23)
**If still not**: Full git bisect between origin/release-0.98.4 and HEAD

