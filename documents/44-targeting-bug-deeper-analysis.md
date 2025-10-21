# Targeting Bug - Deeper Analysis

## User's Question

> "review the path change a bit more once the unit reaches the path destination if the unit has been destroyed, wouldn't it just find a new target?"

**Answer**: YES, you're absolutely correct!

## The Targeting Logic (Works Correctly)

**File**: `src/units/UnitBase.cpp:447-453`

```cpp
void UnitBase::engageTarget() {
    if(target && (target.getObjPointer() == nullptr)) {
        // the target does not exist anymore
        releaseTarget();
        return;
    }
    // ... more checks ...
}
```

This **should** handle destroyed targets properly:
1. `target.getObjPointer()` returns `nullptr` when target is destroyed
2. Unit calls `releaseTarget()`
3. Unit goes back to guard point
4. `targeting()` should acquire new target

## What We've Checked

### ✅ Target Destruction Handling
- Lines 449-453: Detects `nullptr` targets
- Lines 455-459: Detects inactive targets
- Lines 461-465: Detects unattackable targets
- Lines 467-471: Detects out-of-range targets

### ✅ Range Checking
- Line 509: `if(targetDistance > getWeaponRange())`
- Line 558: `if(targetDistance <= getWeaponRange())` for attackPos
- Proper range checks for both target objects and positions

### ✅ Attack Position Logic
- Line 551-571: Units attacking `attackPos` do range checks
- Line 1204: `setTarget()` clears `attackPos`
- Line 206-213: Units fire at `attackPos` if no target exists

## What Doesn't Make Sense

**The symptom**: "missiles flying off the map and at buildings etc."

**Why this is confusing**:
1. Target nullptr check should prevent firing at destroyed units
2. Range checks should prevent firing beyond weapon range
3. Path recalculation shouldn't affect firing logic

## Possible Explanations

### 1. Visual Bug (Not Actual Targeting)
- Missiles rendering in wrong location?
- Bullets created with wrong coordinates?
- Camera/viewport issue?

### 2. Race Condition
- Target destroyed between check and fire?
- Multiple game cycles creating stale state?
- Timing issue with path vs targeting?

### 3. attackPos Not Cleared
Line 436-444: `releaseTarget()` does NOT clear `attackPos`

```cpp
void UnitBase::releaseTarget() {
    if(forced == true) {
        guardPoint = location;
    }
    setDestination(guardPoint);
    // attackPos is NOT cleared here!
}
```

**Scenario**:
1. Unit has `attackPos` from old command
2. Unit acquires target object (clears `attackPos`)
3. Target destroyed → `releaseTarget()` called
4. `attackPos` still has old value?

**But**: Line 1204 in `setTarget()` clears `attackPos`, so this shouldn't happen.

### 4. The Path Change IS The Issue (But Not How We Thought)

**Lines 486-492**:
```cpp
} else if(movementDistance > 1) {
    destination = targetLocation;
    if(!pathRequestQueued) {
        enqueuePathRequest();
    }
}
```

**Hypothesis**: Units keep moving toward `destination` even after target is destroyed?
- Unit has old path toward target location A
- Target moves to location B (1-3 tiles away)
- Code updates `destination` to B
- Target gets destroyed
- Unit still has `destination` set to B
- Unit moves to B and... then what?

**But**: Once at destination, `targeting()` should find new target.

### 5. HUNT Mode Behavior

In HUNT mode, units chase targets across the entire map. Maybe:
- Units in HUNT mode have different range/targeting rules?
- `isInAttackRange()` behaves differently for HUNT?
- Something about HUNT + path recalculation?

### 6. It's Not Path Recalculation At All

Maybe the bug is from:
- Attack squad limit removal (more units = different behavior)
- Build order changes (AI sends units differently)
- Something in QuantBot AI logic
- Pre-existing bug now more visible

## What We Need To Know

**Questions for user**:
1. **What exactly are you seeing?**
   - Are missiles visually going off-screen?
   - Are units turning and firing at nothing?
   - Are they targeting buildings they shouldn't?
   - Is it launchers specifically (as before)?

2. **When does it happen?**
   - During AI attacks?
   - After enemies are destroyed?
   - Specific unit types?
   - Specific game situations?

3. **Test result from reverting Tile::squash()**
   - Did reverting squash fix it?
   - Still broken?

4. **Can you reproduce it reliably?**
   - Specific map?
   - Specific units?
   - Specific actions?

## Next Steps

### If Tile::squash() revert didn't fix it:
1. **Revert path recalculation change** (734c635)
   - File: `src/units/UnitBase.cpp:476-497`
   - Change back to "clear path if > 1 tile"
   - Test if targeting bug goes away

### If path revert doesn't fix it:
2. **Revert attack squad limit** (b8b1a23)
   - File: `src/players/QuantBot.cpp:1997`
   - Restore `maxAttackSquadSize = 70` limit
   - Test if targeting bug goes away

### If neither fixes it:
3. **Full git bisect**
   - Between `origin/release-0.98.4` and `HEAD`
   - Find exact commit that introduced bug
   - More methodical approach

### Or:
4. **Add diagnostic logging**
   - Log when units fire
   - Log target coordinates
   - Log when `attack()` is called with `nullptr` target
   - Log `attackPos` values
   - Capture exactly what's happening

## My Suspicion

I think the path recalculation change (734c635) is still the most likely culprit, but NOT for the reason I initially thought. 

**New hypothesis**: The change affects timing/state in a subtle way that exposes a different bug elsewhere in the code (possibly in QuantBot AI logic or HUNT mode behavior).

## Recommendation

Ask user for more specific details about what they're seeing, then either:
- Revert path recalculation (most likely)
- Add diagnostic logging (most informative)
- Git bisect (most methodical)

