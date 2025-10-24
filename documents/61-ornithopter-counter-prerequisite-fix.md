# Ornithopter Counter Prerequisite Fix

**Date:** October 24, 2025  
**Version:** 0.98.6.2  
**Issue:** Ornithopter counter logic triggered without verifying prerequisites

## Summary

Fixed bug where QuantBot's ornithopter counter logic attempted to build rocket turrets without first ensuring all prerequisites (Windtrap, Radar, Construction Yard level 2) were met. The counter now proactively builds prerequisites when enemy ornithopters are detected.

## Problem

### Root Cause

The ornithopter counter logic (lines 1618-1642 in `QuantBot.cpp`) only checked for Construction Yard level 2, but **did not verify** the other prerequisites for rocket turrets:

**Rocket Turret Prerequisites (from `ObjectData.ini`):**
```ini
[Rocket-Turret]
TechLevel = 6
Prerequisite = Windtrap, Radar
UpgradeLevel = 2
```

### What Was Happening

**Old flow:**
1. Detect enemy ornithopters
2. Check if CY is level 2 → upgrade if not
3. Try to build rocket turret → **SILENTLY FAILS** if no Windtrap/Radar
4. Essential infrastructure checks come later in the build queue

**Result:**
- AI would attempt to build rocket turrets but couldn't
- No Windtrap or Radar would be prioritized
- AI was defenseless against ornithopters despite having the counter logic

### Example Scenario

```
Enemy has 3 ornithopters
AI has: CY level 2, but NO Windtrap, NO Radar
Counter logic triggers: "Build rocket turret!" 
Result: Nothing happens (isAvailableToBuild returns false)
AI continues with normal build order (refineries, barracks, etc.)
Ornithopters raid with impunity
```

## Solution

### New Logic Flow

Added prerequisite checks and proactive building:

1. **Detect enemy ornithopters** → Counter triggered
2. **Check CY level** → Upgrade to level 2 if needed
3. **Check Windtrap** → Build if missing (prerequisite)
4. **Check Radar** → Build if missing (prerequisite)
5. **Check power** → Ensure sufficient power
6. **Build rocket turret** → Only when ALL prerequisites met

### Code Changes

**File:** `src/players/QuantBot.cpp` (lines 1618-1656)

**Added:**
```cpp
// Check prerequisites for rocket turrets: Windtrap, Radar, CY level 2
bool hasWindtrap = itemCount[Structure_WindTrap] > 0;
bool hasRadar = itemCount[Structure_Radar] > 0;
```

**New conditional chain:**
```cpp
if (pBuilder->getCurrentUpgradeLevel() < 2) {
    // Upgrade CY first...
}
else if (!hasWindtrap && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
    // Build windtrap (prerequisite)
    itemID = Structure_WindTrap;
    logDebug("COUNTER-ORNITHOPTER: Building windtrap (prerequisite for rocket turrets) - enemy ornis: %d", enemyOrnithopterCount);
}
else if (!hasRadar && pBuilder->isAvailableToBuild(Structure_Radar) && getHouse()->hasPower()) {
    // Build radar (prerequisite)
    itemID = Structure_Radar;
    logDebug("COUNTER-ORNITHOPTER: Building radar (prerequisite for rocket turrets) - enemy ornis: %d", enemyOrnithopterCount);
}
else if (pBuilder->isAvailableToBuild(Structure_RocketTurret) && ...) {
    // All prerequisites met - build rocket turret
    itemID = Structure_RocketTurret;
    logDebug("COUNTER-ORNITHOPTER: Building rocket turret - enemy ornis: %d, our turrets: %d, target: %d", ...);
}
```

## Behavior After Fix

### Scenario 1: Early Game Ornithopter Raid
```
Turn 1: Enemy builds 2 ornithopters
AI state: CY level 1, no Windtrap, no Radar

Turn 2: Counter triggered
Action: Upgrade CY to level 2
Log: "COUNTER-ORNITHOPTER: Upgrading construction yard to level 2"

Turn 3: CY now level 2, still no Windtrap
Action: Build Windtrap
Log: "COUNTER-ORNITHOPTER: Building windtrap (prerequisite for rocket turrets) - enemy ornis: 2"

Turn 4: Windtrap complete, no Radar
Action: Build Radar
Log: "COUNTER-ORNITHOPTER: Building radar (prerequisite for rocket turrets) - enemy ornis: 2"

Turn 5: All prerequisites met
Action: Build rocket turret #1
Log: "COUNTER-ORNITHOPTER: Building rocket turret - enemy ornis: 2, our turrets: 0, target: 4"
```

### Scenario 2: Mid-Game Ornithopter Threat
```
AI state: CY level 2, Windtrap exists, Radar exists
Enemy builds 3 ornithopters

Counter triggered:
Action: Build rocket turret immediately (all prerequisites met)
Log: "COUNTER-ORNITHOPTER: Building rocket turret - enemy ornis: 3, our turrets: 0, target: 6"
```

## Priority Order

The ornithopter counter now follows this strict priority:

1. **Repair CY** (if damaged)
2. **Upgrade CY to level 2** (if not upgraded)
3. **Build Windtrap** (if missing)
4. **Build Radar** (if missing AND sufficient power)
5. **Build Rocket Turret** (if all prerequisites met)

This ensures the AI **will always** build the necessary infrastructure when threatened by ornithopters, rather than failing silently.

## Debug Logging

New log messages help track the counter's progression:

```
[DEBUG] COUNTER-ORNITHOPTER: Upgrading construction yard to level 2
[DEBUG] COUNTER-ORNITHOPTER: Building windtrap (prerequisite for rocket turrets) - enemy ornis: 2
[DEBUG] COUNTER-ORNITHOPTER: Building radar (prerequisite for rocket turrets) - enemy ornis: 2
[DEBUG] COUNTER-ORNITHOPTER: Building rocket turret - enemy ornis: 2, our turrets: 0, target: 4
```

## Testing Recommendations

1. **Early game test:**
   - Start AI with basic base (CY level 1, no Windtrap, no Radar)
   - Spawn 2-3 enemy ornithopters nearby
   - Verify AI builds: CY upgrade → Windtrap → Radar → Rocket Turrets

2. **Mid-game test:**
   - AI already has Windtrap + Radar + CY level 2
   - Spawn enemy ornithopters
   - Verify AI immediately builds rocket turrets

3. **No power test:**
   - AI has Windtrap + CY level 2, but NO Radar and low power
   - Spawn enemy ornithopters
   - Verify AI doesn't try to build Radar without power
   - Verify AI builds more Windtraps first

4. **Counter scaling:**
   - Spawn 1, 2, 3, 5 ornithopters incrementally
   - Verify AI builds 2 rocket turrets per ornithopter

## Related Files

- `src/players/QuantBot.cpp` - Ornithopter counter logic
- `config/ObjectData.ini` - Rocket turret prerequisites definition
- `documents/32-ornithopter-counter-priority-fix.md` - Original counter implementation

## Update: Construction Yard Double Upgrade Fix

**Issue:** Construction Yard needs to upgrade TWICE (level 0 → 1 → 2) but the waiting period between upgrades wasn't being logged, making it appear "stuck".

**Additional Fix:**
- Added explicit `isUpgrading()` check with logging
- Shows current upgrade progress: "level X → X+1"
- Makes it clear when waiting for upgrade to complete

**New Logic:**
```cpp
if (pBuilder->getCurrentUpgradeLevel() < 2) {
    if (pBuilder->getHealth() < pBuilder->getMaxHealth() && !pBuilder->isRepairing()) {
        // Repair first
        doRepair(pBuilder);
    }
    else if (pBuilder->isUpgrading()) {
        // NEW: Explicit waiting state with logging
        logDebug("COUNTER-ORNITHOPTER: Waiting for construction yard upgrade to complete (current level: %d → %d)", 
            pBuilder->getCurrentUpgradeLevel(), pBuilder->getCurrentUpgradeLevel() + 1);
    }
    else if (pBuilder->getHealth() >= pBuilder->getMaxHealth()) {
        // Start upgrade (will be called twice: 0→1, then 1→2)
        doUpgrade(pBuilder);
        logDebug("COUNTER-ORNITHOPTER: Upgrading construction yard (level %d → %d, target: level 2)", 
            pBuilder->getCurrentUpgradeLevel(), pBuilder->getCurrentUpgradeLevel() + 1);
    }
}
```

**Expected Log Sequence:**
```
[DEBUG] COUNTER-ORNITHOPTER: Upgrading construction yard (level 0 → 1, target: level 2)
[DEBUG] COUNTER-ORNITHOPTER: Waiting for construction yard upgrade to complete (current level: 0 → 1)
[DEBUG] COUNTER-ORNITHOPTER: Waiting for construction yard upgrade to complete (current level: 0 → 1)
...
[DEBUG] COUNTER-ORNITHOPTER: Upgrading construction yard (level 1 → 2, target: level 2)
[DEBUG] COUNTER-ORNITHOPTER: Waiting for construction yard upgrade to complete (current level: 1 → 2)
[DEBUG] COUNTER-ORNITHOPTER: Waiting for construction yard upgrade to complete (current level: 1 → 2)
...
[DEBUG] COUNTER-ORNITHOPTER: Building windtrap (prerequisite for rocket turrets) - enemy ornis: 2
```

## Status

✅ **Fixed** - Prerequisite checks added  
✅ **Fixed** - Double upgrade logging added  
✅ **Compiled** - No errors  
⏳ **Testing** - Needs gameplay verification  

## Expected Impact

- ✅ AI will now **reliably** build rocket turrets when threatened
- ✅ AI builds prerequisites **proactively** when ornithopters detected
- ✅ Clear visibility into upgrade progress (no more appearing "stuck")
- ✅ No more silent failures or ignored threats
- ✅ Better defensive gameplay for AI players

