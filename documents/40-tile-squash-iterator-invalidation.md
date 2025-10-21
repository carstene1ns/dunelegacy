# Tile::squash() Iterator Invalidation Crash Fix

## Crash Details

**Date**: October 22, 2025  
**Exception**: `EXC_BAD_ACCESS` (SIGSEGV)  
**Address**: `0xc2` (194 decimal) - NULL/corrupted pointer dereference  
**Build**: Debug (-O1)

## Stack Trace

```
Thread 0 Crashed:
0  Tile::squash() const                    Tile.cpp:643
1  UnitBase::move()                        UnitBase.cpp:648
2  UnitBase::update()                      UnitBase.cpp:1438
3  Game::processObjects()                  Game.cpp:290
```

## Root Cause: Iterator Invalidation

### The Bug

**File**: `src/Tile.cpp:638-651`

**Original (buggy) code**:
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

        current->squash();  // ← THIS DESTROYS THE UNIT!
    } while(iter != assignedInfantryList.end());
}
```

### What Happens

**Timeline of crash**:
1. Tank runs over infantry units on a tile
2. `Tile::squash()` called to crush all infantry
3. Loop starts: `iter = assignedInfantryList.begin()`
4. Get first infantry, increment iterator
5. Call `current->squash()` → **infantry unit is destroyed**
6. Destroyed unit removes itself from `assignedInfantryList`
7. **Iterator `iter` is now INVALID** (pointing to freed memory)
8. Loop continues: `while(iter != assignedInfantryList.end())` 
9. Accessing `iter` → **CRASH at address 0xc2**

### Why This Is Iterator Invalidation

**Container modification during iteration**:
```cpp
// We're iterating this list
for (auto iter = assignedInfantryList.begin(); ...) {
    current->squash();  
    // ^ This modifies assignedInfantryList by removing 'current'
    // ^ Now 'iter' points to freed memory!
}
```

This is a classic **iterator invalidation** bug. When you modify a container (add/remove elements) while iterating over it, existing iterators become invalid.

### Why It Didn't Always Crash

The bug is **intermittent** because:
- Only crashes when infantry are actually destroyed (not damaged)
- Memory layout determines if accessing invalid iterator crashes or not
- Compiler optimizations can mask/expose the bug
- Depends on how many infantry units are on the tile

## The Fix

**File**: `src/Tile.cpp:638-656`

```cpp
void Tile::squash() const {
    if (!hasInfantry()) return;
    if (currentGame == nullptr) return; // Safety check during cleanup

    // Copy IDs to avoid iterator invalidation when units are destroyed
    std::vector<Uint32> infantryIDs;
    infantryIDs.reserve(assignedInfantryList.size());
    for (Uint32 id : assignedInfantryList) {
        infantryIDs.push_back(id);
    }

    // Now safely iterate and squash
    for (Uint32 id : infantryIDs) {
        InfantryBase* current = dynamic_cast<InfantryBase*>(
            currentGame->getObjectManager().getObject(id));
        if (current != nullptr) {
            current->squash();
        }
    }
}
```

### Changes

**1. Copy IDs First**
```cpp
std::vector<Uint32> infantryIDs;
for (Uint32 id : assignedInfantryList) {
    infantryIDs.push_back(id);
}
```
- Makes a local copy of all infantry object IDs
- `assignedInfantryList` can be modified safely now
- We iterate over the local copy, not the original list

**2. Safe Iteration**
```cpp
for (Uint32 id : infantryIDs) {
    InfantryBase* current = dynamic_cast<InfantryBase*>(...);
    if (current != nullptr) {
        current->squash();
    }
}
```
- Even if `current->squash()` modifies `assignedInfantryList`, we don't care
- Our loop is iterating over `infantryIDs`, which never changes
- No iterator invalidation possible

**3. dynamic_cast Instead of static_cast**
```cpp
// BEFORE:
static_cast<InfantryBase*>(...)  // Undefined behavior if wrong type

// AFTER:
dynamic_cast<InfantryBase*>(...)  // Returns nullptr if wrong type
```
- Safer type checking
- Returns `nullptr` if object isn't actually `InfantryBase`
- Prevents crashes from type mismatches

**4. currentGame Null Check**
```cpp
if (currentGame == nullptr) return;
```
- Extra safety during game cleanup
- Prevents crashes if squash is called after game ends

## Why The Original Code Looked "Safe"

**Incrementing iterator before squashing**:
```cpp
InfantryBase* current = static_cast<InfantryBase*>(...getObject(*iter));
++iter;  // ← Increment BEFORE squashing

if(current == nullptr)
    continue;

current->squash();  // ← Iterator already moved
```

This looks like it should work because:
- We dereference `*iter` to get the ID
- We immediately increment `++iter` to next element
- THEN we squash the current one

**But it's still broken** because:
- The loop condition checks `iter != assignedInfantryList.end()`
- If squashing removes elements, `end()` might change
- The list structure itself might be reallocated
- `iter` might now point past the new `end()` or into freed memory

## Related Bugs

This is the **same class of bug** as:
- **Document 36**: Carryall crash (stale `target` pointer to destroyed RepairYard)
- Both involve **stale references** to destroyed objects
- Both require **defensive null checking**

## Pattern: Always Copy IDs Before Iteration

**When iterating over game objects**:
```cpp
// BAD - Iterator invalidation risk
for (auto* obj : objectList) {
    obj->doSomething();  // Might destroy 'obj' or modify 'objectList'
}

// GOOD - Copy IDs first
std::vector<Uint32> ids;
for (auto* obj : objectList) {
    ids.push_back(obj->getID());
}
for (Uint32 id : ids) {
    auto* obj = getObject(id);
    if (obj) obj->doSomething();
}
```

## Testing

**Scenario to reproduce**:
1. Build several infantry units (troopers, soldiers)
2. Group them on a single tile (5 infantry per tile)
3. Run them over with a tank
4. **Before fix**: Random crashes (iterator invalidation)
5. **After fix**: All infantry crushed safely, no crash

## Impact

**Before Fix**:
- Random crashes when tanks run over infantry
- Intermittent SIGSEGV during unit movement
- Hard to reproduce (timing/memory layout dependent)
- Game appears unstable

**After Fix**:
- Safe iteration even when units are destroyed
- No crashes when squashing infantry
- Predictable, stable behavior
- Proper defensive programming

## Summary

**Fixed**: Iterator invalidation crash in `Tile::squash()` when infantry units are destroyed

**Root Cause**: Modifying `assignedInfantryList` during iteration invalidated the iterator

**Solution**: Copy object IDs into temporary vector before iterating, preventing invalidation

**Bonus**: Added `dynamic_cast` for type safety and `currentGame` null check for cleanup safety

**Result**: Stable, crash-free infantry squashing

