# Rollback: Reverted Both Squash Fix and Path Recalculation

## User Feedback

> "it seems like your fix for 1 causes 2. so we might just need to roll it all back. no squash fix, no smooth unit movement (back to default path finding)"

## What Was Reverted

### 1. Tile::squash() - Back to Original (Buggy) Version

**File**: `src/Tile.cpp:638-651`

**Reverted FROM** (the "fix"):
```cpp
void Tile::squash() const {
    if (!hasInfantry()) return;
    if (currentGame == nullptr) return;

    // Collect valid infantry pointers BEFORE squashing
    std::vector<InfantryBase*> infantryToSquash;
    infantryToSquash.reserve(assignedInfantryList.size());
    
    for (Uint32 id : assignedInfantryList) {
        InfantryBase* infantry = static_cast<InfantryBase*>(
            currentGame->getObjectManager().getObject(id));
        if (infantry != nullptr && infantry->isActive()) {
            infantryToSquash.push_back(infantry);
        }
    }

    for (InfantryBase* infantry : infantryToSquash) {
        if (infantry != nullptr && infantry->isActive()) {
            infantry->squash();
        }
    }
}
```

**Reverted TO** (original):
```cpp
void Tile::squash() const {
    if (!hasInfantry()) return;

    auto iter = assignedInfantryList.begin();
    do {
        InfantryBase* current = static_cast<InfantryBase*>(
            currentGame->getObjectManager().getObject(*iter));
        ++iter;

        if(current == nullptr)
            continue;

        current->squash();
    } while(iter != assignedInfantryList.end());
}
```

**Expected behavior**: 
- ❌ Will crash when tanks run over infantry (iterator invalidation)
- ✅ No firing bug (doesn't process zombie infantry)

### 2. Path Recalculation - Back to Old Logic

**File**: `src/units/UnitBase.cpp:476-489`

**Reverted FROM** (smooth movement):
```cpp
Coord targetLocation = target.getObjPointer()->getClosestPoint(location);

if(destination != targetLocation) {
    FixPoint movementDistance = blockDistance(destination, targetLocation);
    
    if(movementDistance > 3) {
        // Target moved very far - clear path and stop
        clearPath();
    } else if(movementDistance > 1) {
        // Target moved 1-3 tiles - keep following, queue new path
        destination = targetLocation;
        if(!pathRequestQueued) {
            enqueuePathRequest();
        }
    } else {
        // Minor movement - just update destination
        destination = targetLocation;
    }
}
```

**Reverted TO** (original stuttery):
```cpp
Coord targetLocation = target.getObjPointer()->getClosestPoint(location);

if(destination != targetLocation) {
    // Only recalculate path if target moved significantly (> 1 tile)
    FixPoint movementDistance = blockDistance(destination, targetLocation);
    if(movementDistance > 1) {
        // Target moved significantly, recalculate path
        clearPath();
    } else {
        // Minor movement, just update destination
        destination = targetLocation;
    }
}
```

**Expected behavior**:
- ✅ Units will stutter when chasing targets (old behavior)
- ✅ Units will clear paths frequently
- ✅ `justStoppedMoving` will trigger less often
- ✅ `squash()` will be called RARELY
- ✅ Squash crash will be rare (but still possible)

## The Core Issue

The path recalculation improvement **exposed a pre-existing bug** in `Tile::squash()`:

1. **Old path logic**: Units stuttered → rarely completed paths → `squash()` called rarely → bug rarely hit
2. **New path logic**: Units moved smoothly → completed paths often → `squash()` called constantly → bug hit constantly

## Why The Fix Caused Firing Bug

The "fix" was processing **zombie infantry** (destroyed but still in list with corrupt data):
- Fix collected ALL IDs, including destroyed infantry
- Called `squash()` on zombie infantry with corrupt `target`/`attackPos` fields
- This corruption somehow propagated to nearby units
- Units fired at invalid coordinates

## Current State

**Back to square one**:
- ❌ Squash crash can occur (rare)
- ✅ No firing bug
- ❌ Units stutter when chasing targets (gameplay regression)
- ❌ Path recalculation happens too frequently (performance regression)

## The Real Root Cause (Still Unknown)

**Why is `assignedInfantryList` corrupt?**

Possible scenarios:
1. Infantry destroyed by weapons, but not removed from list
2. Race condition during combat (multiple simultaneous squashes)
3. Timing issue in `destroy()` → `removeObjectFromMap()` → `unassignInfantry()` chain
4. Units targeting destroyed infantry, arriving at tile with stale list

**This needs further investigation to properly fix both issues.**

## Files Changed

- `src/Tile.cpp:638-651` - Reverted to original squash
- `src/units/UnitBase.cpp:476-489` - Reverted to old path logic

## Build Status

✅ Build successful  
⚠️ Game will have original bugs:
- Rare squash crash
- Unit stuttering when chasing targets

