# Large-Scale Battle Performance Investigation

## Current Situation
- Latest builds introduce targeting/time-slicing for units (5 ms), pathfinding (3 ms) and anti-air extras (turret/launcher quick passes).
- Rocket turrets now reset their scan timer to `40 + rand(0,20)` ticks after each search; timers are seeded randomly at spawn to desynchronise scans.
- We extended the AI loop with a rocket turret immediate-scan queue (2 ms budget) and a launcher quick-pass (max 12 per frame). Idle turrets no longer re-enqueue automatically; they now rely on the timer unless they just lost a target.
- To understand spikes we instrumented the main subsystems. Every ~3600 frames (~1 minute at 60 fps) the game logs frame averages, 95th percentile and max timing for targeting, turret queue, launcher quick-pass and pathfinding to `~/Library/Application Support/Dune Legacy/Dune Legacy.log`.

## Observations So Far
- Even with time budgets, a single `findTarget()` call can cost several ms when grid cells are densely populated. The budget guard fires **after** the scan, so one expensive call can still stall the frame.
- Large “sandbox” maps with all houses active create thousands of units. Baseline per-frame work (structure/unit updates, rendering) plus the targeting queues pushes frame time beyond 16 ms, yielding low FPS.

## Investigation Goals
1. Use the new logging to identify which subsystem (unit targeting, turret queue, launcher pass, pathfinding) dominates frame time in late-game scenarios.
2. Once hotspots are confirmed, consider:
   - Switching from time slices to *count-based* processing (N items per frame) for AA queues to guarantee a hard cap regardless of single-call cost.
   - Revisiting spatial grid parameters or per-cell filtering (e.g., maintain air-only lists) if targeting scans stay expensive.
   - Throttling other subsystems (AI, rendering frequency) or adopting adaptive budgets when FPS drops.
3. Keep gameplay responsive (fast reacquisition of Ornithopters) without allowing any subsystem to monopolise the frame.

## Next Steps Before Further Optimisations
- Run the heavy battlefield scenario and collect logs for several minutes to capture the timing summaries.
- Note fps drops alongside logged stats to correlate frame hits with subsystem spikes.
- Based on data, decide whether to:
  * cap launcher/turret processing by item count,
  * further increase/decrease budgets,
  * pursue spatial-grid tweaks or predictive heuristics.

Deliver this document with timing logs when opening a new discussion so we can pick up the investigation quickly.

---

## Build Notes (Release configuration)

These steps work from the repo root:

```sh
# 1. Inspect available schemes/configurations (optional)
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild \
  -list \
  -project "IDE/xCode/Dune Legacy.xcodeproj"

# 2. Build the release app bundle
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild \
  -project "IDE/xCode/Dune Legacy.xcodeproj" \
  -scheme "Dune Legacy" \
  -configuration Release \
  build
```

The finished app lands in `IDE/xCode/build/Release/Dune Legacy.app`. Adjust `-configuration` (`GameDebug`, `Debug`) if you need one of the other setups exposed in the Xcode project.
