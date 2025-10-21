# Proper Tile::squash() Fix

## The Root Cause

When `Tile::squash()` calls `infantry->squash()`, it destroys the infantry, which:
1. Calls `removeObjectFromMap(objectID)`
2. Which removes the infantry from `assignedInfantryList`
3. **While we're iterating that list!**

## Why Both Previous Versions Had Problems

### Original Buggy Code
```cpp
auto iter = assignedInfantryList.begin();
do {
    InfantryBase* current = getObject(*iter);
    ++iter;  // Increment before squashing
    current->squash();  // But this modifies the list we're iterating!
} while(iter != assignedInfantryList.end());
```

**Problem**: Even though iterator is incremented first, `removeObjectFromMap()` removes the infantry from **ALL tiles**, potentially invalidating multiple iterators or causing complex list corruption.

**Result**: ✅ No firing bug, ❌ Crashes

### First "Fix" (Copied IDs)
```cpp
std::vector<Uint32> infantryIDs;
for (Uint32 id : assignedInfantryList) {
    infantryIDs.push_back(id);
}

for (Uint32 id : infantryIDs) {
    InfantryBase* current = getObject(id);
    if (current != nullptr) {
        current->squash();
    }
}
```

**Problem**: After first `squash()`, subsequent IDs might point to:
- Partially destroyed objects
- Objects with corrupt targeting data (target, attackPos, destination)
- "Zombie" objects that are in the process of being cleaned up

**Result**: ✅ No crash, ❌ Firing bug (corrupt targeting data)

## The Proper Fix

```cpp
void Tile::squash() const {
    if (!hasInfantry()) return;
    if (currentGame == nullptr) return; // Safety during cleanup

    // STEP 1: Collect POINTERS to valid, active infantry
    std::vector<InfantryBase*> infantryToSquash;
    infantryToSquash.reserve(assignedInfantryList.size());
    
    for (Uint32 id : assignedInfantryList) {
        InfantryBase* infantry = static_cast<InfantryBase*>(
            currentGame->getObjectManager().getObject(id));
        
        // Filter: only collect infantry that are CURRENTLY alive and valid
        if (infantry != nullptr && 
            infantry->isActive() && 
            !infantry->isDestroyed()) {
            infantryToSquash.push_back(infantry);
        }
    }

    // STEP 2: Squash all collected infantry
    for (InfantryBase* infantry : infantryToSquash) {
        // Double-check validity (might have been destroyed by previous squash)
        if (infantry != nullptr && 
            infantry->isActive() && 
            !infantry->isDestroyed()) {
            infantry->squash();
        }
    }
}
```

## Why This Works

1. **Collects POINTERS, not IDs**: Pointers remain valid even after list modification
2. **Filters before collecting**: Only processes infantry that are currently valid/active
3. **Double-checks before squashing**: Skips infantry already destroyed by previous iterations
4. **Separates collection from processing**: List can be safely modified during squash phase

## What We're Filtering Out

The key additions are:
- `infantry->isActive()`: Object hasn't been marked for deletion
- `!infantry->isDestroyed()`: Object isn't in the process of being destroyed

These checks ensure we only process **fully valid, living infantry** with **intact targeting data**.

## Expected Results

✅ No crash (iterator-safe)  
✅ No firing bug (no corrupt targeting data)  
✅ Proper cleanup (all valid infantry are squashed)

## Testing Plan

1. Build and run
2. Trigger squash scenarios (units running over infantry)
3. Verify no crash
4. Verify no missiles flying off-map
5. Check logs for any unusual behavior

## Files Changed

- `src/Tile.cpp:638-663` - New squash implementation

