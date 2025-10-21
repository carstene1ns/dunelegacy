# Off-Screen Firing Bug - Investigation Summary

## Quick Reference
- **Problem:** Launchers and sonic tanks fire at phantom targets 4-5× beyond weapon range, off the map
- **When it started:** After introducing spatial grid and modifying QuantBot attack algorithm
- **Status:** Diagnostic logging added + defensive fixes applied
- **Next Step:** Build and test to identify root cause

## What I Found

### Critical Issue #1: HUNT Mode Bypasses All Range Checks
**Location:** `src/units/UnitBase.cpp:1023-1025`

```cpp
case HUNT: {
    return true;  // ← ALWAYS returns true, regardless of distance!
} break;
```

**Impact:** 
- Units in HUNT mode never release targets based on range
- `isInAttackRange()` returns true even if target is across the map
- The only protection is the weapon range check at line 499 in `engageTarget()`

**Why this matters:**
- QuantBot sets all attack squad units to HUNT mode
- If they acquire a target at any distance, they won't let go
- They'll chase it forever unless it dies or goes off-map

### Critical Issue #2: No Distance Validation in Target Acquisition
**Location:** `src/ObjectBase.cpp:494-627` (`findTargetViaGrid`)

**Problem:**
- In HUNT mode, `searchRadius = maxReachableRadius` (entire grid)
- Can find and acquire targets anywhere on the map
- No maximum distance sanity check
- A unit at (10,10) can acquire a target at (120,120) - distance ~155 tiles

**Why this matters:**
- Spatial grid legitimately finds distant targets
- HUNT mode says "yes, attack it"
- Unit tries to close distance but keeps firing if in correct angle

### Issue #3: AttackPos Has No Ongoing Validation
**Location:** `src/units/UnitBase.cpp:541-561`

**Problem:**
- `attackPos` validated when SET (line 865)
- Never re-validated during engagement
- If unit moves or map changes, attackPos might become invalid

**Why this matters:**
- If something sets attackPos to an edge tile, it stays forever
- Unit will try to attack it even if now out of range or off-map

### Issue #4: No Protection Against Extreme Distances
**Location:** Multiple locations

**Problem:**
- `engageTarget()` checks `targetDistance > getWeaponRange()` at line 499
- But for HUNT mode, units just keep moving toward target
- No "this target is impossibly far away, give up" logic

**Why this matters:**
- Units can be chasing targets 100+ tiles away
- They fire whenever they're facing the right direction
- Looks like they're shooting at nothing because target is so far away

## Fixes Applied

### Fix #1: Added Excessive Distance Check (Line 514-520)
```cpp
if(targetDistance > getWeaponRange() * 5) {
    SDL_Log("[DIAG] Unit %u releasing target %u: excessive distance...");
    releaseTarget();
    return;
}
```

**What it does:**
- Forces units to release targets that are > 5× weapon range away
- Prevents chasing targets across the entire map
- Should stop most off-screen firing

### Fix #2: Added Target Location Validation (Line 485-492)
```cpp
Coord targetActualLocation = target.getObjPointer()->getLocation();
if (!targetActualLocation.isValid() || !currentGameMap->tileExists(targetActualLocation)) {
    releaseTarget();
    return;
}
```

**What it does:**
- Checks target's ACTUAL location, not just closest point
- Catches targets that have moved off-map
- Complements the existing closest-point check

### Fix #3: Added AttackPos Validation (Line 570-587)
```cpp
if(!currentGameMap->tileExists(attackPos)) {
    attackPos.invalidate();
    return;
}

if(targetDistance > getWeaponRange() * 2) {
    attackPos.invalidate();
    return;
}
```

**What it does:**
- Validates attackPos is still on map before using it
- Clears attackPos if it's too far away (> 2× weapon range)
- Prevents attacking invalid positions

### Fix #4: Comprehensive Diagnostic Logging
**What it does:**
- Logs when targets are acquired by spatial grid
- Logs when units actually fire
- Logs when targets/attackPos are released/cleared
- Logs distances and weapon ranges

**Why it's important:**
- Will show us EXACTLY what's happening
- Can identify which hypothesis is correct
- Can catch edge cases we haven't thought of

## Most Likely Root Cause

Based on the code analysis, I believe the issue is:

**Hypothesis: Spatial Grid + HUNT Mode Interaction**

1. QuantBot sets units to HUNT mode after moving them toward enemy
2. Spatial grid's ring search finds targets at extreme distances (valid for HUNT mode)
3. Units acquire these distant targets (legitimately)
4. HUNT mode's `isInAttackRange()` always returns true
5. Units path toward target but are "in attack range" immediately
6. Whenever they face the right direction, they fire
7. Target is so far away it's off-screen or beyond map edge
8. Looks like they're "firing at nothing"

**The 5× weapon range check (Fix #1) should solve this** by forcing units to give up on absurdly distant targets.

## Alternative Theories

### Theory A: AttackPos Bug
- Something sets attackPos to invalid location
- Unit fires at that position forever
- **How we'll know:** Diagnostic logs will show attackPos usage

### Theory B: Target Teleportation
- Target was close, then teleported far (carryall, etc.)
- Unit keeps firing at now-distant target
- **How we'll know:** Logs will show target distances jumping

### Theory C: Coordinate Wraparound
- Some calculation wraps negative coordinates to huge values
- Unit thinks target is close but it's actually far
- **How we'll know:** Logs will show impossible coordinates

### Theory D: Guard Point Interference
- Guard point restoration in QuantBot confuses targeting
- Units think they're guarding a distant location
- **How we'll know:** Would need to add guardPoint logging

## Testing Plan

### Step 1: Build with Diagnostics
```bash
cd IDE/xCode
xcodebuild -project "Dune Legacy.xcodeproj" \
           -scheme "Dune Legacy" \
           -configuration GameDebug \
           clean build
```

### Step 2: Run and Capture Logs
```bash
./Dune\ Legacy.app/Contents/MacOS/Dune\ Legacy 2>&1 | tee ~/dune-debug.log
```

### Step 3: Reproduce Issue
- Play until launchers/sonic tanks fire off-screen
- Let it continue for a minute to collect data
- Exit game

### Step 4: Analyze Logs
```bash
grep "\[DIAG\]" ~/dune-debug.log > ~/dune-diag-only.log
grep "excessive distance" ~/dune-diag-only.log
grep "attackPos" ~/dune-diag-only.log
grep "firing at" ~/dune-diag-only.log
```

### Step 5: Determine Outcome

**If no diagnostic logs appear:**
- The fixes worked! Issue is resolved.
- The excessive distance check caught and fixed it silently.

**If "excessive distance" logs appear:**
- Confirms spatial grid was finding too-distant targets
- Fix #1 is working correctly
- Issue should be resolved

**If "attackPos" logs appear:**
- Confirms attackPos was the issue
- Need to trace where it's being set

**If "firing at" logs appear WITHOUT release logs:**
- Means units are still firing at seemingly-valid targets
- Need to investigate why target appears valid but isn't

## What Changed vs Original Code

### Original QuantBot (src/players/AIPlayer.cpp)
```cpp
for(const UnitBase* pUnit : getUnitList()) {
    doMove2Pos(pUnit, destination.x, destination.y, false);
    doSetAttackMode(pUnit, HUNT);
}
```

**Key difference:**
- Didn't restore guard points
- Simpler, but units would wander after combat

### New QuantBot (src/players/QuantBot.cpp)
```cpp
const Coord originalGuardPoint = unit->getGuardPoint();
doMove2Pos(pUnit, destination.x, destination.y, false);
unit->setGuardPoint(originalGuardPoint.x, originalGuardPoint.y);
doSetAttackMode(pUnit, HUNT);
unit->clearAttackPosition();
```

**Key difference:**
- Saves and restores guard point
- More complex but should maintain unit home base
- Explicitly clears attackPos

**Potential issue:**
- The guard point restoration might interact badly with HUNT mode
- Units might be confused about where "home" is

## If Fixes Don't Work

### Option 1: Simplify QuantBot
Remove guard point restoration, go back to simpler approach:
```cpp
doMove2Pos(pUnit, destination.x, destination.y, false);
doSetAttackMode(pUnit, HUNT);
clearAttackPosition();
```

### Option 2: Change HUNT Mode Behavior
Modify `isInAttackRange()` for HUNT to have maximum range:
```cpp
case HUNT: {
    // Allow long chase, but not infinite
    return (blockDistance(location, pObject->getCenterPoint()) <= getWeaponRange() * 10);
} break;
```

### Option 3: Add Range Cap to Spatial Grid
Modify `findTargetViaGrid()` to never return targets beyond a maximum distance:
```cpp
const int maxSearchDistance = huntMode ? (checkRange * 10) : checkRange;
if(distance > maxSearchDistance) {
    continue;
}
```

### Option 4: Disable Spatial Grid for Ranged Units
Fall back to legacy targeting for launchers/sonic tanks:
```cpp
const ObjectBase* ObjectBase::findTarget() const {
    // ... existing code ...
    
    if(getItemID() == Unit_Launcher || getItemID() == Unit_SonicTank) {
        return findTargetLegacy(*this, checkRange);
    }
    
    // ... use spatial grid for others ...
}
```

## Success Criteria

The issue is FIXED when:
1. ✅ Launchers don't fire beyond 2× weapon range
2. ✅ Sonic tanks don't fire beyond 2× weapon range  
3. ✅ Units don't fire at locations off the map
4. ✅ Units still acquire and engage targets normally
5. ✅ No performance regression (spatial grid still used)
6. ✅ No crashes or new bugs introduced

## Related Documentation
- `documents/04-off-screen-firing-investigation.md` - Detailed hypothesis and testing
- `documents/05-diagnostic-build-instructions.md` - How to build and analyze logs
- `documents/01-grid-lifecycle.md` - Spatial grid design
- `documents/02-target-query.md` - Target query planning
- `documents/firing-at-nothing` - Original issue report and conversation history

## Key Files
- `src/units/UnitBase.cpp` - Unit attack/engagement logic
- `src/ObjectBase.cpp` - Target finding via spatial grid
- `src/players/QuantBot.cpp` - AI attack coordination
- `src/SpatialGrid.cpp` - Grid data structure
- `include/units/UnitBase.h` - Unit state definitions

## Timeline
- **Before:** Working legacy targeting, no spatial grid
- **Change:** Introduced spatial grid + modified QuantBot
- **Issue:** Off-screen firing observed
- **Now:** Diagnostic logging + defensive fixes added
- **Next:** Test and identify root cause

