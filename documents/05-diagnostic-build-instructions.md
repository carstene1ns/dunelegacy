# Diagnostic Build Instructions

## Overview
I've added comprehensive diagnostic logging to help track down the off-screen firing bug. The changes add `[DIAG]` log messages that will help identify exactly when and why launchers/sonic tanks are firing at invalid targets.

## Changes Made

### 1. Enhanced Target Validation (`src/units/UnitBase.cpp`)

#### Added: Actual location validation (lines 485-492)
When a unit has a target, we now check BOTH:
- The "closest point" to the unit (what we aim at)
- The target's actual location

This catches cases where `getClosestPoint()` returns a valid tile but the target itself is off-map.

#### Added: Excessive distance check (lines 514-520)
If a target is more than 5× weapon range away, we now force release it. This prevents units from chasing targets across the entire map in HUNT mode.

**Logs:** `[DIAG] Unit X releasing target Y: excessive distance Z (weapon range W)`

#### Added: Attack firing diagnostics (lines 559-563)
Logs when launchers/sonic tanks actually fire at targets, showing:
- Unit ID
- Target ID  
- Distance to target
- Weapon range

**Logs:** `[DIAG] Unit X (launcher/sonic) firing at target Y: distance Z, weaponRange W tiles`

#### Added: AttackPos validation (lines 570-587)
Before attacking a position (not an object), we now:
1. Check if the tile exists
2. Check if distance is > 2× weapon range (sanity check)

**Logs:** 
- `[DIAG] Unit X clearing invalid attackPos (A,B) - not on map`
- `[DIAG] Unit X has attackPos (A,B) too far away: distance Z, weaponRange W`

#### Added: AttackPos firing diagnostics (lines 599-603)
Similar to target firing, but for position attacks.

**Logs:** `[DIAG] Unit X (launcher/sonic) firing at attackPos (A,B): distance Z, weaponRange W`

### 2. Target Acquisition Logging (`src/ObjectBase.cpp`)

#### Added: findTargetViaGrid diagnostics (lines 610-616)
Logs whenever the spatial grid finds a target for launchers/sonic tanks, showing:
- Seeker unit ID
- Target acquired
- Distance
- Hunt mode status
- Check range vs weapon range

**Logs:** `[DIAG] findTargetViaGrid: unit X acquired target Y at distance Z (huntMode=H, checkRange=C, weaponRange=W)`

## How to Use

### Step 1: Build with Diagnostics

On macOS (your platform):
```bash
cd IDE/xCode
xcodebuild -project "Dune Legacy.xcodeproj" \
           -scheme "Dune Legacy" \
           -configuration GameDebug \
           clean build
```

The GameDebug configuration will include all SDL_Log output.

### Step 2: Run the Game
```bash
cd IDE/xCode/build/GameDebug
./Dune\ Legacy.app/Contents/MacOS/Dune\ Legacy 2>&1 | tee ~/dune-debug.log
```

This pipes all output to both the terminal and a log file.

### Step 3: Reproduce the Issue
1. Start a custom game with multiple AI players
2. Let it run until you observe launchers/sonic tanks firing off-screen
3. Note the approximate game time when it happens
4. Stop the game (or let it continue to collect more data)

### Step 4: Analyze the Logs
```bash
# Filter for just diagnostic messages
grep "\[DIAG\]" ~/dune-debug.log > ~/dune-diag-only.log

# Look for excessive distances
grep "excessive distance" ~/dune-diag-only.log

# Look for invalid attackPos
grep "attackPos" ~/dune-diag-only.log

# Look at what targets were acquired
grep "findTargetViaGrid" ~/dune-diag-only.log

# Look at actual firing events
grep "firing at" ~/dune-diag-only.log
```

## What to Look For

### Scenario A: Targets Moving Off Map
```
[DIAG] findTargetViaGrid: unit 1234 acquired target 5678 at distance 3.5 (huntMode=1, checkRange=0, weaponRange=5)
[DIAG] Unit 1234 (launcher/sonic) firing at target 5678: distance 12.5, weaponRange 5 tiles
[DIAG] Unit 1234 releasing target 5678: targetLocation (128,64) doesn't exist
```
**Interpretation:** Target was acquired legitimately but then moved off-map. The tile validation caught it.

### Scenario B: Excessive Initial Distance
```
[DIAG] findTargetViaGrid: unit 1234 acquired target 5678 at distance 45.2 (huntMode=1, checkRange=0, weaponRange=5)
[DIAG] Unit 1234 (launcher/sonic) firing at target 5678: distance 45.2, weaponRange 5 tiles
```
**Interpretation:** Spatial grid is returning targets that are way too far away. The 5× weapon range check should catch this now.

### Scenario C: Invalid AttackPos
```
[DIAG] Unit 1234 has attackPos (95,128) too far away: distance 67.3, weaponRange 5
[DIAG] Unit 1234 clearing invalid attackPos (95,128) - not on map
```
**Interpretation:** Something set an attackPos to an invalid location. Need to trace where attackPos is being set.

### Scenario D: Normal Operation (No Issues)
```
[DIAG] findTargetViaGrid: unit 1234 acquired target 5678 at distance 3.5 (huntMode=1, checkRange=0, weaponRange=5)
[DIAG] Unit 1234 (launcher/sonic) firing at target 5678: distance 4.2, weaponRange 5 tiles
```
**Interpretation:** Everything working correctly - target acquired within reasonable range, firing while in range.

## Expected Outcomes

### If the fixes work:
- You should see `[DIAG]` messages about releasing targets or clearing attackPos
- The off-screen firing should stop
- Units should behave normally

### If the issue persists:
- We'll see diagnostic messages that don't match any of the scenarios above
- This means there's another code path we haven't considered
- Share the diagnostic log and we can identify the next place to investigate

## Rollback (If Needed)
```bash
cd /Users/stefanvanderwel/development/dune/dunelegacy
git diff src/units/UnitBase.cpp src/ObjectBase.cpp > diagnostic-patches.diff
git checkout src/units/UnitBase.cpp src/ObjectBase.cpp
```

To reapply later:
```bash
git apply diagnostic-patches.diff
```

## Next Steps Based on Findings

### If excessive distance is the issue:
Strengthen the distance checks or add maximum range cap in HUNT mode

### If attackPos is the issue:
Add logging to doAttackPos and anywhere else attackPos is set to find the source

### If spatial grid returns bad targets:
Add bounds checking or distance validation in findTargetViaGrid before returning

### If targets move off-map:
The current fixes should handle this, but we might need to add prediction

### If none of the above:
There's another code path we haven't identified - will need the logs to find it

## Performance Note
The diagnostic logging will have minimal performance impact:
- Only logs for launchers and sonic tanks
- Only logs when specific conditions occur (not every frame)
- String formatting only happens if the condition is true

If you notice FPS drops, you can disable specific logs by commenting out the SDL_Log lines.

## Questions to Answer
When you run the diagnostic build, please note:
1. **When does it happen?** Early game, mid-game, late game?
2. **What units?** Just launchers, just sonic tanks, or both?
3. **Attack mode?** Are they in HUNT, GUARD, AREAGUARD when it happens?
4. **How often?** Constantly, occasionally, once then stops?
5. **Which diagnostic logs appear?** Share the grep results above
6. **Does it still fire off-screen?** Or do the fixes prevent it?

## Additional Debug Options

### Enable all object targeting logs:
Remove the `getItemID() ==` checks to log for ALL units (warning: very verbose)

### Add attack() method logging:
In `UnitBase::attack()` around line 197, add:
```cpp
SDL_Log("[DIAG] Unit %u attack() called: hasTarget=%d, attackPos=(%d,%d), weaponTimer=%d",
        objectID, target ? 1 : 0, attackPos.x, attackPos.y, primaryWeaponTimer);
```

### Add destination tracking:
In `engageTarget()` after line 533, add:
```cpp
SDL_Log("[DIAG] Unit %u in attack range: loc=(%d,%d), dest=(%d,%d), guard=(%d,%d)",
        objectID, location.x, location.y, 
        destination.x, destination.y, guardPoint.x, guardPoint.y);
```

This will show if destination/guardPoint are diverging.

