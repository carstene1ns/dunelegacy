# Root Cause Identified: Kiting + Attack Interaction

## The Smoking Gun

I found it! The issue is an interaction between:
1. The kiting logic in `checkAllUnits()` (runs every AI update)
2. The attack squad logic in `attack()` (sets units to HUNT mode)
3. How `doMove2Pos()` and guard points work

## The Bug Flow

### Normal Attack Flow (Working)
```
1. attack() calls doMove2Pos(destination)
   - Sets guardPoint = destination
   - Clears target
2. attack() restores guardPoint to original
3. attack() sets mode to HUNT
4. Unit moves toward destination, acquires targets along the way
5. Unit fires at nearby targets (< weapon range)
```

### Broken Flow (Off-Screen Firing)

```
1. attack() sets unit to HUNT mode headed toward enemy base
2. Unit acquires target via spatial grid (could be far away)
3. checkAllUnits() kiting logic triggers (happens every AIUPDATEINTERVAL):
   - if (unit has target && distance to target <= 6 tiles)
   - doSetAttackMode(AREAGUARD)
   - doMove2Pos(squadCenterLocation, forced=true)
   
4. doMove2Pos() does:
   - setTarget(nullptr) ← CLEARS the target
   - setDestination(squadCenterLocation)
   - setGuardPoint(squadCenterLocation) ← MOVES guard point!
   - setForced(true)
   
5. Unit now has:
   - NO target (cleared by doMove2Pos)
   - destination = squadCenterLocation
   - guardPoint = squadCenterLocation  
   - attackMode = AREAGUARD
   
6. Unit paths back toward squadCenter
7. targeting() enqueues target request
8. findTargetViaGrid() finds new target (AREAGUARD has 2× weapon range)
9. Unit acquires target that could be far away
10. **BUT**: guardPoint is now at squadCenterLocation (potentially far from unit)
11. Unit is in AREAGUARD mode, range check is:
    blockDistance(guardPoint, target) <= (2×weaponRange + weaponRange + 1)
12. If guardPoint is far from unit's current location, this can approve very distant targets!
```

## Why Only Launchers and Sonic Tanks?

**The kiting logic ONLY applies to launchers and deviators!**

```cpp
else if ((pUnit->getItemID() == Unit_Launcher || pUnit->getItemID() == Unit_Deviator)
    && pUnit->hasATarget() && (difficulty != Difficulty::Easy)) {
    // Special kiting logic here
}
```

Other units don't have this kiting behavior, so they don't get their guard points moved around constantly.

## The Core Problem

### Issue #1: guardPoint vs location confusion

When a unit is in AREAGUARD mode, `isInAttackRange()` checks:
```cpp
case AREAGUARD: {
    checkRange = getAreaGuardRange() + getWeaponRange() + 1;
} break;

// Then later:
return (blockDistance(guardPoint*TILESIZE + Coord(TILESIZE/2, TILESIZE/2), 
                      pObject->getCenterPoint()) 
        <= checkRange*TILESIZE);
```

**It checks distance from GUARD POINT, not from UNIT LOCATION!**

So if:
- Unit is at (10, 10)
- guardPoint is at (50, 50) because kiting moved it
- Target is at (55, 55)
- Weapon range = 5

The check is:
- blockDistance((50,50), (55,55)) = ~7 tiles
- checkRange = 2×5 + 5 + 1 = 16 tiles
- 7 <= 16: TRUE! Attack approved!

But unit is actually at (10,10), so target is ~63 tiles away!

### Issue #2: Kiting happens every frame it's checked

The kiting logic in `checkAllUnits()` runs every AIUPDATEINTERVAL. If a launcher has a target within 6 tiles:
1. It gets sent back to squad center
2. Guard point moves to squad center
3. Target gets cleared
4. New target acquired (now validated against squad center location, not unit location)
5. Unit fires at this new target from wherever it is

## Why We See Off-Screen Firing

1. Launcher is in HUNT mode attacking enemy base
2. Kiting triggers because enemy is close
3. Launcher sent back to squad center (guardPoint = squadCenter)
4. Target cleared, new target acquired
5. New target is validated against squadCenter location (far from launcher)
6. Launcher fires at target that's way beyond its actual weapon range
7. Visual: rockets flying off-screen toward phantom targets

## The Actual Code Locations

### Kiting Logic (QuantBot.cpp ~line 2746)
```cpp
else if ((pUnit->getItemID() == Unit_Launcher || pUnit->getItemID() == Unit_Deviator)
    && pUnit->hasATarget() && (difficulty != Difficulty::Easy)) {
    const ObjectBase* pTarget = pUnit->getTarget();
    if (pTarget != nullptr) {
        if (blockDistance(pUnit->getLocation(), pTarget->getLocation()) <= 6 
            && pTarget->getItemID() != Unit_Ornithopter) {
            doSetAttackMode(pUnit, AREAGUARD);
            doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true);
        }
    }
}
```

### Attack Range Check (UnitBase.cpp ~line 1015)
```cpp
case AREAGUARD: {
    checkRange = getAreaGuardRange() + getWeaponRange() + 1;
} break;

// Line 1045:
return (blockDistance(guardPoint*TILESIZE + Coord(TILESIZE/2, TILESIZE/2), 
                      pObject->getCenterPoint()) 
        <= checkRange*TILESIZE);
```

### Attack Squad Logic (QuantBot.cpp ~line 2242)
```cpp
const Coord originalGuardPoint = unit->getGuardPoint();
doMove2Pos(pUnit, targetDestination.x, targetDestination.y, false);
unit->setGuardPoint(originalGuardPoint.x, originalGuardPoint.y);
doSetAttackMode(pUnit, HUNT);
unit->clearAttackPosition();
```

## Solutions

### Solution 1: Fix AREAGUARD Range Check (Recommended)
Check distance from **unit location**, not guard point:

```cpp
bool UnitBase::isInAttackRange(const ObjectBase* pObject) const {
    int checkRange;
    Coord checkLocation = location; // Default to unit's location
    
    switch(attackMode) {
        case GUARD: {
            checkRange = getWeaponRange();
            checkLocation = guardPoint; // Only GUARD checks from guard point
        } break;

        case AREAGUARD: {
            checkRange = getAreaGuardRange() + getWeaponRange() + 1;
            // AREAGUARD should check from UNIT location, not guard point
            checkLocation = location;
        } break;

        case AMBUSH: {
            checkRange = getViewRange() + 1;
            checkLocation = location;
        } break;

        case HUNT: {
            return true;
        } break;

        default: {
            return false;
        } break;
    }

    return (blockDistance(checkLocation*TILESIZE + Coord(TILESIZE/2, TILESIZE/2), 
                          pObject->getCenterPoint()) 
            <= checkRange*TILESIZE);
}
```

### Solution 2: Don't Change Guard Point in Kiting
Keep the original guard point when kiting:

```cpp
else if ((pUnit->getItemID() == Unit_Launcher || pUnit->getItemID() == Unit_Deviator)
    && pUnit->hasATarget() && (difficulty != Difficulty::Easy)) {
    const ObjectBase* pTarget = pUnit->getTarget();
    if (pTarget != nullptr) {
        if (blockDistance(pUnit->getLocation(), pTarget->getLocation()) <= 6 
            && pTarget->getItemID() != Unit_Ornithopter) {
            // Save original guard point
            Coord originalGuard = pUnit->getGuardPoint();
            
            doSetAttackMode(pUnit, AREAGUARD);
            doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true);
            
            // Restore original guard point
            const_cast<UnitBase*>(pUnit)->setGuardPoint(originalGuard.x, originalGuard.y);
        }
    }
}
```

### Solution 3: Use RETREAT Mode for Kiting
Don't use AREAGUARD for kiting, use RETREAT:

```cpp
else if ((pUnit->getItemID() == Unit_Launcher || pUnit->getItemID() == Unit_Deviator)
    && pUnit->hasATarget() && (difficulty != Difficulty::Easy)) {
    const ObjectBase* pTarget = pUnit->getTarget();
    if (pTarget != nullptr) {
        if (blockDistance(pUnit->getLocation(), pTarget->getLocation()) <= 6 
            && pTarget->getItemID() != Unit_Ornithopter) {
            doSetAttackMode(pUnit, RETREAT);  // Changed from AREAGUARD
            doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true);
        }
    }
}
```

RETREAT mode has restrictive targeting, so units won't fire while retreating.

### Solution 4: Remove Kiting from checkAllUnits()
The git diff shows a comment was added:
```cpp
// Launcher retreat logic removed from onDamage() to prevent spam
// It's handled in checkAllUnits() instead
```

But checkAllUnits() runs every AIUPDATEINTERVAL unconditionally. The old onDamage() version only triggered when the unit took damage. Maybe the move to checkAllUnits() was the mistake - it's now triggering too often and disrupting combat.

**Consider moving kiting back to onDamage() where it only triggers on actual damage.**

## Recommendation

**Use Solution #1 (Fix AREAGUARD Range Check)**

This is the most correct fix because:
1. AREAGUARD should logically check from the unit's location, not a potentially distant guard point
2. It fixes the root cause rather than working around it
3. It won't break existing behavior for other units
4. It's a simple, clean change

The original intent of AREAGUARD is "defend an area around where I am," not "defend an area around my home base from anywhere on the map."

## Testing the Fix

After implementing Solution #1:
1. Launchers should still kite away from close enemies ✓
2. Launchers should only fire at targets within weapon range of THEIR LOCATION ✓
3. No more off-screen firing ✓
4. Other units unaffected ✓

## Why This Wasn't Caught Earlier

The original code probably had:
- No kiting for launchers, OR
- Kiting that didn't change guard point, OR
- Units were mostly in HUNT mode which bypasses the range check

The combination of:
- New spatial grid (finds targets across map)
- Kiting in checkAllUnits() (moves guard point)
- AREAGUARD mode (checks range from guard point not location)

Created a perfect storm where the range check became meaningless.

