# UnitBase.cpp Changes Review - Off-Screen Firing Investigation

## Summary of ALL Changes vs Remote

### 1. Target Location Validation (Lines 478-492) ✅
```cpp
if (!currentGameMap->tileExists(targetLocation)) {
    releaseTarget();
    return;
}
if (!targetActualLocation.isValid() || !currentGameMap->tileExists(targetActualLocation)) {
    releaseTarget();
    return;
}
```
**Purpose:** Prevent targeting off-map coordinates  
**Should Help:** Yes - releases target if coordinates are invalid  
**Could Cause Issue:** No - only adds safety checks

### 2. `isInAttackRange()` Fix (Lines 1041-1084) ⚠️
**The Key Change:**

**BEFORE (all modes except HUNT):**
```cpp
return (blockDistance(guardPoint*TILESIZE + ..., target) <= checkRange*TILESIZE);
```
Always checked from `guardPoint`

**AFTER:**
- **GUARD mode:** Checks from `guardPoint` (same as before)
- **AREAGUARD mode:** Checks from `location` (CHANGED)
- **AMBUSH mode:** Checks from `location` (CHANGED)
- **HUNT mode:** Returns `true` (same as before - no range limit)

**AREAGUARD Check Range:**
```cpp
checkRange = getAreaGuardRange() + getWeaponRange() + 1
           = 2*weaponRange + weaponRange + 1
           = 3*weaponRange + 1
```
**For a launcher (10 tile range): checkRange = 31 tiles!**

**Analysis:**
- Units in AREAGUARD mode think targets within 31 tiles are "in attack range"
- This means `isInAttackRange()` returns true for distant targets
- But they still shouldn't FIRE unless within weapon range...

### 3. Firing Logic in engageTarget() (Lines 500-564) ✅
```cpp
targetDistance = blockDistance(location, targetLocation);  // Distance in tiles

if(targetDistance > getWeaponRange()) {
    // NOT in weapon range - move towards target, DON'T fire
    setDestination(targetLocation);
    return;
}

// Only reach here if within weapon range
if(getCurrentAttackAngle() == newTargetAngle) {
    attack();  // Fire!
}
```

**This logic looks correct** - units should only fire if `targetDistance <= weaponRange`

### 4. AttackPos Validation (Lines 570-578) ✅
```cpp
if(!currentGameMap->tileExists(attackPos)) {
    attackPos.invalidate();
    return;
}
```
**Purpose:** Prevent attacking invalid positions  
**Should Help:** Yes  
**Could Cause Issue:** No

### 5. Diagnostic Logging (Lines 550-563) 📊
```cpp
if((getItemID() == Unit_Launcher || getItemID() == Unit_SonicTank)) {
    if(targetDistance > suspiciousDistanceInPixels) {
        SDL_Log("[DEBUG] Unit X firing at target Y: distance=Z tiles, weaponRange=W, ...
    }
}
```
**Purpose:** Debug logging to diagnose the issue  
**Effect:** None on behavior, just logs

### 6. Pathfinding Changes (Lines 1356-1587) ✅
- Added node budget support
- Changed return type to PathSearchResult
- **Should not affect firing logic**

## Potential Issues

### Theory 1: HUNT Mode ⚠️
If launchers are in **HUNT mode**, `isInAttackRange()` returns `true` for ANY distance.

**Flow:**
1. Launcher in HUNT mode acquires target 50 tiles away
2. `isInAttackRange()` returns true ✓
3. Target not released
4. In `engageTarget()`, if `targetDistance > weaponRange`, moves towards target
5. Should NOT fire until within range

**This should be OK** - but need to verify with logs

### Theory 2: Distance Units Mismatch ❓
- `targetDistance` calculated with `blockDistance(location, targetLocation)` (tiles)
- `getWeaponRange()` returns tiles
- Comparison: `targetDistance > getWeaponRange()` should work

**Appears correct** - both in tiles

### Theory 3: AREAGUARD Large Range ⚠️
- AREAGUARD allows targeting up to 31 tiles away (for 10-tile weapon)
- But firing should still be limited to 10 tiles

**Should be OK** - targeting and firing are separate checks

## What the Diagnostic Logs Will Show

When launchers fire, you'll see:
```
[DEBUG] Unit 123 (HUNT/AREAGUARD/GUARD mode) firing at target 456:
        distance=15.3 tiles, weaponRange=10,
        targetLoc=(50,60), myLoc=(35,55)
```

### What to Look For:
1. **Attack Mode:** HUNT, AREAGUARD, or GUARD?
2. **Distance:** Is it > weaponRange? (Should not be firing if so!)
3. **Locations:** Is target actually off-map or just far away?

### If distance > weaponRange:
🔴 **BUG CONFIRMED** - The check at line 510 is not working

### If distance <= weaponRange but target appears "off-screen":
✅ **Not a bug** - Target is within range, just not visible to player

### If target location is invalid (negative or > map size):
🔴 **BUG** - Validation at lines 478-492 not working

## Conclusion

**The code logic appears sound to me:**
- Target validation should prevent off-map targets
- Distance check should prevent firing beyond weapon range
- isInAttackRange() fix should prevent the kiting bug

**Next Step:** Run the game and check the `[DEBUG]` logs when launchers fire to see what's actually happening.

