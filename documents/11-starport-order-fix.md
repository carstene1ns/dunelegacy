# StarPort Order Button Investigation

## Problem
AI can't click the StarPort order button - units are queued but delivery never happens. Appears as if the Frigate didn't come or isn't unloading.

## Key Discovery: Frigates Don't Use Pathfinding!

**Good news:** The pathfinding changes (2048 nodes, 6ms budget) are NOT the cause of this issue.

### Why?
Air units (including Frigates) don't use A* pathfinding at all. They fly in a straight line to their destination:

`src/units/AirUnit.cpp:144-147`
```cpp
void AirUnit::navigate() {
    moving = true;
    justStoppedMoving = false;
}
```

Compare this to ground units which call `createPath()` and use A* pathfinding. Frigates simply:
1. Spawn at map edge (closest edge to StarPort)
2. Fly directly toward destination in a straight line
3. Can pass through anything (`canPass()` always returns `true`)

### How Far Do Frigates Travel?

`src/structures/StarPort.cpp:242`
```cpp
pos = currentGameMap->findClosestEdgePoint(getLocation() + Coord(1,1), Coord(1,1));
```

Frigates spawn at the **closest map edge** to the StarPort. On a 64x64 map, this could be:
- **Minimum:** ~5-10 tiles (StarPort near edge)
- **Maximum:** ~32 tiles (StarPort in center)

They fly at a variable speed (slower near destination, faster when far away).

## Diagnostic Logging Added

### 1. StarPort Order Tracking
`src/players/QuantBot.cpp:1748-1761`

Logs when AI has an order in progress:
```cpp
if (!pStarPort->okToOrder()) {
    int secondsRemaining = (arrivalTimer * GAMESPEED_DEFAULT) / 1000;
    logDebug("StarPort house %d: order in progress, arrivalTimer=%d (≈%d seconds)", 
        getHouse()->getHouseID(), arrivalTimer, secondsRemaining);
    
    if (arrivalTimer > MILLI2CYCLES(120000)) {
        SDL_Log("[WARN] StarPort house %d has abnormally high arrivalTimer: %d (>2 minutes) - possible bug!",
            getHouse()->getHouseID(), arrivalTimer);
    }
}
```

### 2. Frigate Spawning
`src/structures/StarPort.cpp:244-254`

Logs when Frigate spawns and its flight distance:
```cpp
SDL_Log("[StarPort] House %d: Spawning Frigate at map edge (%d,%d) for StarPort at (%d,%d)", 
    owner->getHouseID(), pos.x, pos.y, getX(), getY());

SDL_Log("[StarPort] House %d: Frigate destination set to (%d,%d), distance: %.1f tiles", 
    owner->getHouseID(), closestPoint.x, closestPoint.y, 
    blockDistance(pos, closestPoint).toFloat());
```

### 3. Frigate Arrival
`src/units/Frigate.cpp:94-95`

Logs when Frigate reaches StarPort:
```cpp
SDL_Log("[Frigate] Reached StarPort at (%d,%d), starting cargo deployment", 
    destination.x, destination.y);
```

### 4. Frigate Destruction
`src/units/Frigate.cpp:69-70`

Logs if Frigate is destroyed before completing delivery:
```cpp
SDL_Log("[Frigate] DESTROYED before reaching StarPort at (%d,%d)! Clearing order.", 
    pStarPort->getX(), pStarPort->getY());
```

## What the Logs Will Show

When you play the game and encounter the issue, check the logs for one of these scenarios:

### Scenario 1: Normal Delivery
```
StarPort house 2: placing order with queue
[... 60 seconds of periodic "order in progress" messages ...]
[StarPort] House 2: Spawning Frigate at map edge (0,15) for StarPort at (20,15)
[StarPort] House 2: Frigate destination set to (20,15), distance: 20.0 tiles
[Frigate] Reached StarPort at (20,15), starting cargo deployment
```

### Scenario 2: Frigate Destroyed
```
StarPort house 2: placing order with queue
[... 60 seconds ...]
[StarPort] House 2: Spawning Frigate at map edge (0,15) for StarPort at (20,15)
[StarPort] House 2: Frigate destination set to (20,15), distance: 20.0 tiles
[Frigate] DESTROYED before reaching StarPort at (20,15)! Clearing order.
```
**Fix:** AI needs better air defenses or should wait for safer conditions before ordering.

### Scenario 3: Frigate Never Spawns
```
StarPort house 2: placing order with queue
StarPort house 2: order in progress, arrivalTimer=3750 (≈60 seconds)
StarPort house 2: order in progress, arrivalTimer=3700 (≈59 seconds)
... [timer keeps counting down to 0 but no Frigate spawn log]
```
**Fix:** Bug in `owner->createUnit(Unit_Frigate)` - investigate unit creation.

### Scenario 4: Stuck Timer
```
StarPort house 2: placing order with queue
StarPort house 2: order in progress, arrivalTimer=3750 (≈60 seconds)
StarPort house 2: order in progress, arrivalTimer=3750 (≈60 seconds)
... [timer never counts down]
[WARN] StarPort house 2 has abnormally high arrivalTimer: 8000 (>2 minutes) - possible bug!
```
**Fix:** Timer update bug - investigate `updateStructureSpecificStuff()`.

### Scenario 5: Frigate Spawns But Never Arrives
```
StarPort house 2: placing order with queue
[... 60 seconds ...]
[StarPort] House 2: Spawning Frigate at map edge (0,15) for StarPort at (20,15)
[StarPort] House 2: Frigate destination set to (20,15), distance: 20.0 tiles
... [no arrival or destruction message, order stuck forever]
```
**Fix:** Frigate movement/navigation bug - investigate `AirUnit::update()` or `Frigate::update()`.

## Files Modified
- `src/players/QuantBot.cpp` (lines 1746-1761, 1832-1834) - AI order tracking
- `src/structures/StarPort.cpp` (lines 244-254) - Frigate spawn logging
- `src/units/Frigate.cpp` (lines 69-70, 94-95) - Frigate lifecycle logging

## Status
✅ Comprehensive diagnostic logging implemented
✅ Compiled successfully
✅ Confirmed pathfinding changes are NOT the cause
🔜 Needs in-game testing to identify which scenario occurs

## Next Steps
1. Play the game and reproduce the stuck StarPort issue
2. Check console logs to see which scenario matches
3. Based on the scenario, implement targeted fix
