# Document 73: Complete Desync Analysis

**Version:** 0.98.6.3  
**Date:** October 26, 2025  
**Status:** Analysis Complete

## User's Critical Insight

**"AIPlayer has been around for years and years"** with random `buildTimer` without causing desyncs.

This is a crucial observation that challenges the assumption that random `buildTimer` is the desync cause.

## What Changed Recently (Last 2 Weeks)

### Commit 175e8f5 (Oct 25) - The Real Culprit

**Changes that DEFINITELY cause desyncs:**

1. **Combat Stats Conditional Counters** ← **DOCUMENT 69 FIXED THIS**
   - `src/structures/TurretBase.cpp` - Added ornithopter-specific counters
   - `src/structures/RocketTurret.cpp` - Added firing tracking
   - `src/Bullet.cpp` - Added detonation tracking  
   - `src/Map.cpp` - Added hit/kill tracking

**Desync mechanism:**
```cpp
// If one client finds ornithopter target, another doesn't (timing difference)
if(getItemID() == Structure_RocketTurret && target->getItemID() == Unit_Ornithopter) {
    currentGame->combatStats.rocketTurretTargetsOrni++;  // Conditional increment!
}
```

### Other Recent Changes (Analyzed)

#### ✅ Document 57 - Turret Targeting Frequency
- Added random timing: `rand(0, 5)` for target scanning
- **FIXED in Document 69** - replaced with `objectID % N`

#### ✅ Document 68 - Rocket Turret Damage
- Changed damage calculation for air units
- **SAFE** - deterministic calculation, no conditionals

#### ✅ Document 62 - Simplified Unit Ratios
- Consolidated config from 5 ratio sets to 1
- **SAFE** - configuration system, loaded identically on all clients

#### ✅ Documents 59-67 - Ornithopter Balance
- Speed changes, insurance turrets, wave sizes
- **SAFE** - all deterministic logic

#### ✅ Documents 53-58 - Config System
- External configuration loading
- **SAFE** - all clients load same config file

## Random BuildTimer Analysis

### Why AIPlayer Works Fine

AIPlayer has had this code since forever:
```cpp
// Line 42 in AIPlayer.cpp
buildTimer = getRandomGen().rand(0,3) * 50;
```

**It works because:**
1. The random generator is **seeded identically** on all clients at game start
2. As long as **all clients call `rand()` in the same order**, they get the same sequence
3. AIPlayer's logic is simple enough that it always executes the same code paths

### Why QuantBot Might Be Different

QuantBot has **more complex conditional logic**:
- Different build paths for Campaign vs Custom mode
- Different logic based on house type
- Ornithopter counter logic (new)
- Insurance turret system (new)

**Potential desync scenario:**
```
Client A: 
  Cycle 1000: Check ornithopter counter → triggers → calls rand()
  
Client B:
  Cycle 1000: Check ornithopter counter → doesn't trigger (1 ornithopter difference)
  Cycle 1001: Calls rand() at different point

→ Random sequences NOW DIVERGED
```

But this would only happen if there's a **conditional difference** in the code execution!

## The Real Question

**If Document 69 fixed the combat stats conditionals, is the desync actually fixed?**

The random `buildTimer` might be a red herring if:
1. Combat stats were the only conditional that varied between clients
2. All other code paths execute identically

## Testing Recommendations

### Test 1: Document 69 Only
1. Revert my QuantBot buildTimer changes (Document 71)
2. Keep only Document 69 fixes (combat stats removal, turret timing)
3. Test multiplayer for 15+ minutes
4. **If no desync:** BuildTimer wasn't the issue!
5. **If still desyncs:** Something else is wrong

### Test 2: Full Fix (Current State)
1. Keep both Document 69 AND Document 71
2. Test multiplayer for 15+ minutes  
3. Should definitely not desync

### Test 3: AIPlayer Multiplayer
1. Use AIPlayer (not QuantBot) in multiplayer
2. Play for 15+ minutes
3. **If desyncs:** It's NOT the buildTimer (AIPlayer has had it for years)
4. **If no desync:** Confirms AIPlayer works fine with random timers

## What's Actually Safe

### ✅ Safe Random Usage
- **AIPlayer buildTimer** - Works fine, years of stability
- **SmartBot timers** - Also worked fine
- Random placement coordinates - If all clients attempt placement

### ❌ Dangerous Patterns
- **Conditional increments** based on specific unit types
- **Conditional random calls** (only some clients call rand())
- **Performance counter usage** in game logic decisions

## SDL_GetTicks() Usage

Found in `QuantBot.cpp` line 483:
```cpp
const Uint32 currentTime = SDL_GetTicks();
if(lastMilitaryLogTime == 0) {
    lastMilitaryLogTime = currentTime;
} else if(currentTime - lastMilitaryLogTime >= 30000) {
    // Log military stats
}
```

**Status: SAFE ✅**
- Only used for logging timing
- `lastMilitaryLogTime` is `static` (persists between calls)
- Never affects game decisions

## Conclusion

### Most Likely Scenario

**The desync was caused by Document 69 issues ONLY:**
1. Combat stats conditional counters
2. Turret random timing

**Document 71 (buildTimer fix) might be unnecessary** if AIPlayer works fine with random timers.

### Recommended Action

1. **Keep Document 69 fixes** - definitely necessary
2. **Test with Document 69 only** - see if desync is gone  
3. **Consider reverting Document 71** - if unnecessary, it changes existing behavior

### Alternative Scenario

If desync persists even with Document 69:
1. There's another conditional we haven't found
2. QuantBot's complex logic creates divergent code paths
3. Document 71 might be necessary after all

## Files Analyzed

### Recently Modified (Potential Desync Sources)
- ✅ `src/structures/TurretBase.cpp` - Fixed in Doc 69
- ✅ `src/structures/RocketTurret.cpp` - Fixed in Doc 69
- ✅ `src/Bullet.cpp` - Fixed in Doc 69
- ✅ `src/Map.cpp` - Fixed in Doc 69
- ⚠️ `src/players/QuantBot.cpp` - BuildTimer (Doc 71) - possibly unnecessary

### Not Modified (Stable Code)
- ✅ `src/players/AIPlayer.cpp` - Has random buildTimer, works fine
- ✅ `src/players/SmartBot.cpp` - Has random timers, works fine
- ✅ `src/players/CampaignAIPlayer.cpp` - Has random calls, works fine

## Final Recommendation

**Test the game with ONLY Document 69 fixes first.**

If it still desyncs, then Document 71 is needed. If it doesn't desync, Document 71 was unnecessary and should be reverted to maintain compatibility with existing AIPlayer behavior.

The fact that AIPlayer has worked for years with random timers is strong evidence that random `buildTimer` alone isn't the issue.

