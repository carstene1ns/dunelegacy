# StarPort Frigate Bug - Root Cause and Fix

## Problem
Frigates were spawning successfully but immediately losing their target to the StarPort, causing them to not know where to deliver cargo.

## Root Cause

The excessive distance check added earlier to prevent units from targeting off-map locations was incorrectly affecting **unarmed units** like Frigates.

### The Bug Flow:
1. ✅ Frigate spawns at map edge `(25,0)` successfully
2. ✅ Frigate gets target set to StarPort at `(24,11)`  
3. ✅ Distance calculated: 11 tiles
4. ❌ **Frigates have weaponRange = 0** (they're cargo ships, not combat units)
5. ❌ Check: `targetDistance (11) > getWeaponRange() * 5` → `11 > 0` → TRUE
6. ❌ Frigate releases target immediately!
7. ❌ Without target, Frigate doesn't know where to deliver

### Code Location
`src/units/UnitBase.cpp:516`

**Before:**
```cpp
if(attackMode != HUNT && targetDistance > getWeaponRange() * 5) {
    SDL_Log("[ERROR] Unit %u releasing target: excessive distance...");
    releaseTarget();
    return;
}
```

This check was meant to catch combat units with bogus off-map targets, but it also caught unarmed units traveling legitimately long distances.

## Solution

Added `canAttackStuff` check to exempt unarmed units (Frigates, MCVs, Harvesters) from the excessive distance validation:

**After:**
```cpp
if(attackMode != HUNT && canAttackStuff && targetDistance > getWeaponRange() * 5) {
    SDL_Log("[ERROR] Unit %u releasing target: excessive distance...");
    releaseTarget();
    return;
}
```

### Why This Works:
- **Combat units** (`canAttackStuff = true`): Still protected from off-map targeting bugs
- **Unarmed units** (`canAttackStuff = false`): Can travel any distance to reach their objectives
  - Frigates traveling to StarPorts
  - MCVs moving to deployment locations
  - Harvesters going to spice fields

## Testing

### Diagnostic Logs Added:
1. **StarPort timer countdown** - tracks order progress
2. **Frigate spawning** - confirms creation and placement
3. **Spatial grid registration** - verifies Frigate is added to game world
4. **Destination setting** - shows flight path
5. **Target release errors** - caught the bug!

### Log Output Showing the Bug:
```
[StarPort] House 0: arrivalTimer reached 0! Attempting to spawn Frigate...
[StarPort] House 0: Calling owner->createUnit(Unit_Frigate)...
[StarPort] House 0: Frigate created successfully, objectID=4
[StarPort] House 0: Spawning Frigate at map edge (25,0) for StarPort at (24,11)
[SpatialGrid] registerObject SUCCESS for object 4 at tile (25,0), cell (6,0)
[StarPort] House 0: Frigate destination set to (25,11), distance: 11.0 tiles
[ERROR] Unit 4 (GUARD mode) releasing target 3: excessive distance 11.0 tiles (weapon range 0 tiles) - targeting bug!
```

The last line revealed the issue - Frigate with 0 weapon range was being caught by the excessive distance check.

## Files Modified
- `src/units/UnitBase.cpp` (line 516) - Added `canAttackStuff` condition
- `src/structures/StarPort.cpp` (lines 236-269) - Diagnostic logging
- `src/units/Frigate.cpp` (lines 69-70, 94-95) - Diagnostic logging  
- `src/SpatialGrid.cpp` (lines 73-114, 133-154) - Diagnostic logging

## Other Units Affected
This fix also benefits:
- **MCVs** - Can now travel long distances to deployment sites
- **Harvesters** - Can reach distant spice fields without target loss
- **Carryalls** - Won't lose pickup/delivery targets due to distance

## Status
✅ Bug identified via diagnostic logging
✅ Fix implemented - exempt unarmed units from excessive distance check
✅ Build succeeded
🔜 Needs in-game testing to confirm Frigates now deliver successfully

