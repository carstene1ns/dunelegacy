# Diagnostic Logging Cleanup

## Summary
Cleaned up diagnostic logging to only show **problems/anomalies**, not normal operations. This prevents log flooding while still catching bugs.

## Changes Made

### Log Level Convention
- **`[ERROR]`** - Something is definitely wrong (should never happen in normal gameplay)
- **`[WARN]`** - Something unusual happened that might indicate a problem
- Removed `[DIAG]` logs that showed normal operations

### What Gets Logged Now

#### ERROR Level (Critical Issues)

1. **Firing beyond weapon range** (UnitBase.cpp:560-564, 601-605)
   ```
   [ERROR] Unit X (launcher/sonic) firing beyond weapon range at target Y: distance Z tiles, weaponRange W tiles
   ```
   - Only logs if launcher/sonic tank actually fires at a target beyond its weapon range
   - This should NEVER happen with the fix applied
   - If you see this, the targeting bug still exists

2. **Excessive distance target acquisition** (UnitBase.cpp:515-517)
   ```
   [ERROR] Unit X releasing target Y: excessive distance Z tiles (weapon range W tiles) - targeting bug!
   ```
   - Only logs if target is > 5× weapon range away
   - Indicates spatial grid returned an absurdly distant target
   - Should be rare/never with proper targeting

#### WARN Level (Unusual but Handled)

3. **Target location doesn't exist** (UnitBase.cpp:479-480, 488-489)
   ```
   [WARN] Unit X releasing target Y: targetLocation (A,B) doesn't exist
   [WARN] Unit X releasing target Y: target actual location (A,B) invalid
   ```
   - Target moved off-map or became invalid
   - Handled gracefully by releasing target
   - May occur when units are carried off-map by carryalls

4. **Invalid attackPos** (UnitBase.cpp:573-574, 583-585)
   ```
   [WARN] Unit X clearing invalid attackPos (A,B) - not on map
   [WARN] Unit X has attackPos (A,B) too far away: distance Z tiles, weaponRange W tiles - clearing
   ```
   - Attack position was set to invalid location
   - Handled by clearing attackPos
   - May indicate issue in code that sets attackPos

5. **Distant target acquisition (non-HUNT)** (ObjectBase.cpp:615-617)
   ```
   [WARN] findTargetViaGrid: unit X acquired distant target Y at Z tiles (checkRange=C, weaponRange=W)
   ```
   - Only logs for launchers/sonic tanks in non-HUNT modes
   - Only if target is > 2× weapon range away
   - May occur in AREAGUARD mode with large area ranges
   - Still valid but unusual enough to log

### What NO LONGER Gets Logged

❌ Normal target acquisition (every time a unit finds a target)  
❌ Normal firing (every time a unit shoots)  
❌ Normal distance calculations  
❌ Routine validation checks that pass  

## Expected Log Volume

### Before Fix (With Bug Active)
You would see frequent:
- `[ERROR]` firing beyond weapon range (constantly)
- `[ERROR]` excessive distance releases (frequently)

### After Fix (Working Correctly)
You should see:
- **No `[ERROR]` logs** (or extremely rare)
- Occasional `[WARN]` logs for edge cases (targets leaving map, etc.)
- Mostly silent logs = healthy gameplay

### If Bug Persists
You'll see:
- Repeated `[ERROR]` logs from same unit IDs
- Patterns of excessive distances
- This means the AREAGUARD fix didn't fully solve it

## How to Check Logs

### During Gameplay
Logs print to console in real-time. Watch for `[ERROR]` or `[WARN]` tags.

### After Gameplay
```bash
# Check for any errors
grep "\[ERROR\]" ~/dune-debug.log

# Check for warnings
grep "\[WARN\]" ~/dune-debug.log

# Filter for launcher/sonic tank issues
grep "\[ERROR\].*launcher\|sonic" ~/dune-debug.log

# Count occurrences
grep -c "\[ERROR\]" ~/dune-debug.log
```

### Expected Results After Fix
```bash
$ grep "\[ERROR\]" ~/dune-debug.log
# Should return no results or very few

$ grep "\[WARN\]" ~/dune-debug.log  
# May have some entries for edge cases (targets leaving map, etc.)
```

## Log Interpretation Guide

### Scenario A: Clean Logs (Fix Working)
```
# No ERROR logs
# Few or no WARN logs
# Gameplay looks normal
```
**Conclusion:** Bug is fixed! ✅

### Scenario B: ERROR Logs Present (Bug Still Exists)
```
[ERROR] Unit 1234 (launcher) firing beyond weapon range at target 5678: distance 45.3 tiles, weaponRange 5 tiles
[ERROR] Unit 1234 (launcher) firing beyond weapon range at target 5678: distance 45.3 tiles, weaponRange 5 tiles
[ERROR] Unit 1234 (launcher) firing beyond weapon range at target 5678: distance 45.3 tiles, weaponRange 5 tiles
```
**Conclusion:** AREAGUARD fix didn't solve it, need to investigate further ❌

### Scenario C: Excessive Distance Logs (Targeting Issue)
```
[ERROR] Unit 1234 releasing target 5678: excessive distance 78.5 tiles (weapon range 5 tiles) - targeting bug!
```
**Conclusion:** Spatial grid is returning targets that are too far away ❌

### Scenario D: WARN Logs Only (Edge Cases)
```
[WARN] Unit 1234 releasing target 5678: target actual location (128,64) invalid
[WARN] Unit 3456 has attackPos (95,120) too far away: distance 15.3 tiles, weaponRange 5 tiles - clearing
```
**Conclusion:** Edge cases being handled correctly, may be fine ⚠️

## Performance Impact

### Before Cleanup
- Logged every target acquisition for launchers/sonic tanks
- Logged every firing event for launchers/sonic tanks
- High volume in battles with many ranged units
- Potential FPS impact in heavy combat

### After Cleanup
- Logs only exceptions/failures
- Minimal log volume in normal gameplay
- No performance impact
- Clean, readable logs

## Debugging Workflow

If you see ERROR logs:

1. **Note the pattern**
   - Same unit repeatedly? Check that unit's behavior
   - Many different units? Check global targeting logic
   - Only certain unit types? Check type-specific code

2. **Check the distance values**
   - Distance > 50 tiles? Likely AREAGUARD bug persists
   - Distance = 2-3× weapon range? May be AREAGUARD/HUNT mode confusion
   - Distance varies wildly? Possible guard point issue

3. **Correlate with gameplay**
   - Does it happen during kiting? Check kiting logic
   - Does it happen during attack? Check attack squad logic
   - Does it happen randomly? Check spatial grid

4. **Share the logs**
   - Copy ERROR logs to a file
   - Include context (what was happening in game)
   - We can analyze the pattern to identify remaining issues

## Temporary Verbose Mode

If you need more detail for debugging, you can temporarily re-enable verbose logging by changing log levels:
- `[ERROR]` → `[DEBUG]` to see even when working
- Remove distance/type checks to log all units

But for normal gameplay/testing, the current ERROR/WARN setup is ideal.

## Success Criteria

✅ No `[ERROR]` logs during normal gameplay  
✅ Rare `[WARN]` logs for edge cases only  
✅ Launchers/sonic tanks don't fire off-screen  
✅ Clean, readable log output  
✅ Minimal performance impact  

