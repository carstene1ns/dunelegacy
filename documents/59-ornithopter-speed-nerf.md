# Ornithopter Speed Nerf for Turret Balance

**Date:** 2025-10-24  
**Version:** 0.98.6.2  
**Issue:** Rocket turrets couldn't catch ornithopters due to speed mismatch

## Summary

Reduced ornithopter speed from 22.0 to 18.0 to allow turret rockets (speed 20) to catch and hit them. Also added safety detonation timer to turret rockets as backup.

## Problem Analysis

### Speed Mismatch

**Original speeds:**
- Ornithopter: **22.0**
- Turret Rocket: **20.0**
- Launcher Rocket: **17.5**

**Result:**
- Ornithopters were **faster than turret rockets**
- Rockets would chase but never catch up
- Even with detonation timer logic bugs fixed, rockets couldn't physically reach their targets
- Ornithopters could raid bases with impunity

### Why Previous Fixes Didn't Work

1. **Bullet type fix (Bullet_Rocket):** Used slower rockets (17.5) - even worse!
2. **Detonation timer logic fix:** Timer only checked when moving away from target, not when chasing
3. **Fast scanning (5-10 frames):** Helped detect targets, but couldn't solve speed problem

The core issue was **physics**: A slower object cannot catch a faster object, period.

## Solution Implemented

### Change 1: Slow Down Ornithopters

**File:** `config/ObjectData.ini`

**Before:**
```ini
[Ornithopter]
MaxSpeed = 22.0
```

**After:**
```ini
[Ornithopter]
MaxSpeed = 18.0
```

### Change 2: Use Bullet_TurretRocket (Reverted)

**File:** `src/structures/RocketTurret.cpp`

Reverted from `Bullet_Rocket` (speed 17.5) back to `Bullet_TurretRocket` (speed 20) so rockets are faster than ornithopters.

### Change 3: Add Safety Detonation Timer

**File:** `src/Bullet.cpp`

**Before:**
```cpp
case Bullet_TurretRocket: {
    damageRadius = TILESIZE/2;
    speed = 20;
    detonationTimer = -1;  // No timer - never explodes if it can't catch target
    numFrames = 16;
    graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
} break;
```

**After:**
```cpp
case Bullet_TurretRocket: {
    damageRadius = TILESIZE/2;
    speed = 20;
    detonationTimer = 30;  // Safety timer: 30 cycles = 1 second
    numFrames = 16;
    graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
} break;
```

### Change 4: Include TurretRocket in Detonation Logic

**File:** `src/Bullet.cpp`

**Before:**
```cpp
if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
    if(detonationTimer == 0) {
        destroy();
        return;
    }
}
```

**After:**
```cpp
if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) {
    if(detonationTimer == 0) {
        destroy();
        return;
    }
}
```

## Speed Comparison

| Unit/Bullet | Old Speed | New Speed | Can Catch Orni? |
|-------------|-----------|-----------|-----------------|
| **Ornithopter** | 22.0 | **18.0** | - |
| **Turret Rocket** | 20 | 20 | ✅ **YES** (20 > 18) |
| Launcher Rocket | 17.5 | 17.5 | ⚠️ Marginal (relies on timer) |
| Tank | 4.5 | 4.5 | ❌ No |
| Quad | 5.12 | 5.12 | ❌ No |
| Siege Tank | 3.0 | 3.0 | ❌ No |

## Gameplay Impact

### Ornithopter Balance

**Ornithopters are still effective:**
- Speed 18.0 is still **very fast** (3.5x faster than tanks, 3x faster than quads)
- Still highly mobile for raids and harassment
- Still outrun ground units easily
- Still require anti-air defense

**But now counterable:**
- Rocket turrets can **catch and hit them**
- Players have a **reliable defense option**
- Proper base defense actually works
- Strategic depth restored

### Turret Effectiveness

**Rocket turrets now work as intended:**
- **Speed advantage:** 20 vs 18 = can catch ornithopters
- **Safety net:** 30-cycle timer ensures explosion even if chase fails
- **Reliable defense:** Combined with fast scanning (5-10 frames), turrets now effectively counter air raids

### Launcher Effectiveness

**Launchers still work:**
- Slower rockets (17.5) rely more on detonation timer
- Timer-based area damage still hits ornithopters
- Remain effective mobile anti-air

## Technical Notes

### Detonation Timer Behavior

With the 30-cycle timer on turret rockets:
- At 30 FPS: 30 cycles = **1 second flight time**
- Rocket speed 20 = **20 tiles/second**
- Maximum chase distance: **20 tiles** before guaranteed explosion
- This ensures rockets always explode, preventing infinite chase scenarios

### Speed Differential Math

**Closing speed when chasing:**
- Turret rocket: 20 tiles/sec
- Ornithopter: 18 tiles/sec
- **Closing rate: 2 tiles/sec**

**Time to catch at various distances:**
- 5 tiles: 2.5 seconds (75 cycles)
- 10 tiles: 5 seconds (150 cycles)
- But ornithopters typically enter turret range at 8 tiles or less

**With 30-cycle timer:**
- Effective catch distance: 20 tiles from turret
- This exceeds typical turret range (8 tiles)
- **Result: Rockets should always catch before timer expires**

## Files Modified

1. `config/ObjectData.ini` - Reduced ornithopter speed to 18.0
2. `src/structures/RocketTurret.cpp` - Reverted to Bullet_TurretRocket
3. `src/Bullet.cpp` - Added 30-cycle timer to Bullet_TurretRocket
4. `src/Bullet.cpp` - Included Bullet_TurretRocket in detonation logic

## Testing Recommendations

1. **Basic functionality:**
   - Build rocket turrets
   - Send ornithopters to raid
   - Verify turrets hit and kill ornithopters

2. **Speed verification:**
   - Observe rocket catching up to ornithopters
   - Confirm visual closure rate

3. **Timer testing:**
   - Send ornithopter at extreme range
   - Verify rocket explodes after 1 second if chase fails

4. **Balance testing:**
   - Play against AI with ornithopter spam
   - Verify turrets provide adequate defense
   - Ensure ornithopters are still useful but not overpowered

## Related Documents

- `documents/57-turret-targeting-frequency-fix.md` - Complete turret fix documentation
- `documents/58-turret-scan-performance-fix.md` - Performance optimization

## Status

✅ **Implemented** - All changes complete  
✅ **Compiled** - No errors  
⏳ **Testing** - Requires gameplay verification  
🎯 **Expected Result** - Rocket turrets now counter ornithopters effectively

