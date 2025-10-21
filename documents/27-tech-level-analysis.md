# Tech Level Analysis for Build Order

## Tech Level Requirements (from MapSettingsWindow.cpp)

```
Level 1:  Concrete, Windtrap, Refinery
Level 2:  Radar, Barracks/Light Factory/WOR, Silo, Soldier/Trooper, Trike
Level 3:  Light Factory, Quad
Level 4:  2x2 Concrete, Wall, Heavy Factory, Tank, Harvester, MCV
Level 5:  Gun Turret, Hightech Factory, Repair Yard, Launcher, Carryall
Level 6:  Starport, Rocket Turret, Siege Tank
Level 7:  House IX, Sonic Tank/Deviator/Devastator, Ornithopter
Level 8:  Palace, Fremen, Saboteur
```

## Current Build Order Issues

### ✅ Already Protected by `isAvailableToBuild()`
The game's `BuilderBase::updateBuildList()` already checks:
```cpp
if (objData.techLevel > currentGame->techLevel) {
    // reject this item
}
```

So `pBuilder->isAvailableToBuild(itemID)` will return `false` for buildings above the current tech level.

### ⚠️ Potential Inefficiencies

Our current Custom mode build order checks for buildings in this order, but some aren't available until higher tech levels:

1. ✅ **WindTrap** - Tech 1 (OK)
2. ✅ **Refinery** - Tech 1 (OK)
3. ⚠️ **StarPort** - Tech 6 (checked early, but usually fails until tech 6)
4. ✅ **Radar** - Tech 2 (OK if money > 500)
5. ⚠️ **Rocket Turret counter** - Tech 6 + Upgrade 2 (checked early for ornithopter response)
6. ✅ **Light Factory** - Tech 2/3 (OK)
7. ⚠️ **Heavy Factory** - Tech 4 (checked at money > 1000, fails in tech 1-3)
8. ⚠️ **Repair Yard** - Tech 5 (checked at money > 1000, fails in tech 1-4)
9. ⚠️ **High-Tech Factory** - Tech 5 (checked early)
10. ⚠️ **IX** - Tech 7 (checked at money > 1000)
11. ⚠️ **Palace** - Tech 8 (checked at money > 5000)

## Problems Identified

1. **Early Campaign Missions (Tech 1-3):**
   - AI checks for Heavy Factory, Repair Yard, High-Tech, etc. every cycle
   - All fail `isAvailableToBuild()` check, wasting cycles
   - Build order might stall waiting for unavailable buildings

2. **Rocket Turret Counter Logic:**
   - Checks for enemy ornithopters (Tech 7)
   - Tries to build rocket turrets (Tech 6 + Upgrade 2)
   - In early missions, this entire branch is dead code

3. **StarPort Early Check:**
   - Checked very early (after refineries)
   - Not available until Tech 6
   - Wastes cycles in missions 1-5

## Options for Improvement

### Option 1: **Tech Level Gating (Conservative)**
Add explicit tech level checks before each building block:

```cpp
// Early infrastructure (Tech 1-2)
if (itemCount[Structure_WindTrap] == 0 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
    itemID = Structure_WindTrap;
}
else if (...refineries...) {
    itemID = Structure_Refinery;
}
else if (currentGame && currentGame->techLevel >= 2 && itemCount[Structure_Radar] == 0 && money > 500) {
    itemID = Structure_Radar;
}
else if (currentGame && currentGame->techLevel >= 3 && itemCount[Structure_LightFactory] == 0) {
    itemID = Structure_LightFactory;
}
else if (currentGame && currentGame->techLevel >= 4 && itemCount[Structure_HeavyFactory] == 0 && money > 1000) {
    itemID = Structure_HeavyFactory;
}
else if (currentGame && currentGame->techLevel >= 5 && itemCount[Structure_RepairYard] == 0 && money > 1000) {
    itemID = Structure_RepairYard;
}
else if (currentGame && currentGame->techLevel >= 6) {
    // StarPort + Rocket Turret logic
}
else if (currentGame && currentGame->techLevel >= 7 && enemyOrnithopterCount > 0) {
    // Ornithopter counter logic (now makes sense)
}
// ... etc
```

**Pros:**
- Explicit and readable
- Avoids wasted `isAvailableToBuild()` calls
- Clear progression through tech levels

**Cons:**
- More verbose
- Duplicate tech level checks
- Harder to maintain

---

### Option 2: **Tech Level Sections (Cleaner)**
Organize build order into tech level sections:

```cpp
Uint32 itemID = NONE_ID;
int techLevel = currentGame ? currentGame->techLevel : 8;

// Count enemy ornithopters for dynamic response
int enemyOrnithopterCount = 0;
if (techLevel >= 7 && currentGame) {
    for (int i = 0; i < NUM_HOUSES; i++) {
        const House* pHouse = currentGame->getHouse(i);
        if (pHouse && pHouse->getTeamID() != getHouse()->getTeamID()) {
            enemyOrnithopterCount += pHouse->getNumItems(Unit_Ornithopter);
        }
    }
}

// === TECH 1-2: Basic Economy ===
if (itemCount[Structure_WindTrap] == 0 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
    itemID = Structure_WindTrap;
}
else if ((itemCount[Structure_Refinery] == 0 || itemCount[Structure_Refinery] < itemCount[Unit_Harvester] / 3) 
    && pBuilder->isAvailableToBuild(Structure_Refinery)) {
    itemID = Structure_Refinery;
}
else if (techLevel >= 2 && itemCount[Structure_Radar] == 0 && money > 500) {
    itemID = Structure_Radar;
}
else if (techLevel >= 2 && itemCount[Structure_LightFactory] == 0 && money > 500) {
    itemID = Structure_LightFactory;
}

// === TECH 4+: Heavy Production ===
else if (techLevel >= 4 && itemCount[Structure_HeavyFactory] == 0 && money > 1000) {
    itemID = Structure_HeavyFactory;
}

// === TECH 5+: Advanced Infrastructure ===
else if (techLevel >= 5 && itemCount[Structure_RepairYard] == 0 && money > 1000) {
    itemID = Structure_RepairYard;
}
else if (techLevel >= 5 && itemCount[Structure_HighTechFactory] == 0 && money > 1000) {
    itemID = Structure_HighTechFactory;
}

// === TECH 6+: Advanced Logistics & Defense ===
else if (techLevel >= 6 && itemCount[Structure_StarPort] == 0 
    && findPlaceLocation(Structure_StarPort).isValid()) {
    itemID = Structure_StarPort;
}
else if (techLevel >= 6 && enemyOrnithopterCount > itemCount[Structure_RocketTurret]) {
    // Rocket turret counter logic
}

// === TECH 7+: Elite Units & Structures ===
else if (techLevel >= 7 && itemCount[Structure_IX] == 0 && money > 1000) {
    itemID = Structure_IX;
}

// === TECH 8+: End Game ===
else if (techLevel >= 8 && money > 5000 && itemCount[Structure_Palace] == 0
    && itemCount[Structure_HeavyFactory] > 0 && itemCount[Structure_LightFactory] > 0) {
    itemID = Structure_Palace;
}

// ... more refinery/factory expansion logic (available at all tech levels) ...
```

**Pros:**
- Clear tech progression
- Grouped logically
- Still efficient (early returns)
- Comments show progression path

**Cons:**
- Might miss optimal build orders (e.g., building StarPort before Radar if both are available)

---

### Option 3: **Hybrid Approach (Recommended)**
Keep priority order, but add tech checks only where they save significant work:

```cpp
Uint32 itemID = NONE_ID;
int techLevel = currentGame ? currentGame->techLevel : 8;

// Count enemy ornithopters ONLY if tech level supports them
int enemyOrnithopterCount = 0;
if (techLevel >= 7 && currentGame) {
    for (int i = 0; i < NUM_HOUSES; i++) {
        const House* pHouse = currentGame->getHouse(i);
        if (pHouse && pHouse->getTeamID() != getHouse()->getTeamID()) {
            enemyOrnithopterCount += pHouse->getNumItems(Unit_Ornithopter);
        }
    }
}

// Essential infrastructure (always check - available at all tech levels or very early)
if (itemCount[Structure_WindTrap] == 0 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
    itemID = Structure_WindTrap;
}
else if ((itemCount[Structure_Refinery] == 0 || itemCount[Structure_Refinery] < itemCount[Unit_Harvester] / 3) 
    && pBuilder->isAvailableToBuild(Structure_Refinery)) {
    itemID = Structure_Refinery;
}
// ... early refinery logic ...

// Tech-gated for efficiency (these are expensive checks or have side effects)
else if (techLevel >= 6 && itemCount[Structure_StarPort] == 0 
    && pBuilder->isAvailableToBuild(Structure_StarPort) 
    && findPlaceLocation(Structure_StarPort).isValid()) {
    itemID = Structure_StarPort;
}
else if (techLevel >= 2 && itemCount[Structure_Radar] == 0 
    && pBuilder->isAvailableToBuild(Structure_Radar) && money > 500) {
    itemID = Structure_Radar;
}

// Counter enemy ornithopters (only relevant at tech 7+)
else if (techLevel >= 6 && enemyOrnithopterCount > itemCount[Structure_RocketTurret]) {
    // Rocket turret counter logic
}

// Normal priority order continues
else if (pBuilder->isAvailableToBuild(Structure_LightFactory) && itemCount[Structure_LightFactory] == 0 && money > 500) {
    itemID = Structure_LightFactory;
}
// ... rest of build order relies on isAvailableToBuild() ...
```

**Pros:**
- Minimal changes to existing code
- Avoids major inefficiencies (ornithopter counting at tech 1!)
- Keeps flexible priority order
- Only gates where it matters

**Cons:**
- Not as "clean" as Option 2
- Some redundant checks remain

---

### Option 4: **Let isAvailableToBuild() Handle It (Status Quo)**
Keep current code, rely entirely on `isAvailableToBuild()`:

**Pros:**
- Already works correctly (buildings won't be built if unavailable)
- Simple, no changes needed
- Game handles all tech level logic

**Cons:**
- Wastes cycles checking unavailable buildings
- Ornithopter counter logic runs every cycle in tech 1-6 (inefficient enemy house iteration)
- Less readable (not obvious what's available when)

---

## Recommendation

**Option 3: Hybrid Approach**

Add tech level checks ONLY for:
1. **Ornithopter counting** (expensive loop, only relevant at tech 7+)
2. **StarPort** (checked very early, not available until tech 6)
3. **Palace** (checked late, not available until tech 8)

This gives 95% of the benefit with 5% of the code changes.

## Implementation Priority

**High Priority:**
- ✅ Gate ornithopter counting behind `techLevel >= 7`
- ✅ Gate StarPort check behind `techLevel >= 6`

**Medium Priority:**
- Gate Heavy Factory check behind `techLevel >= 4`
- Gate Repair Yard check behind `techLevel >= 5`

**Low Priority:**
- Gate Palace check behind `techLevel >= 8`
- Gate IX check behind `techLevel >= 7`

The `isAvailableToBuild()` checks already prevent buildings from being queued, so this is primarily an optimization and readability improvement.

