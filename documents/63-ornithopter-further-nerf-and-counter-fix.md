# Ornithopter Further Nerf and Counter Logic Fix

**Date:** October 24, 2025  
**Version:** 0.98.6.2  
**Issue:** Ornithopters still too fast, counter logic using incorrect calculation

## Summary

Two critical changes to address persistent ornithopter dominance:
1. **Speed nerf:** Reduced ornithopter speed from 18.0 → 17.0
2. **Counter logic fix:** Changed from summing all enemy ornithopters to using maximum from single enemy house

## Change 1: Ornithopter Speed Reduction

### The Problem
Even at speed 18.0, ornithopters were still "ruining everyone's day" - too fast for effective counter-play.

### The Fix
**File:** `config/ObjectData.ini`

```ini
[Ornithopter]
MaxSpeed = 17.0  # Was 18.0
```

### Speed Progression
- **Original:** 22.0 (impossible to catch)
- **First nerf:** 18.0 (catchable but still dominant)
- **Second nerf:** 17.0 (more balanced)

### New Speed Comparison

| Unit/Bullet | Speed | Can Catch Orni? |
|-------------|-------|-----------------|
| **Ornithopter** | **17.0** | - |
| **Turret Rocket** | **20.0** | ✅ YES (3 tiles/sec advantage) |
| Launcher Rocket | 17.5 | ⚠️ Marginal (0.5 tiles/sec) |
| Tank | 4.5 | ❌ No |
| Quad | 5.12 | ❌ No |

**Benefit:** Turret rockets now have a **3 tiles/second** closing speed advantage (was 2), making them more effective at catching and destroying ornithopters.

## Change 2: Counter Logic Fix (Sum → Max)

### The Problem

**Old logic (WRONG):**
```cpp
// Count enemy ornithopters - SUMS across all enemy houses
int enemyOrnithopterCount = 0;
for (int i = 0; i < NUM_HOUSES; i++) {
    const House* pHouse = currentGame->getHouse(i);
    if (pHouse && pHouse->getTeamID() != getHouse()->getTeamID()) {
        enemyOrnithopterCount += pHouse->getNumItems(Unit_Ornithopter);  // BUG: += adds up
    }
}
int requiredTurrets = enemyOrnithopterCount * 2;
```

**Example scenario:**
- Enemy House A: 5 ornithopters
- Enemy House B: 4 ornithopters
- **Old calculation:** 5 + 4 = 9 → build 18 turrets
- **Problem:** Overkill! You only face one enemy at a time, not all simultaneously

### The Fix

**New logic (CORRECT):**
```cpp
// Count enemy ornithopters - use MAXIMUM from a single enemy house, not sum
int maxEnemyOrnithopters = 0;
for (int i = 0; i < NUM_HOUSES; i++) {
    const House* pHouse = currentGame->getHouse(i);
    if (pHouse && pHouse->getTeamID() != getHouse()->getTeamID()) {
        int houseOrnis = pHouse->getNumItems(Unit_Ornithopter);
        if (houseOrnis > maxEnemyOrnithopters) {
            maxEnemyOrnithopters = houseOrnis;  // FIX: Use max, not sum
        }
    }
}
int requiredTurrets = maxEnemyOrnithopters * 2;
```

**Same scenario:**
- Enemy House A: 5 ornithopters
- Enemy House B: 4 ornithopters
- **New calculation:** max(5, 4) = 5 → build 10 turrets
- **Result:** Appropriate defense against the strongest air threat

### Why Max Instead of Sum?

**Reasoning:**
1. **Tactical reality:** You don't face all enemy houses simultaneously in a coordinated attack
2. **Resource efficiency:** Building turrets for the sum wastes resources on over-defense
3. **Proportional response:** Match your defense to the single biggest threat
4. **Scalability:** In 4v4 or 6-player FFA, sum becomes absurd

**Example (4-player FFA):**
- Player 1: 3 ornithopters
- Player 2: 4 ornithopters  
- Player 3: 2 ornithopters
- **Old (sum):** 3 + 4 + 2 = 9 → 18 turrets
- **New (max):** max(3, 4, 2) = 4 → 8 turrets

The new logic is **10 turrets more efficient** and still provides adequate defense.

## Clarification: Proactive Building

### User Concern
"It needs to build them immediately when the enemy has them not wait to be attacked."

### Current Behavior (Already Proactive!)
The counter logic **IS already proactive**:

```cpp
if (maxEnemyOrnithopters > 0 && itemCount[Structure_RocketTurret] < requiredTurrets) {
    // Start building prerequisites and turrets
}
```

**Triggers when:**
- ✅ Enemy has ANY ornithopters (`maxEnemyOrnithopters > 0`)
- ✅ We don't have enough turrets yet

**Does NOT require:**
- ❌ Being attacked
- ❌ Ornithopters being near our base
- ❌ Any damage being taken

### Why It Might APPEAR Slow

The counter needs to build **prerequisites first**:

1. **Construction Yard level 2** (two upgrades: 0→1→2)
2. **Windtrap** (power for radar)
3. **Radar** (required by rocket turrets)
4. **Rocket Turrets** (finally!)

**Timeline example:**
```
Turn 1: Enemy builds ornithopter
Turn 1: AI detects, starts CY upgrade 0→1
Turn 3: CY upgrade 1 complete, starts upgrade 1→2
Turn 5: CY upgrade 2 complete, builds Windtrap
Turn 7: Windtrap complete, builds Radar
Turn 9: Radar complete, builds Rocket Turret #1
Turn 11: Turret #1 complete, builds Rocket Turret #2
```

**This is working as intended** - the AI responds immediately but needs to build the tech tree.

## Files Modified

1. **`config/ObjectData.ini`** - Reduced ornithopter `MaxSpeed` from 18.0 to 17.0
2. **`src/players/QuantBot.cpp`** - Changed counter logic from sum to max

## Code Changes

### QuantBot.cpp Changes

**Lines 1607-1620: Counter calculation**
```cpp
// OLD (sum):
int enemyOrnithopterCount = 0;
// ...
enemyOrnithopterCount += pHouse->getNumItems(Unit_Ornithopter);

// NEW (max):
int maxEnemyOrnithopters = 0;
// ...
if (houseOrnis > maxEnemyOrnithopters) {
    maxEnemyOrnithopters = houseOrnis;
}
```

**Line 1625: Condition check**
```cpp
// OLD:
if (enemyOrnithopterCount > 0 && ...)

// NEW:
if (maxEnemyOrnithopters > 0 && ...)
```

**Lines 1651, 1656, 1663: Debug logging**
```cpp
// OLD:
logDebug("... enemy ornis: %d", enemyOrnithopterCount);

// NEW:
logDebug("... max enemy ornis: %d", maxEnemyOrnithopters);
```

## Testing Recommendations

### 1. Speed Test
- Spawn ornithopters and rocket turrets
- Verify turrets catch and kill ornithopters reliably
- Confirm closing speed is noticeable (3 tiles/sec advantage)

### 2. Counter Logic Test (Max vs Sum)

**Scenario A: Single Enemy**
- Enemy: 5 ornithopters
- Expected: AI builds 10 rocket turrets (5 × 2)

**Scenario B: Multiple Enemies (FFA)**
- Enemy A: 5 ornithopters
- Enemy B: 3 ornithopters
- Enemy C: 2 ornithopters
- **Expected:** AI builds 10 rocket turrets (max(5,3,2) × 2 = 10)
- **NOT:** 20 turrets ((5+3+2) × 2 = 20) ✅ Fixed!

**Scenario C: Team Game (2v2)**
- Enemy Team has total 6 ornithopters (3 each)
- Expected: AI builds 6 rocket turrets (max(3,3) × 2 = 6)

### 3. Proactive Building Test
- Start game, give enemy Hightech Factory
- Enemy builds 1 ornithopter
- Verify AI immediately:
  1. Starts upgrading CY (if needed)
  2. Builds Windtrap (if needed)
  3. Builds Radar (if needed)
  4. Builds rocket turrets

## Expected Impact

### Speed Nerf (18 → 17)
- ✅ **15% speed advantage** for turret rockets (was 11%)
- ✅ **Easier to catch** ornithopters
- ✅ **More time** to react to raids
- ✅ **Still fast** enough to be viable (3.8x faster than tanks)

### Counter Logic Fix (Sum → Max)
- ✅ **More efficient** resource allocation
- ✅ **Scales better** in multiplayer
- ✅ **Appropriate** defense levels
- ✅ **Still effective** against single biggest threat

## Gameplay Balance

**Ornithopters remain viable:**
- Speed 17.0 is still **3.8x faster than tanks**
- Still excellent for **raids and harassment**
- Still **outrun ground units** easily
- Just **more counterable** with proper defense

**Rocket turrets now more effective:**
- **Faster closing speed** (3 vs 2 tiles/sec)
- **Higher hit rate** due to speed advantage
- **More reliable** air defense
- **Better value** for investment

## Related Documents

- `documents/59-ornithopter-speed-nerf.md` - First speed reduction (22→18)
- `documents/61-ornithopter-counter-prerequisite-fix.md` - Counter prerequisite checks
- `documents/57-turret-targeting-frequency-fix.md` - Turret effectiveness fixes

## Status

✅ **Implemented** - Speed nerf and counter logic fix complete  
✅ **Compiled** - No errors  
⏳ **Testing** - Needs gameplay verification  

## Summary

**Speed:** 22 → 18 → 17 (progressive nerfs for balance)  
**Counter:** Sum → Max (smarter defense calculation)  
**Result:** More balanced air combat with appropriate counter-measures

