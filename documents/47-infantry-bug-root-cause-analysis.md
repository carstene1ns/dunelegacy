# Infantry Bug Root Cause Analysis

## User's Key Insight

> "this infantry bug is NEW. I never had it before. could it be from the pathing algo having an infantry target that doesn't exist or something else to do with our recent changes??"

**The user is RIGHT. This is NOT the root cause - it's a SYMPTOM.**

## The Smoking Gun

**File**: `src/units/TrackedUnit.cpp:53-59`

```cpp
void TrackedUnit::checkPos()
{
    GroundUnit::checkPos();

    if(active && justStoppedMoving)
        currentGameMap->getTile(location.x, location.y)->squash();
}
```

**`squash()` is called when a tracked unit (tank, harvester, etc.) STOPS MOVING on a tile.**

## How Path Recalculation Change Triggered This

### OLD Behavior (before commit 734c635)
```cpp
if(movementDistance > 1) {
    clearPath();  // Clear path if target moves > 1 tile
}
```

**Result**:
- Units cleared paths frequently
- Units stuttered/stopped often when chasing targets
- `justStoppedMoving` was set less often (units would resume moving quickly)
- **`squash()` was called RARELY**

### NEW Behavior (commit 734c635)
```cpp
if(movementDistance > 3) {
    clearPath();  // Only clear if target moves > 3 tiles
} else if(movementDistance > 1) {
    // Keep following old path, queue new one
    destination = targetLocation;
    enqueuePathRequest();
}
```

**Result**:
- Units keep moving smoothly
- Units complete their paths more often
- Units actually ARRIVE at destinations
- `justStoppedMoving` is set MORE often
- **`squash()` is called CONSTANTLY**

## Why This Exposed The Bug

The `Tile::squash()` iterator invalidation bug **was always there**, but:

**Before**: Rarely triggered because units stuttered and didn't complete paths  
**After**: Constantly triggered because units smoothly reach destinations

## The Real Question: Why Is assignedInfantryList Corrupt?

This is what we need to investigate. Possible scenarios:

### Scenario 1: Race Condition During Combat
```
1. Tile has 2 infantry: A and B
2. Tank stops on tile → squash() starts
3. squash() begins iterating: processes A
4. Infantry A is destroyed → removes itself from list
5. Iterator becomes invalid
6. Tries to process B → CRASH (iterator invalidation)
```

### Scenario 2: Stale IDs From Destroyed Infantry
```
1. Infantry A is on tile X
2. Infantry A is killed by weapon fire elsewhere
3. destroy() is called
4. destroy() calls removeObjectFromMap()
5. removeObjectFromMap() should call unassignInfantry()
6. BUT: What if there's a timing issue?
7. Tank arrives at tile X → squash()
8. assignedInfantryList still has A's ID
9. getObject(A's ID) returns nullptr or corrupted pointer
10. CRASH
```

### Scenario 3: Multiple Units Squashing Simultaneously
```
1. Two tanks arrive at same tile in same game cycle
2. Both call squash()
3. First squash starts iterating
4. Second squash also starts iterating
5. First squash destroys infantry A
6. Second squash tries to process already-destroyed A
7. CRASH
```

### Scenario 4: Infantry Targeting Infantry
```
1. Unit targets an infantry
2. Infantry gets destroyed by weapon fire
3. Unit's destination still points to that tile
4. Unit arrives, stops → squash()
5. assignedInfantryList has stale entry for destroyed infantry?
```

## What We Need To Check

1. **Is destroy() properly cleaning up infantry from tiles?**
   - Check `UnitBase::destroy()` → `removeObjectFromMap()` → `unassignInfantry()`
   - Are there any scenarios where this chain is broken?

2. **Can multiple units squash the same tile simultaneously?**
   - Is `squash()` marked `const`? (YES - line 638 in Tile.cpp)
   - Can it be called re-entrantly?

3. **Are there timing issues with list modification?**
   - Can infantry be destroyed while squash() is iterating?
   - Is there a lock or mutex protecting assignedInfantryList?

4. **What about the firing bug correlation?**
   - User reported: "firing bug GONE" when squash was reverted
   - How are these connected?
   - Could corrupt infantry list affect targeting?

## The Firing Bug Connection

The user observed:
- **With squash "fix"**: No crash, but missiles fly off-map
- **With squash reverted**: Crash, but no firing bug

**Hypothesis**: The squash "fix" was processing **already-destroyed infantry** with **corrupt targeting data**. This corruption somehow propagated to nearby units, causing them to fire at invalid coordinates!

### How Corruption Could Spread

```cpp
// OLD "fix":
for (Uint32 id : infantryIDs) {
    InfantryBase* current = getObject(id);
    if (current != nullptr) {
        current->squash();  // Might call squash on zombie infantry
    }
}
```

If `current` is a **zombie object** (partially destroyed):
- It might have corrupt `target` or `attackPos` fields
- Squashing it might trigger code that reads these corrupt fields
- This corruption could affect spatial grid or other global state
- Nearby units query spatial grid → get corrupt coordinates
- Fire at invalid targets!

## What Changed To Cause This?

**Primary Suspect**: Commit 734c635 (path recalculation)
- Units complete paths more often
- `justStoppedMoving` triggers more frequently
- `squash()` called constantly
- Pre-existing bug is now hit constantly

**Secondary Suspects**:
- More units on map (removed 70-unit squad limit in b8b1a23)
- More aggressive AI (ornithopter logic, build order changes)
- Different pathfinding patterns (units path differently)

## Recommendation

1. **Keep the squash() fix** (it prevents crashes)
2. **Investigate why assignedInfantryList has stale entries**
3. **Check if path recalculation is causing units to target destroyed infantry**
4. **Add diagnostics**:
   ```cpp
   if (infantry == nullptr) {
       SDL_Log("[ERROR] Null infantry in assignedInfantryList on tile %d,%d", x, y);
   }
   if (!infantry->isActive()) {
       SDL_Log("[ERROR] Inactive infantry ID=%d still in list on tile %d,%d", 
               id, x, y);
   }
   ```

## Test Plan

1. **Test current build** (with fixed squash)
   - Does it crash? NO (hopefully)
   - Do units fire off-map? UNKNOWN (need to test)

2. **If firing bug persists**:
   - Add diagnostics to find corrupt infantry
   - Check if `destination` points to tiles with destroyed infantry
   - Check spatial grid for corrupt entries

3. **If firing bug is gone**:
   - The fix worked! (filtering inactive infantry prevented corruption)
   - Move on to next bug

## Conclusion

The user's intuition was **100% correct**:
- The squash crash is a **SYMPTOM**
- The path recalculation change **EXPOSED** the bug
- There's likely a deeper issue with infantry list management
- The "fix" (filtering inactive infantry) might have solved both issues!

