# Off-Screen Firing Investigation

## Problem Statement
Launchers and sonic tanks are firing at phantom targets off-screen, appearing to shoot 4-5× their normal weapon range beyond the map boundaries. This behavior started after introducing the spatial grid system and modifying the QuantBot attack algorithm.

## Changed Systems
1. **Spatial Grid** - New grid-based targeting system (`SpatialGrid.h/cpp`, `SpatialGridHandle.h`)
2. **QuantBot Attack** - Modified squad attack with guard point preservation
3. **Target Finding** - `findTargetViaGrid()` ring-based search replacing legacy tile scans
4. **Path Finding** - Increased node budget to 2048 and time budget to 6ms

## Key Code Paths

### Attack Flow
```
UnitBase::update()
  └─> UnitBase::engageTarget()
      ├─> Has target? 
      │   ├─> Check target validity (null, inactive, can't attack)
      │   ├─> Check if target tile exists [LINE 478]
      │   ├─> Check if beyond weapon range [LINE 499]
      │   └─> If in range and correct angle: attack() [LINE 537-539]
      └─> Has attackPos?
          ├─> Calculate distance to attackPos [LINE 544]
          └─> If in range and correct angle: attack() [LINE 555-557]
```

### Target Acquisition Flow
```
UnitBase::targeting()
  └─> enqueueTargetRequest()
      └─> Game::queueTargetRequest()
          └─> UnitBase::resolvePendingTargetRequest()
              └─> ObjectBase::findTarget()
                  └─> findTargetViaGrid() [with spatial grid]
                      └─> Ring-based search expanding from unit location
```

### QuantBot Attack Flow
```
QuantBot::attack()
  └─> For each eligible unit:
      ├─> Leader picks destination (closest enemy structure/unit)
      ├─> doMove2Pos(destination) [sets guard point to destination]
      ├─> setGuardPoint(originalGuardPoint) [restores old guard point]
      ├─> doSetAttackMode(HUNT)
      └─> clearAttackPosition()
```

## Potential Root Causes

### Hypothesis 1: HUNT Mode Bypasses Range Checks
**Code:** `UnitBase::isInAttackRange()` line 1023-1025
```cpp
case HUNT: {
    return true;
} break;
```

**Issue:** HUNT mode makes `isInAttackRange()` always return `true`, which means:
- Line 467 in `engageTarget()` never releases targets based on range
- Units only release targets if tile doesn't exist or can't attack
- But there's still a weapon range check at line 499 that should prevent firing

**Test:** Add logging when HUNT units have targets beyond weapon range:
```cpp
if(attackMode == HUNT && target && targetDistance > getWeaponRange()) {
    SDL_Log("HUNT unit %u has target %u beyond range: distance=%.1f, weaponRange=%d",
            objectID, target.getObjPointer()->getObjectID(), 
            targetDistance.toDouble(), getWeaponRange());
}
```

### Hypothesis 2: Attack Position Not Validated During Engagement
**Code:** `UnitBase::engageTarget()` line 541-561
```cpp
} else if(attackPos) {
    targetDistance = blockDistance(location, attackPos);
    Sint8 newTargetAngle = destinationDrawnAngle(location, attackPos);
    
    if(targetDistance <= getWeaponRange()) {
        // ... attack!
    }
}
```

**Issue:** 
- `attackPos` is only validated when SET via `doAttackPos()` (checks tile exists)
- No ongoing validation that `attackPos` is still on the map
- No check that `attackPos` is reachable or sensible
- `QuantBot::attack()` calls `clearAttackPosition()` but what if it's set elsewhere?

**Test:** Add logging for attackPos usage:
```cpp
} else if(attackPos) {
    SDL_Log("Unit %u attacking position (%d,%d), distance=%.1f, weaponRange=%d, tileExists=%d",
            objectID, attackPos.x, attackPos.y,
            blockDistance(location, attackPos).toDouble(),
            getWeaponRange(),
            currentGameMap->tileExists(attackPos) ? 1 : 0);
```

### Hypothesis 3: Spatial Grid Returns Invalid Targets
**Code:** `findTargetViaGrid()` in `ObjectBase.cpp` line 494-627

**Issue:**
- Ring-based search expands from unit location
- In HUNT mode, searchRadius can be `maxReachableRadius` (entire map)
- Might find targets at map edges with valid tiles but problematic coordinates
- If target is at edge (63, 63) and unit is at (10, 10), distance is huge but target is "valid"
- No sanity check on maximum distance from seeker

**Test:** Add logging in `findTargetViaGrid()`:
```cpp
if(bestTarget != nullptr) {
    SDL_Log("findTargetViaGrid: seeker=%u acquired target=%u at distance=%.1f, huntMode=%d, checkRange=%d",
            seeker.getObjectID(), bestTarget->getObjectID(),
            bestDistance.toDouble(), huntMode, checkRange);
}
```

### Hypothesis 4: Target Moves Off Map After Acquisition
**Code:** `UnitBase::engageTarget()` line 476-481

**Issue:**
- Target acquired legitimately via spatial grid
- Target then moves off map (carried by carryall, flies away, etc.)
- Check at line 478 should catch this: `if (!currentGameMap->tileExists(targetLocation))`
- But what if `getClosestPoint()` returns a clamped coordinate that IS on the map?
- Or what if target is technically on map but visual position is off-screen?

**Test:** Check if targets are leaving map:
```cpp
Coord targetLocation = target.getObjPointer()->getClosestPoint(location);

if (!currentGameMap->tileExists(targetLocation)) {
    SDL_Log("Unit %u releasing target %u: target location (%d,%d) doesn't exist",
            objectID, target.getObjPointer()->getObjectID(),
            targetLocation.x, targetLocation.y);
    releaseTarget();
    return;
}

// Add additional check for targets that are on-map but way too far
if(targetDistance > getWeaponRange() * 5) {
    SDL_Log("Unit %u releasing target %u: excessive distance %.1f (5x weapon range %d)",
            objectID, target.getObjPointer()->getObjectID(),
            targetDistance.toDouble(), getWeaponRange());
    releaseTarget();
    return;
}
```

### Hypothesis 5: Guard Point Restoration Creates Phantom Destinations
**Code:** `QuantBot::attack()` line 2244-2250

**Issue:**
- `doMove2Pos()` sets both destination AND guard point to the target location
- Guard point then immediately restored to original
- But destination might still point to that location
- Units might be trying to move to a far-off destination while also acquiring targets

**Test:** Log destination vs guard point:
```cpp
if(attackMode == HUNT && destination.isValid() && guardPoint.isValid()) {
    FixPoint destDist = blockDistance(location, destination);
    FixPoint guardDist = blockDistance(location, guardPoint);
    if(destDist > getWeaponRange() * 3) {
        SDL_Log("HUNT unit %u has distant destination: loc=(%d,%d), dest=(%d,%d), guard=(%d,%d)",
                objectID, location.x, location.y, 
                destination.x, destination.y, guardPoint.x, guardPoint.y);
    }
}
```

### Hypothesis 6: No Range Validation in attack()
**Code:** `UnitBase::attack()` line 197-248

**Issue:**
- `attack()` method doesn't check if target is in weapon range before firing
- It assumes the caller (`engageTarget()`) has already validated
- If `engageTarget()` has a bug, `attack()` will fire anyway
- Only checks weapon timer and angle

**Test:** Add range check in attack():
```cpp
bool UnitBase::attack() {
    if(numWeapons) {
        ObjectBase* pObject = target.getObjPointer();
        if(pObject != nullptr) {
            Coord targetCenterPoint = pObject->getClosestCenterPoint(location);
            FixPoint distance = distanceFrom(getCenterPoint(), targetCenterPoint);
            
            if(distance > getWeaponRange() * TILESIZE * 1.5) {
                SDL_Log("Unit %u attempting to attack out-of-range target %u: distance=%.1f, range=%d",
                        objectID, pObject->getObjectID(),
                        distance.toDouble(), getWeaponRange() * TILESIZE);
                return false;
            }
        }
        
        if((primaryWeaponTimer == 0) || ...) {
            // ... existing attack code
```

## Testing Strategy

### Phase 1: Add Diagnostic Logging
Create a debug build with all the logging from hypotheses 1-6 enabled. Focus on:
- When units fire (attack() called)
- Target acquisition (findTargetViaGrid results)
- Distance calculations
- attackPos usage
- Guard point vs destination divergence

### Phase 2: Reproduce and Capture Logs
1. Start a custom game with multiple AI players
2. Let game run until off-screen firing is observed
3. Note game cycle count when it happens
4. Extract relevant log lines around that time
5. Look for patterns:
   - Is it always HUNT mode?
   - Is attackPos ever set?
   - Are targets being acquired at extreme distances?
   - Are targets leaving the map?

### Phase 3: Test Fixes Based on Findings

#### If Hypothesis 1 (HUNT range bypass):
Add maximum distance check even in HUNT mode

#### If Hypothesis 2 (attackPos validation):
Add tile existence check in engageTarget attackPos branch

#### If Hypothesis 3 (spatial grid invalid targets):
Add maximum distance sanity check in findTargetViaGrid

#### If Hypothesis 4 (targets moving off-map):
Strengthen the off-map detection (check actual coordinates, not just closest point)

#### If Hypothesis 5 (guard point issues):
Simplify QuantBot to not use guard point restoration, just use raw HUNT

#### If Hypothesis 6 (no attack() validation):
Add defensive distance check in attack() before firing

## Quick Wins to Try First

### 1. Add Distance Sanity Check in engageTarget()
Before line 537, add:
```cpp
// Sanity check: don't fire if target is absurdly far away
if(targetDistance > getWeaponRange() * 2) {
    SDL_Log("Unit %u: target too far for engagement: %.1f > %d×2",
            objectID, targetDistance.toDouble(), getWeaponRange());
    if(attackMode == HUNT) {
        // In hunt mode, just keep moving toward it
        setDestination(targetLocation);
    } else {
        releaseTarget();
    }
    return;
}
```

### 2. Validate attackPos in engageTarget()
After line 541, add:
```cpp
} else if(attackPos) {
    // Validate attackPos is still on the map
    if(!currentGameMap->tileExists(attackPos)) {
        SDL_Log("Unit %u: clearing invalid attackPos (%d,%d)",
                objectID, attackPos.x, attackPos.y);
        attackPos.invalidate();
        return;
    }
    
    targetDistance = blockDistance(location, attackPos);
```

### 3. Strengthen Off-Map Detection
At line 478, enhance the check:
```cpp
Coord targetLocation = target.getObjPointer()->getClosestPoint(location);

// Check both the coordinate validity AND that it's reachable
if (!currentGameMap->tileExists(targetLocation) || 
    !target.getObjPointer()->getLocation().isValid() ||
    !currentGameMap->tileExists(target.getObjPointer()->getLocation())) {
    SDL_Log("Unit %u: target %u is off-map or invalid, releasing",
            objectID, target.getObjPointer()->getObjectID());
    releaseTarget();
    return;
}
```

## Files to Monitor
- `src/units/UnitBase.cpp` - Main attack/engagement logic
- `src/ObjectBase.cpp` - Target finding (findTargetViaGrid)
- `src/players/QuantBot.cpp` - AI attack coordination
- `src/SpatialGrid.cpp` - Grid operations
- `include/units/UnitBase.h` - Unit state definitions

## Related Issues Fixed Previously
According to the conversation history:
- Fixed Tile::squash iterator corruption
- Added guard point restoration in QuantBot
- Added tile existence check for targets (line 478)
- Increased pathfinding budgets

These fixes addressed crashes but didn't fix the off-screen firing issue, suggesting the root cause is in the targeting/attack logic, not memory corruption.

## Next Steps
1. Create a debug build with comprehensive logging (see Phase 1)
2. Run a test game and reproduce the issue
3. Analyze logs to determine which hypothesis is correct
4. Implement targeted fix
5. Test fix doesn't break normal targeting behavior
6. Document final solution

## Open Questions
- Does this only affect ranged units (launchers, sonic tanks)?
- Does it only happen in HUNT mode or also GUARD/AREAGUARD?
- Is attackPos ever actually set during QuantBot attacks?
- How far off-screen are the shots going (screen edge vs way beyond map bounds)?
- Does it happen immediately or only after units have been fighting for a while?

