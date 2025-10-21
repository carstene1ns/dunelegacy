# Critical Discovery: Tile::squash() Fix Causes Firing Bug

## Timeline

1. **Original bug**: `Tile::squash()` had iterator invalidation → crash
2. **Fix applied**: Copy IDs to vector, iterate safely → no crash
3. **New bug appeared**: Units firing at nothing off-map
4. **Revert squash fix**: Back to original code
5. **Result**: Firing bug GONE, but crash is BACK

## Crash Details (Latest)

**File**: `Tile.cpp:643` in `Tile::squash()`  
**Called from**: `UnitBase.cpp:648` in `UnitBase::move()`  
**Address**: `0x8c` (140 bytes from NULL)  
**Type**: NULL pointer dereference / iterator invalidation

## The Paradox

**The squash fix (commit 40-tile-squash-iterator-invalidation.md) was:**
```cpp
void Tile::squash() const {
    if (!hasInfantry()) return;
    if (currentGame == nullptr) return; // Safety check

    // Copy IDs to avoid iterator invalidation
    std::vector<Uint32> infantryIDs;
    infantryIDs.reserve(assignedInfantryList.size());
    for (Uint32 id : assignedInfantryList) {
        infantryIDs.push_back(id);
    }

    // Now safely iterate and squash
    for (Uint32 id : infantryIDs) {
        InfantryBase* current = static_cast<InfantryBase*>(
            currentGame->getObjectManager().getObject(id));
        if (current != nullptr) {
            current->squash();
        }
    }
}
```

**This fix:**
- ✅ Prevents the crash
- ❌ Somehow causes units to fire at invalid targets

**The original buggy code:**
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

**This code:**
- ❌ Crashes (iterator invalidation)
- ✅ Does NOT cause firing bug

## Why Would The Fix Cause Firing?

### Hypothesis 1: Squashing Affects Targeting
- When infantry get squashed, they're destroyed
- Destruction might affect targeting/pathing in nearby units
- The fix might process squashing in a different order?
- Or different timing?

### Hypothesis 2: NULL Check Changes Behavior
The fix added:
```cpp
if (current != nullptr) {
    current->squash();
}
```

The original had:
```cpp
if(current == nullptr)
    continue;

current->squash();
```

**Are these equivalent?** YES, they should be identical.

### Hypothesis 3: Iterator Behavior Side Effect
The original code:
- Iterates while items are being removed
- Iterator becomes invalid, but loop continues
- **Might accidentally skip some infantry?**
- Or process them in weird order?

The fixed code:
- Copies all IDs first
- Processes ALL infantry, even if some are destroyed
- **Might squash already-destroyed infantry?**

### Hypothesis 4: assignedInfantryList Corruption
If the original bug wasn't *just* iterator invalidation, but also:
- List corruption
- Stale IDs in list
- Destroyed units not removed from list

Then the "fix" might:
- Process ALL stale IDs (including destroyed units)
- Call `squash()` on units that have corrupt data
- Corrupt data includes invalid `target` or `attackPos`?

## The Real Bug Might Be...

**Units are not being properly removed from `assignedInfantryList` when destroyed!**

If that's true:
- Original code: Iterator invalidation *hides* the stale ID problem (crashes before it matters)
- Fixed code: Processes stale IDs → tries to squash destroyed units → corrupt targeting data?

## What We Need To Check

1. **When infantry are destroyed, are they removed from `assignedInfantryList`?**
   - File: `src/units/InfantryBase.cpp` destructor
   - Or wherever units unassign from tiles

2. **When `getObjectManager().getObject(id)` returns `nullptr`, why?**
   - ID is invalid?
   - Object was destroyed?
   - Should that ID still be in `assignedInfantryList`?

3. **Does `InfantryBase::squash()` modify targeting of other units?**
   - File: `src/units/InfantryBase.cpp`
   - What does `squash()` actually do?

## Immediate Options

### Option A: Fix The Root Cause
Find why destroyed infantry IDs remain in `assignedInfantryList` and fix that.

**Pros**: Solves both bugs  
**Cons**: Might be complex, might take time

### Option B: Hybrid Fix
Keep the iterator safety, but skip IDs that return nullptr:
```cpp
for (Uint32 id : infantryIDs) {
    InfantryBase* current = static_cast<InfantryBase*>(
        currentGame->getObjectManager().getObject(id));
    if (current == nullptr) {
        continue; // Skip destroyed units
    }
    
    // But what if current->isActive() == false?
    // Or current's internal data is corrupt?
    if (!current->isActive()) {
        continue;
    }
    
    current->squash();
}
```

**Pros**: Might prevent both bugs  
**Cons**: Might still have firing bug if issue is elsewhere

### Option C: Don't Call Squash On Certain Conditions
```cpp
for (Uint32 id : infantryIDs) {
    InfantryBase* current = static_cast<InfantryBase*>(
        currentGame->getObjectManager().getObject(id));
    if (current == nullptr || !current->isActive() || current->isDestroyed()) {
        continue;
    }
    current->squash();
}
```

### Option D: Accept The Crash, Fix Firing Bug Source
Revert to original buggy squash, find why units fire off-map via other means.

**Pros**: Might be unrelated bugs  
**Cons**: Game crashes are worse than visual bugs

## Recommendation

1. **First**: Check `InfantryBase::squash()` to see what it does
2. **Second**: Check how infantry unassign from tiles
3. **Third**: Add diagnostics to the "fixed" squash to see what's happening

We need to understand WHY the fix causes firing bugs before we can create a proper solution.

## Next Step

User should tell us which approach to take, or we should investigate the root cause.

