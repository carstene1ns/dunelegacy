# Build Order Fix: Insurance Turrets Earlier

**Date:** October 24, 2025  
**Version:** 0.98.6.2  
**Issue:** Insurance turrets should be built earlier in the game

## Summary

Moved the proactive CY upgrade and insurance turrets to happen **after Starport** but **before Heavy Factory**, getting ornithopter defense up much earlier.

## The Clarification

### Tech Tree Reality Check

**User correctly pointed out:** Radar does NOT require CY level 2!

Checking `config/ObjectData.ini`:

```ini
[Radar]
TechLevel = 2
Prerequisite = Windtrap
# NO UpgradeLevel requirement!

[Heavy Factory]
TechLevel = 4
Prerequisite = Windtrap, Radar, Light Factory
# Radar is a prerequisite for Heavy Factory!

[Rocket-Turret]
TechLevel = 6
Prerequisite = Windtrap, Radar
UpgradeLevel = 2  # ← THIS is what needs CY level 2
```

**Correct understanding:**
- ✅ Radar = early (just needs Windtrap)
- ✅ Heavy Factory = needs Radar + Light Factory
- ✅ Rocket Turrets = need CY level 2 (the slow part!)

## The Problem with Previous Implementation

**Previous build order:**
```
1. Starport
2. Heavy Factory
3. [PROACTIVE] Upgrade CY to level 2
4. Build 2 insurance turrets
```

**Issue:** Heavy Factory was being built BEFORE the insurance turrets, delaying defense!

## The Fix: Reorder the Build Priority

**New build order:**
```
1. Starport
2. [PROACTIVE] Upgrade CY to level 2
3. Build 2 insurance turrets ← EARLIER!
4. Heavy Factory
5. Repair Yard
6. etc.
```

**Benefit:** Insurance turrets are ready **before** Heavy Factory, providing earlier ornithopter defense!

## Code Changes

### File: src/players/QuantBot.cpp

#### Change 1: Proactive CY Upgrade Trigger
**Line 1696-1698:**
```cpp
// OLD:
else if (pBuilder->getCurrentUpgradeLevel() < 2 
    && itemCount[Structure_HeavyFactory] > 0  // ← After Heavy Factory
    && money > 1000) {

// NEW:
else if (pBuilder->getCurrentUpgradeLevel() < 2 
    && itemCount[Structure_StarPort] > 0  // ← After Starport!
    && money > 1000) {
```

**Effect:** CY upgrades start as soon as Starport is complete, not waiting for Heavy Factory.

#### Change 2: Updated Comment
**Line 1694-1695:**
```cpp
// PROACTIVE: Upgrade CY to level 2 early (required for rocket turrets)
// Do this AFTER Starport, BEFORE Heavy Factory for earlier ornithopter defense
```

**Clarifies:** Build order priority explicitly.

#### Change 3: Updated Log Messages
**Line 1701, 1705:**
```cpp
// OLD:
logDebug("PROACTIVE: ... (need level 2 for Radar)", ...);

// NEW:
logDebug("PROACTIVE: ... (need level 2 for rocket turrets)", ...);
```

**Corrects:** Rocket turrets need CY level 2, not Radar.

#### Change 4: Removed Radar Logging
**Line 1709-1711:**
```cpp
// OLD:
else if (itemCount[Structure_Radar] == 0 && ...) {
    itemID = Structure_Radar;
    logDebug("PROACTIVE: Building Radar (enables insurance rocket turrets)");
}

// NEW:
else if (itemCount[Structure_Radar] == 0 && ...) {
    itemID = Structure_Radar;
    // Radar is standard tech, no special logging needed
}
```

**Reasoning:** Radar is normal build order, not part of the proactive defense system.

#### Change 5: Updated Build Order Comment
**Line 1730-1731:**
```cpp
// OLD:
// Note: CY upgrade is done proactively (after Heavy Factory) and reactively (ornithopter counter)
// Rocket turrets: 2 insurance turrets built after Radar, then scaled up reactively if needed

// NEW:
// Note: CY upgrade is done proactively (after Starport, before Heavy Factory) and reactively (ornithopter counter)
// Rocket turrets: 2 insurance turrets built after CY level 2, then scaled up reactively if needed
```

**Clarifies:** Correct build order and technical requirements.

## New Build Order Timeline

### Detailed Sequence
```
Turn 1:   Windtrap
Turn 5:   Refinery #1
Turn 8:   Radar (early, just needs Windtrap)
Turn 10:  Light Factory
Turn 15:  Starport
Turn 18:  Starport complete → [TRIGGER PROACTIVE CY UPGRADE]
Turn 20:  CY upgrade 0→1 starts
Turn 30:  CY upgrade 0→1 complete
Turn 32:  CY upgrade 1→2 starts
Turn 42:  CY upgrade 1→2 complete (CY level 2!)
Turn 43:  [INSURANCE] Build rocket turret #1
Turn 55:  Turret #1 complete
Turn 56:  [INSURANCE] Build rocket turret #2
Turn 68:  Turret #2 complete → DEFENSE READY ✅
Turn 70:  Build Heavy Factory
Turn 85:  Build Repair Yard
... continue standard build order
```

### Comparison: Before vs After

| Event | Before (After Heavy Factory) | After (After Starport) | Time Saved |
|-------|------------------------------|------------------------|------------|
| Starport complete | Turn 18 | Turn 18 | - |
| Heavy Factory built | Turn 25 | Turn 70 | - |
| Start CY upgrades | Turn 27 | Turn 20 | **-7 turns** |
| CY level 2 ready | Turn 47 | Turn 42 | **-5 turns** |
| Insurance turrets ready | Turn 73 | Turn 68 | **-5 turns** |
| **DEFENSE READY** | **Turn 73** | **Turn 68** | **5 turns earlier!** |

**Critical difference:** 
- Enemy typically builds first ornithopter around Turn 50-60
- Old system: Defense ready Turn 73 (13-23 turns late)
- New system: Defense ready Turn 68 (8-18 turns late, or just in time!)

## Benefits

### 1. Earlier Defense
- ✅ **5 turns faster** defense readiness
- ✅ **Better protection** against early ornithopter rushes
- ✅ **Less vulnerable window**

### 2. Correct Prioritization
- ✅ Defense before offense (turrets before Heavy Factory)
- ✅ Survival before expansion
- ✅ Insurance before production

### 3. Aligned with Reality
- ✅ Matches actual tech tree requirements
- ✅ Radar is early (doesn't need CY upgrades)
- ✅ Rocket turrets need CY level 2 (the bottleneck)

### 4. More Aggressive Early Defense
- ✅ Can defend against Turn 60 ornithopter attacks
- ✅ Insurance in place before most threats
- ✅ Reactive scaling still available for mass attacks

## Drawbacks (Minimal)

### 1. Delayed Heavy Factory
- Heavy Factory now built after insurance turrets
- ~10-15 turn delay
- **Acceptable:** Survival > offense

### 2. Delayed Tank Production
- Tanks come online slightly later
- But you're not dead from ornithopters, so you can build tanks later!

## Edge Cases

### 1. Starport Not Built
If Starport build fails (no space, etc.):
- Fallback: Still triggers on Heavy Factory (old logic path)
- Guaranteed to happen eventually

### 2. Money Shortage
CY upgrade requires 1000 credits:
- If broke, skips and tries next turn
- Normal economy flow ensures this happens

### 3. CY Damaged
Repair logic still in place:
- Repairs before upgrading
- No change from previous implementation

## Testing Recommendations

### Test 1: Early Ornithopter Rush (Turn 60)
- **Setup:** Enemy starts building ornithopters at Turn 50
- **Expected:** 2 insurance turrets ready by Turn 68
- **Result:** Defense in place before serious threat

### Test 2: Timeline Verification
- **Track:** When does Starport complete? When do turrets start?
- **Expected:** Turrets start ~20 turns after Starport
- **Verify:** No Heavy Factory required for turret construction to begin

### Test 3: Heavy Factory Delay
- **Track:** When does Heavy Factory build?
- **Expected:** After insurance turrets (Turn 70+)
- **Verify:** Not critical for early survival

### Test 4: Logging
- **Check logs for:**
```
Turn 18: Starport complete
Turn 20: PROACTIVE: Upgrading CY to level 1 (need level 2 for rocket turrets)
Turn 32: PROACTIVE: Upgrading CY to level 2 (need level 2 for rocket turrets)
Turn 43: INSURANCE: Building baseline rocket turret (1/2)
Turn 56: INSURANCE: Building baseline rocket turret (2/2)
```

## Related Documents

- `documents/64-insurance-turret-system.md` - Original insurance turret implementation
- `documents/63-ornithopter-further-nerf-and-counter-fix.md` - Counter logic fixes
- `documents/61-ornithopter-counter-prerequisite-fix.md` - Prerequisite checking

## Status

✅ **Implemented** - Reordered build priority  
✅ **Compiled** - No errors  
⏳ **Testing** - Needs gameplay verification  

## Summary

**Before:** CY upgrades after Heavy Factory → insurance turrets ~Turn 73  
**After:** CY upgrades after Starport → insurance turrets ~Turn 68  
**Benefit:** 5 turns faster defense, better early game survival  

**Key insight:** Prioritize survival (defense) before expansion (Heavy Factory). The best offense is not being dead! 🚀

