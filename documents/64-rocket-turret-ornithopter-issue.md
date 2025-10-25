# Rocket Turret vs Ornithopter Issue Analysis
**Date:** 2025-10-25  
**Problem:** Rocket turret rockets pass through ornithopters without damaging them

---

## The Problem

**User report:**
1. Rocket turrets fire at ornithopters
2. Rockets pass right through them (no hit detection)
3. Even when rockets detonate (2-second timer), they don't kill ornithopters

---

## Code Analysis

### 1. Rocket Creation (TurretBase.cpp line 199)

```cpp
bulletList.push_back( new Bullet( objectID, &centerPoint, &targetCenterPoint,bulletType,
                                       currentGame->objectData.data[itemID][originalHouseID].weapondamage,
                                       pObject->isAFlyingUnit(),  // ← airAttack = true for ornithopters
                                       pObject) );
```

**Status:** ✅ Correct - `airAttack` is set to `true` when targeting flying units

---

### 2. Rocket Tracking (Bullet.cpp lines 316-321)

```cpp
if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) {
    ObjectBase* pTarget = target.getObjPointer();
    if(pTarget && pTarget->isAFlyingUnit()) {
        destination = pTarget->getCenterPoint();  // ← Updates every frame
    }
```

**Status:** ✅ Correct - Rockets track moving ornithopters

---

### 3. Rocket Detonation Logic (Bullet.cpp lines 407-420)

```cpp
} else if(oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < 4)  {
    // Reached or overshot destination
    if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) {
        if(detonationTimer == 0) {
            destroy();  // ← Explodes when timer hits 0
            return;
        }
    } else {
        // Other bullets explode immediately
        destroy();
        return;
    }
}
```

**Key findings:**
- Rockets only explode when `detonationTimer == 0` **AND** they reach/overshoot destination
- If they never reach destination, they won't explode (even with timer at 0)
- Detonation timer: 125 cycles (2 seconds at 16ms/cycle)

---

### 4. Damage Application (Map.cpp lines 147-173)

```cpp
if(air) {
    // Air damage path
    if((bulletID == Bullet_DRocket) || (bulletID == Bullet_Rocket) || 
       (bulletID == Bullet_TurretRocket) || (bulletID == Bullet_SmallRocket)) {
        for(auto objectID : affectedAirUnits) {
            auto pAirUnit = dynamic_cast<AirUnit*>(...);
            if(distance > damageRadius) continue;
            
            const auto scaledDamage = lround(damage) >> (distance/4 + 1);
            pAirUnit->handleDamage(scaledDamage, damagerID, damagerOwner);  // ← Should work!
        }
    }
}
```

**Status:** ✅ Correct - Air attacks damage air units with `Bullet_TurretRocket`

---

## The Root Cause

### Problem 1: Rockets Never "Reach" Ornithopters

**Why rockets pass through:**

From Bullet.cpp line 407:
```cpp
} else if(oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < 4)  {
```

**Condition for detonation:**
- `oldDistanceToDestination < newDistanceToDestination` (overshot)
- OR `newDistanceToDestination < 4` (within 4 pixels)

**The issue:**
- Ornithopter is moving at speed 18.0
- Turret rocket is moving at speed 20
- Rocket has turn speed of 4.5 degrees/frame (line 333)
- Ornithopter is maneuvering

**Result:** Rocket chases ornithopter in a circle, never getting within 4 pixels!

---

### Problem 2: Timer-Based Detonation Doesn't Trigger

From Bullet.cpp lines 409-413:
```cpp
if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) {
    if(detonationTimer == 0) {
        destroy();
        return;
    }
}
```

**This code only runs INSIDE the "reached destination" block!**

If the rocket never reaches the ornithopter, this code never executes, even when `detonationTimer == 0`!

---

## Comparison with 0.96.4

### In v0.96.4 (from git show):

```cpp
case Bullet_TurretRocket: {
    damageRadius = TILESIZE/2;
    speed = 20;
    detonationTimer = -1;  // ← INFINITE! Never expires
    numFrames = 16;
    graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
} break;
```

**Key difference:** In 0.96.4, `detonationTimer = -1` meant rockets chased forever!

**But wait...** This means 0.96.4 had the SAME problem - rockets would chase forever and never explode unless they got within 4 pixels!

---

## The Missing Logic

Looking more carefully at the code flow in Bullet::update():

```cpp
void Bullet::update() {
    // ... rocket tracking logic ...
    
    FixPoint oldDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);
    
    realX += xSpeed;
    realY += ySpeed;
    location.x = floor(realX/TILESIZE);
    location.y = floor(realY/TILESIZE);
    
    // Off map check
    if((location.x < -5) || ...) {
        bulletList.remove(this);
        delete this;
        return;
    } else {
        FixPoint newDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);
        
        if(detonationTimer > 0) {
            detonationTimer--;  // ← Timer decrements every frame
        }
        
        // ... other bullet types ...
        
        } else if(oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < 4)  {
            // Only executes when "reached destination"
            if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) {
                if(detonationTimer == 0) {
                    destroy();  // ← ONLY CHECKED HERE!
                    return;
                }
            }
        }
    }
}
```

**THE BUG:** Timer decrements (line 372) but timer check (line 410) only happens when rocket reaches destination!

---

## The Fix

### Option 1: Check Timer Outside "Reached Destination" Block

**Add explicit timer check for all rockets:**

```cpp
void Bullet::update() {
    // ... existing code ...
    
    if(detonationTimer > 0) {
        detonationTimer--;
    }
    
    // NEW: Check if timer expired (for rockets that may never reach target)
    if((bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) 
       && detonationTimer == 0) {
        destroy();
        return;
    }
    
    // ... rest of update logic ...
}
```

**Location:** After line 373 in `src/Bullet.cpp`

---

### Option 2: Increase Detonation Radius

**Change from 4 pixels to larger radius:**

```cpp
} else if(oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < TILESIZE)  {
    // Detonate within 1 tile (32 pixels) instead of 4 pixels
```

**Location:** Line 407 in `src/Bullet.cpp`

---

### Option 3: Collision Detection

**Add explicit collision check with air units:**

```cpp
// In Bullet::update(), after moving the bullet:
if(bulletID == Bullet_TurretRocket && airAttack) {
    // Check if rocket is close to any air unit
    if(currentGameMap->tileExists(location)) {
        auto* pTile = currentGameMap->getTile(location);
        for(auto unitID : pTile->getAirUnitList()) {
            auto* pAirUnit = dynamic_cast<AirUnit*>(currentGame->getObjectManager().getObject(unitID));
            if(pAirUnit) {
                Coord bulletPos = Coord(lround(realX), lround(realY));
                FixPoint distance = distanceFrom(bulletPos, pAirUnit->getCenterPoint());
                if(distance < TILESIZE/2) {  // Within half a tile
                    destroy();
                    return;
                }
            }
        }
    }
}
```

---

## Recommended Solution

**Implement Option 1 + Option 3:**

1. **Option 1** ensures rockets always explode after 2 seconds (prevents endless circling)
2. **Option 3** adds collision detection so rockets explode when they get close to ornithopters

This combination provides:
- ✅ Rockets explode when close to target (realistic)
- ✅ Rockets explode after timer expires (safety)
- ✅ No more rockets passing through ornithopters

---

## Implementation Priority

**CRITICAL:** This is a gameplay-breaking bug that makes rocket turrets useless against ornithopters.

**Impact:**
- Rocket turrets can't defend against ornithopter raids
- Players lose bases to ornithopters that should be defended
- AI (QuantBot) builds rocket turrets thinking they'll help, but they don't

---

## Testing Plan

1. Build rocket turret
2. Spawn enemy ornithopter
3. Watch rocket turret fire
4. Verify:
   - ✅ Rocket tracks ornithopter
   - ✅ Rocket explodes when close (collision detection)
   - ✅ Rocket explodes after 2 seconds if it misses (timer)
   - ✅ Ornithopter takes damage and is destroyed

---

## Files to Modify

1. `src/Bullet.cpp` - Add timer check and collision detection
2. Test with various scenarios (stationary, moving, circling)

---

## Bottom Line

**The bug:** Rockets only check detonation timer when they "reach destination", but they never reach fast-moving ornithopters, so they circle forever (or until 2-second timer expires WITHOUT detonating).

**The fix:** Check detonation timer every frame AND add collision detection for air units.

**Why it was missed:** The timer logic was nested inside the "reached destination" condition, making it ineffective for targets that are never "reached".

