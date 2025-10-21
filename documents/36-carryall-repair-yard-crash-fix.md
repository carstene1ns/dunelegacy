# Carryall/RepairYard NULL Pointer Crash Fix

## Crash Details

**Date**: October 22, 2025  
**Exception**: `EXC_BAD_ACCESS` (SIGSEGV)  
**Address**: `0x0000000000000110` (NULL pointer + offset)  
**Build**: Debug (-O1)  

## Stack Trace

```
Thread 0 Crashed:
0  RepairYard::unBook()                    RepairYard.h:41
1  UnitBase::setPickedUp(UnitBase*)        UnitBase.cpp:1150
2  GroundUnit::setPickedUp(UnitBase*)      GroundUnit.cpp:194
3  Carryall::pickupTarget()                Carryall.cpp:408
4  UnitBase::update()                      UnitBase.cpp:1436
5  Carryall::update()                      Carryall.cpp:114
6  Game::processObjects()                  Game.cpp:290
```

## Root Cause

**Scenario**: Carryall picks up a unit that was going to a RepairYard that has been destroyed.

### The Bug

**File**: `src/units/UnitBase.cpp:1149-1151`

```cpp
if(goingToRepairYard) {
    static_cast<RepairYard*>(target.getObjPointer())->unBook();  // CRASH!
}
```

**Problem**: The code assumes that if `goingToRepairYard` is true, the target RepairYard still exists. However:

1. A unit requests repair and sets `goingToRepairYard = true`
2. The RepairYard gets destroyed (by enemy attack)
3. The target pointer becomes NULL or invalid
4. A Carryall picks up the unit
5. `setPickedUp()` tries to call `unBook()` on NULL → **CRASH**

### Why This Happens

**Timeline of Events**:
```
1. Unit damaged → requests repair → goingToRepairYard = true
2. Carryall assigned to pick up unit
3. Enemy destroys RepairYard
4. Unit's target becomes NULL
5. Carryall picks up unit → setPickedUp() called
6. Code checks: if(goingToRepairYard) → TRUE
7. Code calls: target.getObjPointer()->unBook() → NULL dereference!
```

The `goingToRepairYard` flag is not automatically cleared when the target RepairYard is destroyed, creating a stale state.

## The Fix

**File**: `src/units/UnitBase.cpp:1149-1151`

```cpp
// BEFORE (Crashes):
if(goingToRepairYard) {
    static_cast<RepairYard*>(target.getObjPointer())->unBook();
}

// AFTER (Safe):
if(goingToRepairYard && target.getObjPointer() != nullptr) {
    static_cast<RepairYard*>(target.getObjPointer())->unBook();
}
```

**Change**: Added null check before dereferencing the target pointer.

## Why This Works

```cpp
if(goingToRepairYard && target.getObjPointer() != nullptr)
```

**Checks two conditions**:
1. `goingToRepairYard` → Unit was trying to go to a repair yard
2. `target.getObjPointer() != nullptr` → The repair yard still exists

**If both are true**: Call `unBook()` to decrement the repair yard's booking count  
**If target is NULL**: Skip the call safely (repair yard was destroyed, no need to unbook)

## Related Code

**RepairYard::unBook()** (`include/structures/RepairYard.h:41`):
```cpp
inline void unBook() { bookings--; }
```

This is what crashes when called on a NULL pointer - it tries to access the `bookings` member variable at offset 0x110 from NULL.

## Impact

**Before Fix**:
- Game crashes when a Carryall picks up a unit whose target RepairYard was destroyed
- Happens frequently in battles where RepairYards are destroyed
- Very frustrating for players - game crash during intense combat

**After Fix**:
- Carryall safely picks up unit even if RepairYard was destroyed
- Unit simply won't be delivered to repair (since target doesn't exist)
- Game continues without crash
- Proper defensive programming

## Testing

**Scenario to Test**:
1. Build a RepairYard
2. Damage a unit so it requests repair
3. The unit will be in state: `goingToRepairYard = true`
4. Order a Carryall to pick up the damaged unit
5. While Carryall is en route, destroy the RepairYard
6. Carryall picks up the unit
7. **Before fix**: Game crashes
8. **After fix**: Game continues, Carryall picks up unit successfully

## Why The Crash Log Helped

The Debug build with `-O1` optimization preserved:
- ✅ Full stack trace with line numbers
- ✅ Source file names
- ✅ Function names
- ✅ Inline function information

This made diagnosis trivial - we could see exactly:
- **Where**: `RepairYard::unBook()` line 41
- **How**: Called from `UnitBase::setPickedUp()` line 1150
- **Why**: NULL pointer at address 0x110

## Prevention

**Better practices for future**:
1. Always check pointer validity before dereferencing
2. Clear state flags (`goingToRepairYard`) when targets are destroyed
3. Use smart pointers or `ObjectPointer` with automatic invalidation
4. Add assertions in development builds: `assert(target.getObjPointer() != nullptr)`

## Alternative Fix (Not Chosen)

We could also clear `goingToRepairYard` when the RepairYard is destroyed:

```cpp
// In RepairYard::destroy()
for (auto* unit : units_booked_here) {
    unit->goingToRepairYard = false;
    unit->target = nullptr;
}
```

But this requires maintaining a list of all units with bookings, which is more complex. The null check is simpler and equally safe.

## Summary

**Fixed**: NULL pointer crash when Carryall picks up unit whose target RepairYard was destroyed.

**Solution**: Added null check before calling `unBook()` on target RepairYard.

**Result**: Game no longer crashes in this scenario, Carryall safely picks up unit even if repair target is gone.

