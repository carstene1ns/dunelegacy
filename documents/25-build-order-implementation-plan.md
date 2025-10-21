# Build Order Implementation Plan

## Status
✅ Added `findPlaceLocationSimple()` method (lines 862-947)
✅ Compiled successfully

## Next Step: Replace Construction Yard Build Order

### Changes Needed in Construction Yard Case:

#### Campaign Mode (Already Good):
- ✅ Windtrap cap at initialItemCount 
- ✅ Silo at 90% capacity
- ✅ Rocket turret limits

#### Custom Mode (Needs Complete Replacement):
**Priority Order (from old version):**
1. WindTrap if count ==  0
2. Refinery if count == 0 or < harvesters/3
3. Refinery if count < 4 and money < 4000
4. StarPort if count == 0
5. Radar if count == 0 and money > 500
6. **Enemy ornithopter counter logic** (NEW)
   - If enemyOrnis > our rocket turrets:
     - Upgrade CY to level 2 if needed
     - Build rocket turret
7. LightFactory if count == 0 and money > 500
8. **HeavyFactory if count == 0 and money > 1000** (EARLIER!)
9. RepairYard if count == 0 and money > 1000
10. Refinery if money < 4000 and harvesters < limit
11. CY Upgrade to level 2 if money > 1000
12. Rocket turrets (progressive, up to 2)
13. HighTechFactory if count == 0
14. More refineries if needed
15. IX if count == 0
16. **Additional Heavy Factories** (when busy + good economy)
17. More refineries (ratio 3.5)
18. **Repair Yards** (1 per 6000 military value)
19. **Additional High-Tech Factories** (when all busy)
20. Silos (capacity management)
21. Palace (requires Heavy + Light factories)

### Slab Fallback Logic:
- If can't place structure, try to build Slab1
- Use findPlaceLocationSimple for slabs
- In isWaitingToPlace: use findPlaceLocationSimple for slabs, findPlaceLocation for others

## Implementation Approach:
Due to size, will need to replace the entire Construction Yard case (currently lines ~1498-1608) with the improved version (lines 1657-1929 from old version).

