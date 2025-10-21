# Tile.cpp Revert Investigation

## Problem Summary

**Visual symptoms:**
- QuantBot units (troopers) walking through buildings
- Units walking off the map
- Game continues running (doesn't crash)
- Happens in NEW games (not save corruption)

**Root cause hypothesis:**
The "fix" to `Tile::squash()` prevented crashes but **masked the real bug**: destroyed/invalid units are still lingering in tile assignment lists with corrupted data.

## What Was "Fixed" (Reverted)

### Original Tile::squash() (remote/working version):
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

### "Fixed" version (that masked the bug):
```cpp
void Tile::squash() const {
    if (assignedInfantryList.empty()) return;

    // Copy IDs first to avoid iterator invalidation
    std::vector<Uint32> infantryIds;
    infantryIds.reserve(assignedInfantryList.size());
    for (const Uint32 infantryId : assignedInfantryList) {
        infantryIds.push_back(infantryId);
    }

    // Iterate over copy
    for (const Uint32 infantryId : infantryIds) {
        if (auto* infantry = static_cast<InfantryBase*>(
            currentGame->getObjectManager().getObject(infantryId))) {
            infantry->squash();
        }
    }
}
```

**Why the "fix" was wrong:**
- It prevented iterator invalidation crashes
- But it didn't address **why** destroyed units were still in the list
- Invalid units with corrupted data bypass collision/bounds checks

## What We've Done

1. **Reverted** `Tile::squash()` to original version
2. **Rebuilt** in GameDebug configuration
3. **Ready to test** - game will now crash when the real bug occurs

## Expected Behavior When Testing

When you run the game and units try to squash infantry:

**Best case:** Game crashes with stack trace showing:
- `Tile::squash()` at line 643
- Which unit is doing the squashing
- Which infantry unit ID is invalid
- This points to where unit cleanup is failing

**Crash will reveal:**
- Is the infantry already destroyed but still in assignedInfantryList?
- Was unassignInfantry() never called?
- Is ObjectManager returning nullptr for valid IDs?

## Next Steps

1. **Run the game** in GameDebug mode
2. **Reproduce** the issue (QuantBot units moving around)
3. **Wait for crash** (should happen when tank drives over infantry)
4. **Capture crash log** with stack trace
5. **Analyze** to find where unit cleanup is broken

## Likely Root Causes

Based on crash logs in `documents/firing-at-nothing`:

1. **Infantry destruction** doesn't call `unassignInfantry()`
2. **Unit movement** creates ghost entries in tile lists
3. **Spatial grid** updates don't sync with tile lists
4. **ObjectManager** removes units but tiles keep stale IDs

The crash will tell us which one it is.

## Files Reverted

- `src/Tile.cpp` - reverted squash() to original version

