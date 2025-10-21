# Problems Found - Analysis and Root Causes

## Executive Summary

After extensive changes (32 files, 2,119 insertions, 611 deletions), the game exhibited critical bugs:
- Units walking through buildings
- Units walking off the map
- Launchers/sonic tanks firing off-screen at phantom targets

**Root Cause**: Wrong approach to "fixing" crashes masked the underlying data corruption issue.

---

## Visual Symptoms Observed

### 1. Units Walking Through Buildings
**What**: QuantBot units (troopers, tanks) visibly move through enemy structures as if they don't exist

**When**: New games (not save corruption)

**Why This Is Critical**: Collision detection is fundamental - this breaks core game mechanics

---

### 2. Units Walking Off Map
**What**: Units move beyond map boundaries into invalid coordinates

**When**: During normal gameplay, especially with AI units

**Why This Is Critical**: Indicates corrupted position data or broken bounds checking

---

### 3. Off-Screen Firing
**What**: Launchers and sonic tanks fire missiles toward targets well beyond weapon range and map boundaries

**When**: During combat, particularly with kiting logic active

**Why This Is Critical**: Game balance broken, visual glitch, indicates targeting corruption

---

## The Investigation Journey

### Phase 1: Initial Symptoms (documents/firing-at-nothing)
- **Crash logs showed**: Iterator invalidation in `Tile::squash()`
- **Stack trace**: `InfantryBase::squash()` → `Tile::squash()` → crash in `std::list`
- **Diagnosis**: Infantry being destroyed while iterating tile's infantry list

### Phase 2: The "Fix" (WRONG APPROACH)
**Change Made**: Rewrote `Tile::squash()` to copy IDs before iterating
```cpp
// Copy IDs to avoid iterator invalidation
std::vector<Uint32> infantryIds;
for (const Uint32 id : assignedInfantryList) {
    infantryIds.push_back(id);
}
// Then iterate over copy
for (const Uint32 id : infantryIds) {
    if (auto* infantry = getObject(id)) {
        infantry->squash();
    }
}
```

**Result**: 
✅ Crashes stopped
❌ **But** real problem was hidden

### Phase 3: New Symptoms Appeared
- Units walking through buildings
- Units walking off map
- Off-screen firing persisted

**Key Insight**: The "fix" prevented crashes but didn't solve WHY destroyed units were still in tile lists!

### Phase 4: Bandaid Fixes
Multiple attempts to treat symptoms:
1. Added off-map target validation
2. Added attackPos validation
3. Added collision detection logging
4. Added movement validation logging

**Result**: None solved the core issue

### Phase 5: Kiting Bug Identified
**Discovery**: Kiting logic for launchers/deviators:
```cpp
doSetAttackMode(pUnit, AREAGUARD);
doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true);
```

**Problem**: `doMove2Pos()` has a side effect - it sets unit's `guardPoint` to the destination!

**Impact**: When target is lost, `releaseTarget()` sends unit back to guard point and it fires there

**This explained**: Off-screen firing (units firing at squad rally locations in empty desert)

---

## Root Causes Analysis

### Root Cause #1: Tile Data Corruption (UNSOLVED)
**The Real Problem**: Destroyed units remain in `assignedInfantryList` with invalid/corrupted data

**Why This Happens** (Hypothesis):
1. Unit is destroyed via `destroy()`
2. Unit should call `unassignInfantry()` to remove itself from tile
3. Something in destruction sequence fails
4. Unit object is deleted but tile still has its ID
5. Tile's `getGroundObject()` returns null or corrupted pointer
6. Collision detection sees "no object" and allows passage

**Evidence**:
- Units walk through buildings (collision check fails)
- Units walk off map (bounds check on corrupted data fails)
- Game doesn't crash (because we masked it with ID copy)

**What We Should Have Done**:
1. Let the crash happen
2. Find WHERE unit destruction fails to unassign
3. Fix the cleanup sequence
4. NOT mask the crash

---

### Root Cause #2: Guard Point Side Effects
**The Problem**: `doMove2Pos()` changes `guardPoint` as a side effect

**Code**:
```cpp
void UnitBase::doMove2Pos(int x, int y, bool force) {
    // ...
    setGuardPoint(x, y);  // SIDE EFFECT!
    // ...
}
```

**Impact**: Kiting logic moves units but also changes where they "return home" to

**Kiting Flow**:
1. Launcher targets enemy
2. Enemy gets within 6 tiles
3. Kiting: `doMove2Pos(squadCenter)` → **guard point now = squad center**
4. Launcher moves away
5. Target is lost or dies
6. `releaseTarget()` called
7. Unit returns to guard point (squad center, which might be in empty desert)
8. Unit fires at that location

**Fix Attempted** (in firing-at-nothing doc):
- Save old guard point before `doMove2Pos()`
- Restore it after
- Didn't work because of other complexities

---

### Root Cause #3: Too Many Changes At Once
**The Problem**: 2,119 lines changed across 32 files simultaneously

**Impact**:
- Can't isolate which change caused which bug
- Bugs interact with each other
- Diagnostic logging everywhere made it HARDER not easier
- Complexity spiral: fix one bug → create another → add more code → more bugs

**Evidence**:
- Kiting bug interacted with target validation
- Tile corruption interacted with collision detection
- Spatial grid changes interacted with tile lists
- Each fix added more complexity

---

### Root Cause #4: Treating Symptoms Not Disease
**Pattern Observed**:
1. Crash in `Tile::squash()` → Fix: copy IDs (WRONG - masked corruption)
2. Off-map firing → Fix: validate targets (WRONG - didn't address guard point)
3. Units through buildings → Fix: add logging (WRONG - didn't fix tile corruption)

**What We Should Have Done**:
- Each symptom pointed to root cause
- Should have investigated WHY before adding fixes
- Should have reverted earlier

---

## Why The Game Kept Running (No Crashes)

The `Tile::squash()` "fix" prevented crashes by:
1. Copying IDs before iterating
2. Checking `if (infantry)` before calling methods
3. Skipping null/invalid pointers

**But This Allowed**:
- Corrupted data to persist
- Collision detection to fail (sees null objects)
- Units to access invalid memory (undefined behavior)
- Game state to become inconsistent

**Result**: Game runs but is fundamentally broken

---

## Lessons Learned

### 1. Don't Mask Crashes - Fix Them
**Crash = Diagnostic Tool**
- Crash tells you WHERE the bug is
- Crash prevents game from running with corrupted state
- "Fixing" the crash without fixing the cause is worse than crashing

### 2. Question Side Effects
**`doMove2Pos()` had hidden side effect**
- Changed guard point without documentation
- Caused kiting to break targeting
- Should have been obvious in retrospect

### 3. One Change At A Time
**Sequential Testing**
- Change one system
- Test thoroughly
- Commit if good, revert if bad
- Then change next system

### 4. Revert Faster
**Don't Dig Deeper When Lost**
- If bugs multiply, revert
- Start over with clean slate
- Reimplement carefully

### 5. Logs Should Be Targeted
**Too Much Logging = No Logging**
- Every system logging made it impossible to find real issues
- Selective logging would have been better
- Performance logging was good (targeted, useful)

### 6. Understand Before Changing
**Read The Code First**
- `doMove2Pos()` implementation wasn't checked before using
- Tile destruction sequence wasn't understood
- Spatial grid lifecycle wasn't clear

---

## What Should Happen Next

### Immediate Action: REVERT
```bash
git reset --hard origin/release-0.98.4
```

**Why**: 
- Clean slate
- Working game
- Can reimplement carefully

### Future Reimplement (If Desired)

#### Phase 1: Infrastructure (Safe Changes)
- Performance monitoring (worked well)
- Stream safety checks (worked well)
- Save/load robustness (worked well)
**Test**: Verify game still works

#### Phase 2: Pathfinding Improvements (Isolated)
- Increase node budget
- Increase time budget
- Queued pathfinding
**Test**: Verify pathfinding works, no other systems affected

#### Phase 3: QuantBot Improvements (Careful)
- Attack squad size removal (simple, safe)
- AI mode detection improvements (simple, safe)
**Test**: Play several games, verify AI works

#### Phase 4: Ornithopter Logic (Complex)
- Implement smart attack decisions
- Test ONLY ornithopter behavior
- Keep simple fallback
**Test**: Focus on ornithopter behavior only

#### Phase 5: Kiting (If Needed)
- Understand guard point system FIRST
- Consider alternative approach (don't use doMove2Pos)
- Or: save/restore guard point correctly
**Test**: Extensive testing with launchers/deviators

#### Never: Tile::squash() "Fix"
- Don't copy IDs to mask crashes
- If crashes return, find where unit cleanup fails
- Fix the cleanup, not the symptom

---

## Summary

**Good Changes That Worked**:
- Performance monitoring ✅
- Pathfinding improvements ✅
- Safety checks (null pointers, overflow) ✅
- Save/load robustness ✅

**Bad Changes That Failed**:
- Tile::squash() ID copying ❌ (masked corruption)
- Kiting with doMove2Pos() ❌ (guard point side effect)
- Too many simultaneous changes ❌ (couldn't debug)
- Excessive logging ❌ (noise not signal)

**Root Issues Unsolved**:
1. Tile data corruption (units in lists after destruction)
2. Spatial grid desync (possibly)
3. Guard point system not understood

**Path Forward**:
- Revert to clean state
- Reimplement incrementally
- Test each change
- Fix root causes not symptoms

---

**Document Complete**: All problems analyzed, root causes identified, lessons learned documented.

**Ready for**: User review and approval to revert.

