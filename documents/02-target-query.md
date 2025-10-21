# Target Query Planning

## Objectives
- Replace tile-by-tile scans in `ObjectBase::findTarget*` with spatial-grid-driven queries.
- Preserve gameplay semantics (guard range limits, hunt mode full-map search).
- Tolerate inconsistent grid state without crashing; fall back gracefully.

## Inputs & Outputs
- **Inputs:** `ObjectBase& seeker`, guard/hunt parameters, optional filters (structures only, units only).
- **Outputs:** `ObjectBase* target` (null when none found). No ownership transfer.

## Core Flow
1. Acquire the seeker’s location and guard radius (view/weapon/area guard range).
2. Compute the bounding box of grid cells covering that radius.
3. Iterate rings of cells:
   - Build a ring iterator that yields each cell perimeter once.
   - For each cell: fetch entries via `SpatialGrid::forEachInCell` into a local buffer.
   - Validate candidates, apply visibility/fog checks, distance checks, and type filters.
   - Track the closest acceptable target within the current ring.
   - If any candidate survives, stop expanding (guard mode) or return immediately (hunt mode once the first valid target appears).
4. Hunt mode: if all rings within map bounds are empty, continue expanding until the grid boundary is reached. If still empty, optionally fall back to legacy `findClosestTarget()` to cover misconfigured grid cases.

## CRUD & Race Checklist
- **Create:** When a candidate is read from the cell cache, verify `entry.ptr != nullptr`, `entry.ptr->getGridHandle().matches(entry)`, and `!entry.ptr->isMarkedForDeletion()`. Skip otherwise.
- **Read:** Never dereference raw entries directly. Copy into a temporary vector so cell mutations during iteration (e.g., unit dying mid-loop) do not invalidate iterators.
- **Update:** Do not modify grid entries from the query path; if a stale entry is detected, queue a maintenance task (`SpatialGrid::markStale(cellCoord)`) to clean after iteration.
- **Delete:** Double-check that `target` remains valid before returning (call `target->isAlive()` / `target->isActive()`).

## Defensive Measures
- Guard against empty buffers, null pointers, and mismatched generations with early `continue`.
- Clamp cell coordinates when forming rings; if a ring lies entirely outside the map, stop expanding.
- Catch `std::bad_alloc` when building buffers; on failure, log and fall back to the legacy search to keep gameplay responsive.
- Include `SDL_assert` hooks around invariants (e.g., ring builder never yields duplicates).

## API Sketch
```cpp
struct TargetQuery {
    const ObjectBase& seeker;
    int maxRange;              // guard range or INT_MAX for hunt
    TargetFilter filter;       // predicate encapsulating canAttack, FO visibility, etc.
    bool huntMode;
};

const ObjectBase* SpatialTargeting::findTarget(const TargetQuery& query);
```

`TargetFilter` exposes `bool accepts(const ObjectBase& candidate, FixPoint sqDistance)`.

## Integration Points
- `ObjectBase::findTarget` feeds the new query helper. Guard/area-guard/ambush set `maxRange` accordingly. `HUNT` sets `maxRange = INT_MAX` and `huntMode = true`.
- Existing helper `findClosestTarget()` becomes a wrapper around the same query with `huntMode = true` and an unrestricted range.
- Special cases (e.g., sandworms) contribute custom filters but reuse the same ring iteration.

## Logging & Telemetry
- Log once per match (debug level) when the query falls back to legacy behaviour due to grid failure.
- Track metrics (optional): number of rings expanded, candidates evaluated, fallback count. Useful for tuning cell size.

## Testing
- Unit tests for the ring iterator (correct perimeter enumeration, bounds clamping).
- Simulated scenarios:
    * Guard unit spotting enemy in immediate cell.
    * Hunt unit locating enemy across map with sparse population.
    * Candidate removed mid-query (ensure no crash, target skipped).
    * Grid intentionally disabled (`handle.isValid() == false`) → legacy path engaged.
- Run soak tests with large armies; monitor logs for stale-entry warnings.

