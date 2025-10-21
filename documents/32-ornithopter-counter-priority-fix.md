# AI Attack & Defense Improvements

## Changes Summary

1. **Ornithopter Counter Priority Fix** - AI builds turrets matching enemy ornithopter count
2. **Attack Squad Limit Removed** - AI now sends all available units to attack (no 70-unit cap)

---

## Problem 1: Ornithopter Counter Not Working

AI was not building rocket turrets to counter enemy ornithopters, despite having logic for it. The AI would build only 2 rocket turrets and then keep building refineries even while under heavy ornithopter attack.

## Root Cause

**Build order priority issue** - The ornithopter counter logic was blocked by greedy economic checks:

### Original Priority Order (BROKEN):
```
1. First Windtrap
2. First Refinery / Harvester ratio refinery
3. Greedy refinery (if < 4 AND money < 4000) ❌ BLOCKS EVERYTHING
4. First StarPort
5. First Radar
6. Ornithopter counter (NEVER REACHED!) ❌
7. ... other buildings ...
```

**Why it failed:**
- Line 1604: `itemCount[Structure_Refinery] < 4 && money < 4000`
- When under ornithopter attack, you're spending money on defense/military
- Money drops below 4000
- AI keeps building refineries instead of rocket turrets
- Ornithopter counter logic at line 1616 never executes!

**Why only 2 turrets built:**
- Line 1672 had a separate "progressive turret building" capped at 2
- This was NOT the ornithopter counter - just generic defense
- That's why exactly 2 turrets were built regardless of enemy ornithopter count

## Solution

### New Priority Order (FIXED):
```
1. Ornithopter Counter (HIGH PRIORITY) ✓
   - If enemyOrnis > myTurrets:
     - Upgrade CY to level 2 (if needed)
     - Build rocket turret to match enemy count
2. First Windtrap
3. First Refinery / Harvester ratio refinery
4. Greedy refinery (if < 4 AND money < 4000)
5. First StarPort
6. First Radar
7. ... other buildings ...
```

**Removed conflicting logic:**
- Deleted duplicate CY upgrade check (line 1656-1670)
- Deleted "progressive turret building" cap at 2 (line 1672-1678)
- All turret building now controlled by ornithopter counter

## Code Changes

### src/players/QuantBot.cpp

**1. Moved ornithopter counter to TOP of build order** (line 1597):
```cpp
// CRITICAL: Counter enemy ornithopters with rocket turrets (HIGH PRIORITY)
if (enemyOrnithopterCount > itemCount[Structure_RocketTurret] && enemyOrnithopterCount > 0) {
    if (pBuilder->getCurrentUpgradeLevel() < 2) {
        // Upgrade CY first
        doUpgrade(pBuilder);
    }
    else if (pBuilder->isAvailableToBuild(Structure_RocketTurret)) {
        // Build rocket turret
        itemID = Structure_RocketTurret;
    }
}
// Essential infrastructure (moved below ornithopter counter)
else if (itemCount[Structure_WindTrap] == 0) { ... }
else if (itemCount[Structure_Refinery] < 4 && money < 4000) { ... }
// ... etc
```

**2. Removed duplicate/conflicting logic** (line 1655):
```cpp
// REMOVED: Duplicate CY upgrade check (was at line 1656-1670)
// REMOVED: Progressive turret building cap at 2 (was at line 1672-1678)
// Note: CY upgrade and rocket turret building are now handled by ornithopter counter above
```

## Expected Behavior

### Before (Broken):
```
Enemy has 5 ornithopters
AI builds 2 rocket turrets (progressive cap)
AI keeps building refineries (greedy check)
Base gets destroyed by ornithopters ❌
```

### After (Fixed):
```
Enemy has 5 ornithopters
AI upgrades CY to level 2 (if needed)
AI builds rocket turret #1 (enemyOrnis=5 > myTurrets=0)
AI builds rocket turret #2 (enemyOrnis=5 > myTurrets=1)
AI builds rocket turret #3 (enemyOrnis=5 > myTurrets=2)
AI builds rocket turret #4 (enemyOrnis=5 > myTurrets=3)
AI builds rocket turret #5 (enemyOrnis=5 > myTurrets=4)
AI now has 5 turrets matching 5 enemy ornithopters ✓
AI returns to economic expansion (refineries, etc.)
```

## Priority Logic Explanation

**Why ornithopter counter is now #1 priority:**
- **Survival > Economy**: Can't harvest if base is destroyed
- **Time-sensitive**: Ornithopters attack quickly, need immediate response
- **Tactical counter**: Rocket turrets are THE counter to air units
- **Reversible**: Can build economy later, but can't rebuild destroyed base

**What happens when no ornithopters:**
- Check fails immediately: `enemyOrnithopterCount > 0` is false
- Skips to next priority (Windtrap)
- Zero performance impact when not needed

## Testing Checklist

- [ ] AI upgrades CY when seeing enemy ornithopters
- [ ] AI builds rocket turrets matching enemy ornithopter count
- [ ] AI builds MORE than 2 turrets (old cap removed)
- [ ] AI returns to economic building after turrets built
- [ ] AI doesn't spam turrets when no ornithopters present
- [ ] Logs show "COUNTER-ORNITHOPTER" messages

## Log Output

Look for these in logs:
```
[QuantBot] COUNTER-ORNITHOPTER: Upgrading construction yard to level 2
[QuantBot] COUNTER-ORNITHOPTER: Building rocket turret - enemy ornis: 5, our turrets: 0
[QuantBot] COUNTER-ORNITHOPTER: Building rocket turret - enemy ornis: 5, our turrets: 1
[QuantBot] COUNTER-ORNITHOPTER: Building rocket turret - enemy ornis: 5, our turrets: 2
...
```

## Summary (Ornithopter Counter)

**Fixed:** Ornithopter counter now has **HIGHEST PRIORITY** in build order and will build turrets to **MATCH enemy ornithopter count** (not capped at 2).

**Result:** AI will properly defend against air attacks by building sufficient rocket turrets before economic expansion.

---

## Problem 2: Attack Squad Size Limited to 70 Units

The AI had an arbitrary limit of 70 units in attack squads. On large maps with many units, this prevented the AI from using its full military force, making it less aggressive and less challenging.

### Root Cause

**Hard-coded limit** in `attack()` method:

```cpp
int maxAttackSquadSize = 70; // max units that AI can send

// Later in the code:
if (attackSquadSize >= maxAttackSquadSize) {
    logDebug("Attacking with %d units", attackSquadSize);
    return; // Stop adding units - squad is full
}
```

**Why this was bad:**
- Large maps (128×128) can support 100+ military units
- AI would only send 70 units, leaving 30+ units idle
- Made AI less aggressive and easier to defeat
- No strategic reason for this limit

### Solution

**Removed the squad size limit entirely:**

```cpp
// BEFORE (Limited):
int maxAttackSquadSize = 70;
if (attackSquadSize >= maxAttackSquadSize) {
    return; // Stop at 70 units
}

// AFTER (Unlimited):
// Send all available military units to attack (no squad size limit)
doSetAttackMode(pUnit, HUNT);
attackSquadSize++;
```

### Changes Made

**File**: `src/players/QuantBot.cpp`

**Removed** (line 1959):
```cpp
int maxAttackSquadSize = 70; // DELETED
```

**Simplified** (lines 1995-1999):
```cpp
// OLD:
if (attackSquadSize >= maxAttackSquadSize) {
    logDebug("Attacking with %d units", attackSquadSize);
    return;
}
else {
    doSetAttackMode(pUnit, HUNT);
    attackSquadSize++;
}

// NEW:
// Send all available military units to attack (no squad size limit)
doSetAttackMode(pUnit, HUNT);
attackSquadSize++;
```

### Expected Behavior

**Before (Limited):**
```
AI has 100 military units
Only 70 units sent to attack
30 units remain idle at base
Enemy easily defends against 70 units
```

**After (Unlimited):**
```
AI has 100 military units
All 100 units sent to attack
Massive overwhelming force
Much more challenging to defend against
```

### Impact by Map Size

| Map Size | Typical Military Units | Before (70 cap) | After (No cap) |
|----------|------------------------|-----------------|----------------|
| 32×32    | 20-30 units           | All sent ✓      | All sent ✓     |
| 64×64    | 40-60 units           | All sent ✓      | All sent ✓     |
| 128×128  | 100+ units            | 70 sent (30% idle) ❌ | All sent ✓ |

### Notes

- This makes the AI more aggressive and challenging
- Combined with proper turret placement, creates better strategic gameplay
- The `attackSquadSize` counter still exists for logging/diagnostics
- Units excluded from attacks: Harvesters, MCVs, Carryalls, Ornithopters (have separate logic), Sandworms

## Summary (Attack Squad Limit)

**Fixed:** Removed arbitrary 70-unit cap on attack squads.

**Result:** AI now uses its full military force, making it more aggressive and challenging, especially on large maps.

---

## Overall Impact

These two changes together create a much more competent AI:
1. ✅ **Defends properly** - Builds turrets to counter air threats
2. ✅ **Attacks aggressively** - Sends all available units (no arbitrary limits)
3. ✅ **Prioritizes correctly** - Defense before economy when under threat
4. ✅ **Scales with map size** - Effective on both small and large maps

