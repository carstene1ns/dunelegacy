# Revert Tile::squash() Fix - Testing for Targeting Bug

## User Report

> "the bug hasn't been resolved, still missiles flying off the map and at buildings etc. can we try remove that infantry fix all together to see if it is the culprit - the squash. revert to previous code"

## The Symptom

**Units firing at nothing over long distances:**
- Missiles flying off the map
- Firing at buildings/locations where nothing exists
- Firing beyond weapon range

## Suspected Culprit: Tile::squash() Changes

**What was changed in commit 743f2d7:**

### BEFORE (Original):
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

### AFTER (Modified for iterator invalidation fix):
```cpp
void Tile::squash() const {
    if (!hasInfantry()) return;
    if (currentGame == nullptr) return; // NEW

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

## Why This Seemed Unrelated

**The squash() function:**
- Only runs when tanks run over infantry
- Only affects infantry units being crushed
- Doesn't touch targeting logic
- Doesn't touch weapon range checks
- Doesn't touch unit AI or movement

**The targeting bug:**
- Units firing at nothing
- Missiles going off-screen
- Firing beyond weapon range
- Seems completely unrelated to infantry squashing

## Possible Explanations If This IS The Cause

**Hypothesis 1: Timing/Ordering**
- Original code had iterator invalidation bug (would crash sometimes)
- But maybe crash was preventing something worse from happening?
- New code runs "correctly" but exposes a downstream bug?

**Hypothesis 2: Side Effects**
- Destroying infantry properly now triggers some other code path
- That code path has a bug related to targeting
- Very indirect connection

**Hypothesis 3: Coincidence**
- The targeting bug is from a different change
- It just happened to appear around the same time
- Reverting won't fix it

## What We Reverted

**Reverted**: Entire `Tile::squash()` function back to original
- Removed iterator invalidation fix
- Removed ID copying
- Removed currentGame null check
- Back to original implementation with known crash bug

## Testing Plan

**Test 1: Targeting Bug**
1. Run game with reverted code
2. Observe if units still fire at nothing
3. Check if missiles go off-screen
4. Verify weapon range behavior

**Expected results:**
- **If targeting bug is GONE**: squash() change was somehow responsible (surprising!)
- **If targeting bug REMAINS**: The bug is from a different change

**Test 2: Crash Bug**
1. Run tanks over groups of infantry
2. Try to trigger the iterator invalidation crash
3. See if game becomes unstable

**Expected results:**
- Might crash when running over infantry groups
- Original crash bug returns

## Other Suspects

If reverting squash() doesn't fix it, these are more likely culprits:

**1. Attack Squad Limit Removal** (commit b8b1a23)
- Removed 70-unit cap on attack squads
- More units attacking = more targeting
- Could expose targeting bugs

**2. QuantBot Improvements** (commits 9a8baae, e73948a)
- Major AI behavior changes
- Ornithopter attack logic
- Build order changes
- Could have targeting side effects

**3. Pathfinding Changes** (commits 9a0294d, b57716c)
- Path budget changes
- Path recalculation threshold (3 tiles)
- Could affect when units re-evaluate targets

**4. Original Remote Code**
- Bug might exist in origin/release-0.98.4
- Never noticed before
- Just more visible now with other changes

## Current State

**File**: `src/Tile.cpp:638-651`
**Status**: **REVERTED** to origin/release-0.98.4 version
**Known issues**: Iterator invalidation crash bug is back

## Next Steps

**Immediate**: Test if targeting bug is fixed
- If YES → investigate why squash() affected targeting (very weird!)
- If NO → revert a different commit and test again

**If targeting bug remains**: Investigate these areas:
1. `src/units/UnitBase.cpp` - targeting logic
2. `src/players/QuantBot.cpp` - AI attack commands
3. Weapon range checks
4. Target validation code

## Risk

**By reverting this:**
- ⚠️ Iterator invalidation crash bug is back
- ⚠️ Game might crash when tanks run over infantry
- ⚠️ Unstable behavior in combat

**Worth it if:**
- ✅ Fixes the targeting bug
- ✅ Helps us identify the real problem
- ✅ Temporary for debugging only

## Commit Message

```
Revert Tile::squash() changes to test if it's causing targeting bug

User report: Units firing at nothing, missiles off-screen
Suspected: Tile::squash() iterator invalidation fix
Reverted: Entire squash() function to original (origin/release-0.98.4)

This reintroduces the iterator invalidation crash bug, but allows us
to test if the squash changes were somehow causing targeting issues.

Testing: Run game, check if targeting bug is resolved
Risk: Game might crash when tanks run over infantry groups
```

## Summary

**Action**: Reverted `Tile::squash()` to original code
**Purpose**: Test if it's causing "firing at nothing" bug
**Risk**: Iterator invalidation crash returns
**Next**: Test and report if targeting bug is fixed

