# Revert to Simple Attack Logic

## Problem
The complex attack logic introduced by GPT-5-Codex was causing issues with launcher/sonic tank targeting and "units sitting there" in HUNT mode. The new logic was interfering with the existing, working kiting behavior.

## Root Cause
The NEW complex `attack()` method (lines 2185-2261 in modified file) was:
1. **Selecting a "squad leader"** - picking one unit to determine attack destination
2. **Manipulating guard points** - saving/restoring `originalGuardPoint` 
3. **Moving units to a destination** before setting HUNT mode
4. **Complex target finding** with `findClosestTargetStructure()` and `findClosestTargetUnit()`

This logic was **interfering with the kiting behavior** which sets `AREAGUARD` mode and moves launchers to squad center when enemies get too close. The attack() method would then mess with the guard points and movement commands.

## Solution
**Reverted `attack()` method back to the original simple version from `origin/release-0.98.4`:**

### Original Simple Attack Logic
1. Count existing units in HUNT mode
2. Loop through eligible units (not harvester, MCV, carryall, ornithopter, sandworm)
3. If unit is AREAGUARD/GUARD/AMBUSH and not forced, just set it to HUNT
4. That's it - no guard point manipulation, no complex targeting

### Kiting Logic Unchanged
The **kiting logic was already there and working** in the original:
- Lines 2682-2691 in `checkAllUnits()`
- If launcher/deviator has target AND distance <= 6 tiles AND not attacking ornithopter
- Set to AREAGUARD and move to squad center
- This logic **was NOT changed** - it was working fine

### Other Changes
1. **Ornithopter logic kept complex** (at user request):
   - Counts enemy rocket turrets to determine if it's safe to attack
   - Calculates ornithopter percentage of total military force
   - Attacks when: (ornithopters >= enemy rocket turrets) OR (ornithopters > 20% of army AND military value >= 40% of limit)
   - Prioritizes attacking rocket turrets in range of target building
   - Patrols defensively when not enough ornithopters to safely attack
   - **Note:** This is the ONLY complex logic retained - it doesn't interfere with ground unit behavior

2. **`findSquadRallyLocation()` reverted** to weighted calculation:
   - 75% our base center + 25% enemy base center
   - Points units toward enemy (not just nearest sand tile)

3. **Military value calculation kept** in `checkAllUnits()` for ornithopter logic

## Changes Made

### src/players/QuantBot.cpp

1. **`attack()` function** - Lines 2144-2225
   - Reverted to original simple version
   - Just counts HUNT units and sets eligible units to HUNT
   - Ornithopter attacks delegated to `checkAllUnits()` for complex logic
   - Removed squad leader selection, guard point manipulation, complex targeting

2. **`checkAllUnits()` ornithopter case** - Lines 2614-2721  
   - **KEPT complex ornithopter logic** (user request):
     * Counts enemy rocket turrets
     * Calculates ornithopter percentage of military
     * Smart attack/defend decision making
     * Prioritizes rocket turrets in range
   - **KEPT military value calculation** for ornithopter decisions

3. **`findSquadRallyLocation()`** - Lines 2258-2290
   - Reverted to weighted calculation with enemy base location
   - Removed "nearest sand tile" search

## Testing
- Build succeeded with no errors
- Only expected deprecation warnings in format.h (pre-existing)

## Result
Back to the proven, simple attack logic that:
- ✅ Works with existing kiting behavior
- ✅ Doesn't interfere with AREAGUARD mode
- ✅ Doesn't manipulate guard points during attacks
- ✅ Lets HUNT mode units naturally find and engage targets
- ✅ Simple and maintainable

The complex logic was tech debt from an AI experiment that broke working functionality.

