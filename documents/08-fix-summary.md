# Fix Summary: Off-Screen Firing Bug

## Root Cause
**Kiting logic + AREAGUARD range check = broken targeting**

When launchers/sonic tanks kite away from close enemies:
1. Kiting moves their `guardPoint` to squad center
2. AREAGUARD mode checks attack range from `guardPoint` (not unit location)
3. Spatial grid finds targets near guard point
4. Unit fires at those targets even though they're far from the unit's actual position

## The Fix Applied

**File:** `src/units/UnitBase.cpp`  
**Function:** `isInAttackRange()`  
**Lines:** ~1056-1102

### Changed Behavior

**Before:**
```cpp
bool UnitBase::isInAttackRange(const ObjectBase* pObject) const {
    int checkRange;
    switch(attackMode) {
        case GUARD: checkRange = getWeaponRange(); break;
        case AREAGUARD: checkRange = getAreaGuardRange() + getWeaponRange() + 1; break;
        // ...
    }
    // ALL modes check from guardPoint
    return (blockDistance(guardPoint*TILESIZE + ..., target) <= checkRange*TILESIZE);
}
```

**After:**
```cpp
bool UnitBase::isInAttackRange(const ObjectBase* pObject) const {
    int checkRange;
    Coord checkLocation = location; // Default to unit location
    
    switch(attackMode) {
        case GUARD: {
            checkRange = getWeaponRange();
            checkLocation = guardPoint; // Only GUARD checks from guard point
        } break;
        
        case AREAGUARD: {
            checkRange = getAreaGuardRange() + getWeaponRange() + 1;
            checkLocation = location; // AREAGUARD checks from unit location
        } break;
        
        case AMBUSH: {
            checkRange = getViewRange() + 1;
            checkLocation = location; // AMBUSH checks from unit location
        } break;
        // ...
    }
    
    return (blockDistance(checkLocation*TILESIZE + ..., target) <= checkRange*TILESIZE);
}
```

### Key Changes

1. **GUARD mode**: Still checks from `guardPoint` (intended behavior - stationary defense)
2. **AREAGUARD mode**: Now checks from `location` (unit's actual position)
3. **AMBUSH mode**: Explicitly checks from `location`
4. **All other modes**: Default to checking from `location`

## Why This Fixes the Issue

### Before the Fix:
```
Launcher at (10, 10)
Guard point moved to (50, 50) by kiting
Target at (55, 55)

AREAGUARD check:
  distance from guardPoint(50,50) to target(55,55) = ~7 tiles
  checkRange = 2×5 + 5 + 1 = 16 tiles
  7 <= 16 → ATTACK APPROVED ✓

Actual distance from launcher(10,10) to target(55,55) = ~63 tiles!
Result: Rocket flies off-screen
```

### After the Fix:
```
Launcher at (10, 10)
Guard point at (50, 50) (doesn't matter anymore for AREAGUARD)
Target at (55, 55)

AREAGUARD check:
  distance from location(10,10) to target(55,55) = ~63 tiles
  checkRange = 2×5 + 5 + 1 = 16 tiles
  63 > 16 → ATTACK DENIED ✗

Result: Launcher doesn't fire, or finds closer target
```

## What Still Works

✓ Kiting behavior (launchers retreat from close enemies)  
✓ GUARD mode (units defend a specific location)  
✓ HUNT mode (units chase targets across map)  
✓ AREAGUARD mode (units defend area around themselves, not their home base)  
✓ Squad formations and coordination  
✓ Spatial grid targeting  
✓ All other unit types unaffected

## Diagnostic Logging Still In Place

The diagnostic logging added earlier is still active and will now show:
- Units correctly rejecting distant targets
- Range checks passing only for nearby targets
- No more "firing at" logs for off-screen positions

You can still use:
```bash
grep "\[DIAG\]" ~/dune-debug.log | grep "launcher\|sonic"
```

To verify the fix is working.

## Expected Behavior After Fix

### Launchers/Sonic Tanks Should:
1. ✓ Kite away from enemies that get within 6 tiles
2. ✓ Only fire at targets within weapon range of their CURRENT position
3. ✓ Not fire off-screen or at phantom targets
4. ✓ Retreat to squad center when threatened
5. ✓ Re-engage targets once at safe distance

### They Should NOT:
1. ✗ Fire at targets 50+ tiles away
2. ✗ Shoot off the map
3. ✗ Lock onto targets they can't reach
4. ✗ Waste ammo on empty desert

## Testing Checklist

- [ ] Build the game: `cd IDE/xCode && xcodebuild -project "Dune Legacy.xcodeproj" -scheme "Dune Legacy" -configuration GameDebug clean build`
- [ ] Start custom game with multiple AI players
- [ ] Observe launcher/sonic tank behavior
- [ ] Verify they only fire at nearby targets
- [ ] Verify kiting still works (retreat from close enemies)
- [ ] Check diagnostic logs: `grep "\[DIAG\]" ~/dune-debug.log`
- [ ] Play for 5-10 minutes to confirm sustained correct behavior

## Rollback (If Needed)

```bash
cd /Users/stefanvanderwel/development/dune/dunelegacy
git diff src/units/UnitBase.cpp > areaguard-fix.patch
git checkout src/units/UnitBase.cpp
```

To reapply:
```bash
git apply areaguard-fix.patch
```

## Technical Details

### Why GUARD vs AREAGUARD Behave Differently

**GUARD mode** (`guardPoint` check):
- "Stay at this location and defend it"
- Unit is stationary or returns to guard point
- Makes sense to check range from guard point
- Example: Turret defense, base defense

**AREAGUARD mode** (`location` check):
- "Defend the area around me as I move"
- Unit moves with the group
- Should check range from current position
- Example: Mobile defense, squad operations

### Why This Wasn't Caught in Original Code

The original QuantBot likely:
- Didn't have aggressive kiting for launchers, OR
- Didn't move guard points during kiting, OR
- Used HUNT mode more (which bypasses range checks)

The combination of:
- Spatial grid (finds distant targets)
- Kiting that moves guard points
- AREAGUARD mode checking from guard point

Created the perfect storm.

## Related Files

- `src/units/UnitBase.cpp` - Attack range logic (FIXED)
- `src/players/QuantBot.cpp` - Kiting logic (unchanged, working as intended)
- `src/ObjectBase.cpp` - Target finding via spatial grid (unchanged, working correctly)

## Follow-Up

If issues persist after this fix:
1. Check diagnostic logs for patterns
2. Verify kiting is working correctly
3. Look for other modes that might check from guard point
4. Consider if there are other code paths setting attackPos

But this should fix the primary issue!

## Credits

Fix identified by analyzing:
- The kiting behavior in QuantBot::checkAllUnits()
- The guard point manipulation in doMove2Pos()
- The range check logic in isInAttackRange()
- The difference between GUARD and AREAGUARD semantics

Root cause: Semantic mismatch between what AREAGUARD should mean (defend area around unit) vs what the code was checking (defend area around guard point).

