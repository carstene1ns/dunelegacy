# Spatial Grid Lifecycle Planning

## Goals
- Maintain a consistent view of every live `ObjectBase` inside the spatial grid.
- Avoid CRUD races that previously caused stale pointers and segmentation faults.
- Supply clear touch points for future systems (targeting, pathing) to query safely.

## Core Invariants
- **Handle ownership.** Each trackable `ObjectBase` owns exactly one `GridHandle`. If the handle is invalid, the object is not in the grid.
- **Cell membership.** A grid cell stores only objects whose handles still reference that cell. No duplicates, no dangling pointers.
- **Update contract.** Movement, teleportation, pickup, and destruction code paths all flow through the same grid API.
- **Fail-safe.** If the grid rejects an operation (nulls, out-of-bounds, allocation failure), the object falls back to the legacy behaviour and the failure is logged.

## Data Structures
- `GridHandle { SpatialGrid* owner; Coord cell; uint32_t objectId; uint32_t generation; }`
  - `generation` increments each time an entry is registered; cells store `[objectId, generation]` to detect stale handles.
  - Methods: `isValid()`, `invalidate()`, `getCell()`, `markCell(Coord)`.
- `SpatialGrid`
  - `std::vector<Cell>`; each `Cell` holds `std::vector<Entry>` where `Entry = { ObjectBase* ptr; uint32_t objectId; uint32_t generation; }`.
  - Optional free list to reuse slots without moving entries (lowers iterator invalidation risk).
- `ScopedGridAssignment`
  - RAII helper for temporary removals (e.g., carryall pickup). Ensures reinsertion even if exceptions/logical aborts occur.

## CRUD Flow

### Create
1. `ObjectManager::addObject` constructs `ObjectBase`.
2. After `objectID` is assigned, call `SpatialGrid::register(ObjectBase&)`.
3. `register` validates object pointer, pulls position via `getLocation()`, and computes target cell.
4. On success: populate `GridHandle`, push `Entry` into cell, set generation.
5. On failure: `GridHandle.invalidate()`, log `SDL_LogWarn`, and leave object out of the grid (callers must tolerate missing handle).

**Race Safeguards**
- `register` acquires the grid mutex (if multi-threaded) before touching cells.
- Generation check ensures we cannot double-register the same object; detect and log duplicates.
- Defer cell insertion if the object is still spawning (e.g., not yet assigned a map tile); provide `postSpawnRegister()` hook.

### Read
1. Query helpers accept `GridHandle` or `(cell, generation)` pairs instead of raw pointers.
2. When iterating a cell, copy the target entries into a small local buffer (`std::array` or `std::vector`).
3. For each entry:
   - Confirm `entry.ptr` is non-null.
   - Verify `entry.objectId == handle.objectId` and `entry.generation == handle.generation`.
   - Check `!entry.ptr->isMarkedForDeletion()`.
4. If any check fails, drop the entry and schedule a maintenance pass to prune the cell.

**Race Safeguards**
- Readers never mutate cells directly; use `SpatialGrid::forEachInCell` to encapsulate copying.
- `ObjectBase` exposes `getGridHandle()` that returns an immutable snapshot (copy) to prevent external mutation.

### Update (Movement / Teleport / Pickup)
1. Movement code calls `SpatialGrid::move(GridHandle&, oldCoord, newCoord)`.
2. `move` early-outs if handle is invalid or positions are equal.
3. Validate both coordinates with `SpatialGrid::isValidCell`.
4. If leaving bounds or entering an invalid tile, call `unregister(handle)` and log; the object will be re-registered when it next enters valid terrain.
5. Remove entry from old cell using generation match; if not found, log and continue (defensive against double-move).
6. Insert into new cell; update handle's stored cell.
7. For teleport/pickup where the object disappears temporarily, `ScopedGridAssignment` calls `SpatialGrid::suspend(handle)` which removes the entry but retains generation; `resume` reinserts using stored data.

**Race Safeguards**
- Removal uses swap-pop to keep O(1) while tracking active iterators only on copied buffers.
- Movement holds the same mutex/lock across removal and insertion to prevent windows where readers see inconsistent state.
- `move` guards against being called during destruction by checking `handle.isValid()` and `object.isAlive()`.

### Delete
1. `ObjectManager::removeObject` calls `SpatialGrid::unregister(GridHandle&)` before invalidating the `objectID`.
2. `unregister` looks up the cell via handle, validates generation, removes the entry, and then `handle.invalidate()`.
3. If the entry is not found, log at debug level to catch double-deletes, but continue safely.
4. After `unregister` returns, teardown continues (selection lists, etc.).

**Race Safeguards**
- `unregister` resilient to repeated calls; second call becomes a no-op because the handle is invalid.
- During mass destruction (e.g., map reset), wrap removals in `SpatialGrid::beginBulkUpdate`/`endBulkUpdate` to batch notifications and cut mutex churn.

## Error Handling & Logging
- `SDL_LogWarn` for recoverable issues (invalid cell, duplicate registration, missing entry on removal).
- `SDL_LogError` for data corruption (generation mismatch with live handle).
- `SDL_assert` in debug builds to identify logic errors without crashing release builds.
- Provide `SpatialGrid::audit()` diagnostic that walks every cell, validates handles, and reports counts for debugging.

## Testing Checklist
- Spawn/kill 10k units rapidly; ensure audit passes and no leaks.
- Teleport/pickup stress test (carryalls, transporter).
- Pause/resume grid updates (simulate asynchronous insertion) without crash.
- Enable sanitizers (ASan/UBSan) and run automated matches to verify no stale pointers.

## Open Questions & Decisions
- **Layering:** A single shared layer will track both ground and air targets. Filtering is handled at query time by allegiance/type checks, so no extra layer is required right now.
- **Pointer ownership:** Keep raw pointers paired with `(objectId, generation)` guards. Switching to `shared_ptr` would propagate through object ownership and may hide ordering bugs; instead we rely on handle validity plus periodic `SpatialGrid::audit()` runs.
- **Threading:** The game loop is currently single-threaded. Implement a `GridLockGuard` stub that compiles to a no-op today but documents the lock boundary so multi-threading can be added later without touching call sites.

## Additional Considerations
- **Lifecycle coverage:** Integrate registration/unregistration inside `ObjectManager::addObject` / `removeObject` (or the corresponding `ObjectBase` hooks) so every trackable object joins/leaves the grid consistently. Movement hooks (`UnitBase::assignToMap`, carryall pickup, teleport) call `SpatialGrid::move`.
- **Audit tooling:** Add a debug-only `SpatialGrid::audit()` that can be triggered from the console or test harness. It scans for invalid handles, duplicate entries, and generation mismatches to catch silent corruption early.
- **Fallback behaviour:** Document how systems should react if the grid operation fails (e.g., fall back to the legacy tile scan, skip pathfinding this frame). This keeps gameplay functional even when the grid is compromised.
- **Configuration toggles:** Provide a runtime flag to disable the grid (for bisecting issues). All callers must handle `handle.isValid() == false` gracefully.

