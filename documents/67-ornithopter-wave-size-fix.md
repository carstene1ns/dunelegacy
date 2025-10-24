# Document 67: Ornithopter Wave Size Fix

**Version:** 0.98.6.2  
**Date:** October 24, 2025  
**Type:** Balance Fix

## Problem

After implementing all the ornithopter nerfs and rocket turret buffs in 0.98.6.2, ornithopters were STILL overpowered. Investigation revealed the root cause:

### The Real Issue: Mass Attack Waves

The AI was **massing ornithopters** before attacking:
- **Hard difficulty**: Required 2 ornithopters before attacking
- **Brutal difficulty**: Required 4 ornithopters before attacking

Combined with ornithopter production ratios:
- Atreides: 15% of military
- Ordos: 25% of military
- Others: 10% of military

**Example scenario:**
- Ordos AI with 10,000 military value
- 25% ornithopters = ~2,500 in ornithopters
- At 600 credits each = **4+ ornithopters**
- All 4+ attack at once = **overwhelming swarm**

### Defensive Failure

Even with all the buffs to rocket turrets (faster scanning, better targeting, proactive insurance turrets), **4+ ornithopters attacking simultaneously** overwhelmed defenses:
- Turrets could only shoot one at a time
- Reload time of 360 frames (12 seconds)
- 4 ornithopters = need 48 seconds to kill all (if perfect hits)
- Ornithopters do 50 damage and fire every 150 frames

Result: Base gets destroyed before turrets can eliminate the swarm.

### Original Dune 2 Behavior

User reported: "in the base game they only attacked one at a time basically"

This was the key insight! The original game had ornithopters attack in **very small groups** (1-2 at most), making them a harassment tool rather than a death swarm.

## Solution

Reduced ornithopter attack thresholds to force smaller, more frequent waves:

```ini
# Before:
Hard_OrnithopterAttackThreshold=2
Brutal_OrnithopterAttackThreshold=4

# After:
Hard_OrnithopterAttackThreshold=1
Brutal_OrnithopterAttackThreshold=3
```

### Code Changes

**File: `config/QuantBot Config.ini`**
- Line 99: `Hard_OrnithopterAttackThreshold=2` → `1`
- Line 114: `Brutal_OrnithopterAttackThreshold=4` → `3`
- Line 22: Updated comment to reflect new values

**File: `src/players/QuantBotConfig.cpp`**
- Line 71: `hard.ornithopterAttackThreshold = 4` → `1`
- Line 84: `brutal.ornithopterAttackThreshold = 4` → `3`

## Expected Results

### Hard Difficulty
- ✅ AI attacks as soon as 1 ornithopter is ready
- ✅ Frequent small raids instead of massed assault
- ✅ Single turret can handle 1 ornithopter
- ✅ More like original Dune 2 behavior

### Brutal Difficulty
- ✅ AI attacks as soon as 3 ornithopters are ready
- ✅ Smaller waves than before (was 4+)
- ✅ 4 insurance turrets can handle 3 ornithopters
- ✅ Still challenging but not overwhelming

## Synergy with Previous Fixes

This change completes the ornithopter balance puzzle by working with:

1. **Speed nerfs** (22 → 15): Turrets can catch them
2. **Turret scan frequency** (5-10 frames): Fast targeting
3. **Turret close-range fix**: Won't refuse to shoot
4. **Rocket detonation timer** (60 cycles): Bullets actually explode
5. **Insurance turrets** (4 proactive): Base defense ready
6. **Wave size limit** (NEW): Prevents overwhelming swarms ✅

Together, these changes transform ornithopters from "unstoppable death swarm" to "dangerous but counterable harassment tool" - which is what they should be.

## Testing Recommendations

1. **Hard difficulty**: Verify ornithopters attack individually
2. **Brutal difficulty**: Verify 3-ornithopter waves are manageable
3. **Check logs**: Confirm AI doesn't wait to mass ornithopters
4. **Defense test**: 4 insurance turrets should handle incoming raids

## Technical Notes

- This is a **configuration-only change** - no code logic modified
- Change affects both default values (C++) and config file (INI)
- Multiplayer: All players must have matching config values
- Can be customized further if needed without recompiling

## Conclusion

The ornithopter problem was never just about speed or turret effectiveness - it was about **wave size**. By forcing the AI to attack with 1 ornithopter (Hard) or 3 ornithopters (Brutal) instead of massing 4+, we've restored the original Dune 2 raid-style gameplay while keeping ornithopters challenging.

This should finally achieve the balance we've been working toward.

