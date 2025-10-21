# Turret Placement Logic Revision

## Problem History

### Original Issue
The original `findPlaceLocation()` heuristic was designed for production buildings (factories, refineries), which favored:
- Adjacency to existing structures (compact base)
- Proximity to base center

This was suboptimal for turrets, which need to defend against threats.

### First Fix (Perimeter Placement) - **REJECTED**
The initial fix in document 33 placed turrets at the **base perimeter**, with scoring that:
- **Rewarded** distance from base center (`score += distanceFromBase * 3`)
- Favored map edges (`score += (15 - distanceToEdge) * 4`)
- Favored being toward enemy direction
- Strong preference for sand over rock

**Result**: Turrets scattered at map edges, isolated from base, easy targets for enemy to pick off one by one.

### Current Fix (Integrated Defense)
Turrets should be **integrated into the base** (next to buildings, near center) but on the **side facing the enemy**.

## New Placement Logic

### Scoring Criteria (in priority order)

**1. Adjacency to Own Buildings** (Highest Priority)
```cpp
score += adjacentOwnBuildings * 15; // Strong bonus for being next to own buildings
```
- Checks all tiles around the turret location (8-directional + edges)
- +15 score for each adjacent friendly building
- Creates compact, integrated defense

**2. Close to Base Center**
```cpp
score -= distanceFromBase * 2; // Penalty for being far from center
```
- **Changed from** `score += distanceFromBase * 3` (reward distance)
- **Changed to** `score -= distanceFromBase * 2` (penalize distance)
- Keeps turrets integrated in base, not scattered at perimeter

**3. Enemy-Facing Side** (Directional Bonus)
```cpp
// Dot product: positive if candidate is on the enemy side of base
int dotProduct = baseToEnemyX * baseToCandidateX + baseToEnemyY * baseToCandidateY;
if (dotProduct > 0) {
    score += dotProduct / 10; // Bonus for being on enemy-facing side
}
```
- Uses **squad rally location** as primary enemy direction indicator
- Falls back to closest enemy structure if no rally point
- Calculates vector from base to enemy, and base to candidate position
- Dot product determines if candidate is on the enemy-facing side
- Positive dot product = same general direction = bonus

**4. Sand Preference** (Tiebreaker)
```cpp
score += sandTiles * 2; // Minor bonus for sand
```
- **Reduced from** `score += sandTiles * 10` (strong preference)
- **Reduced to** `score += sandTiles * 2` (tiebreaker only)
- Just ensures buildable terrain, not a primary factor

### Removed Criteria

**❌ Map Edge Bonus** (was `score += (15 - distanceToEdge) * 4`)
- Caused perimeter placement
- Made turrets isolated targets

**❌ Distance to Enemy** (was `score -= distanceToEnemy * 2`)
- Caused turrets to be placed far forward, away from base
- Conflicts with integrated defense strategy

## Example Placement

### Before (Perimeter):
```
                Enemy →
    
    [Base]──[Refinery]──[Factory]
      │
    [Silo]
    
    
    [Turret]              [Turret]
    (edge)                (edge)
```
❌ Turrets isolated at map edges, far from base

### After (Integrated):
```
                Enemy →
    
    [Base]──[Turret]──[Refinery]──[Factory]
      │        │          │
    [Silo]──[Turret]──[Turret]
```
✅ Turrets next to buildings, on enemy-facing side, layered defense

## Code Location

**File**: `src/players/QuantBot.cpp`  
**Function**: `Coord QuantBot::findTurretPlaceLocation(Uint32 itemID)` (lines 862-964)

## Impact

**Strategic Benefits**:
- Turrets integrated into base layout (not scattered)
- Strong adjacency bonus creates natural defensive perimeter around buildings
- Enemy-facing side bonus creates directional defense without isolation
- Compact base makes it harder for enemy to pick off individual turrets
- Turrets protect each other and nearby buildings

**Gameplay Benefits**:
- AI bases look more natural (turrets next to buildings)
- Turrets benefit from being near other structures (shared defense)
- More challenging for player to breach AI defenses
- Turrets placed strategically on threat side, not randomly at edges

## Why Squad Rally Location?

The user specifically requested to use **squad rally location** as the enemy direction approximation:

> "favour the side of the base closest to the enemy (squad center is a good approximation)"

**Why this works**:
- Squad rally location represents where the AI's military units are gathering
- This location is typically calculated based on enemy threat positions
- More dynamic than just "closest enemy structure"
- Better represents the actual threat vector
- Falls back to closest enemy structure if no rally point exists

## Testing

**Scenario**:
1. Start custom game vs AI (medium/hard difficulty)
2. Let AI build up base
3. Build military units to threaten AI
4. AI should respond by building rocket turrets
5. Observe turret placement:
   - ✅ Turrets should be next to other AI buildings
   - ✅ Turrets should be near base center (not scattered at edges)
   - ✅ Turrets should favor the side of the base facing your units
   - ❌ Turrets should NOT be at map edges far from base

## Related Fixes

**Document 32**: Ornithopter counter priority (AI builds turrets to match enemy ornithopter count)  
**Document 36**: Carryall/RepairYard crash fix (prevents segfault when repair yard destroyed)

This turret placement fix ensures that the turrets the AI builds (from document 32) are placed strategically and effectively.

## Summary

**Changed**: Turret placement from perimeter/edge-focused to integrated/adjacency-focused  
**Result**: Turrets are now placed next to other buildings, near base center, but on the side facing the enemy  
**Benefits**: More natural base layouts, stronger integrated defense, harder for enemies to pick off isolated turrets

