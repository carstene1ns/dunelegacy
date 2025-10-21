# QuantBot Build Order Upgrade Plan

## Goals
- Modernise the main-branch `QuantBot` with the more sophisticated build-order and economy logic seen in the 0.98.5 AI.
- Keep the code modular so future economy tweaks can be added without touching the entire bot.
- Ensure the new logic is configurable (campaign vs custom, difficulty) and well-instrumented for debugging.

## Scope
1. Track map spice and dynamically cap harvester/refinery production.
2. Refine early build priorities (Heavy Factory first, campaign restoration of initial structures, windtrap/silo/rocket-turret rules).
3. Improve placement heuristics (edge scanning, two-tile buffer, slab fallback).
4. Allow High-Tech Factory expansion when all existing ones are busy.

## Non-Goals
- Rewriting full unit-micro or combat behaviour (tactical retreat/orni logic already covered elsewhere).
- Introducing new UI options—changes are code-level defaults with existing game settings.

## Dependencies / Shared Utilities
- `currentGame` / `currentGameMap` accessors.
- Existing placement helpers (`findPlaceLocation`) and map-perimeter checks.
- Logging utility `logDebug` for instrumentation.

## Implementation Steps

### 1. Economy Tracking
**Files:** `QuantBot.h`, `QuantBot.cpp`
- [ ] Add `lastCalculatedSpice` member, persist in save/load, and expose a debug accessor/log entry.
- [ ] Create helper (e.g.) `FixPoint QuantBot::calculateRemainingSpice()` to iterate every tile and sum `Tile::getSpice()`.
- [ ] Recompute `lastCalculatedSpice` at the top of every AI update tick.
- [ ] Derive harvester limit from remaining spice:
  - Custom games: `harvesterLimit = max(1, remainingSpice / kSpicePerHarvester)` with initial constant 2000 (tune later).
  - Campaign games: honour difficulty presets, but clamp down if the spice-based limit is lower.
- [ ] When the limit decreases, skip queueing additional harvesters/refineries; consider cancelling queued harvesters that would exceed the cap.
- [ ] Emit debug logs whenever the limit or remaining spice changes materially.

### 2. Build Priority Rules
**Files:** `QuantBot.cpp`
- [ ] Ensure early-game default counts reflect 0.98.5 (auto-allow Repair Yard/Radar/Light Factory in campaign missions above tech 4).
- [ ] Construction Yard loop:
  - Campaign mode: iterate structure IDs and rebuild until `itemCount[i] == initialItemCount[i]`.
  - Repair yard first: if the yard is damaged, repair before building; upgrade yard only when idle, not already upgrading, and harvesters meet the limit.
  - Windtrap: build only if power deficit and count < initial count.
  - Silo: build when stored credits ≥ 90 % capacity; skip otherwise.
  - Rocket turret: respect “needs power” option; skip if power shortfall.
- [ ] Heavy Factory gating:
  - Light Factory / WOR / Barracks / infantry build checks must confirm `itemCount[Structure_HeavyFactory] == 0` before queueing “light” techs.
- [ ] Palace builds:
  - Require both Heavy and Light factory and respect `GameOptions::onlyOnePalace`.
- [ ] Repair Yard ratio: maintain “1 repair yard per ~6000 military value” rule.
- [ ] Windtrap cap: stop building once reaching the initial count (unless others destroyed).
- [ ] High-Tech factory expansion: if every existing high-tech is busy, allow queuing another (bounded by budget and placement).
- [ ] Silo builds remain based on storage thresholds.

### 3. Placement Enhancements
**Files:** `QuantBot.h`, `QuantBot.cpp`
- [ ] Add `Coord findPlaceLocationSimple(Uint32 itemID)`:
  - Quick scan across the full map (including edges) for any passable tile.
  - No adjacency scoring; used for slabs or emergency placements.
- [ ] Update `findPlaceLocation`:
  - Iterate from 0 … mapSize (edges included).
  - Evaluate a two-tile buffer around the candidate rather than one.
  - Special-case scoring for Rocket Turrets (prefer facing enemy).
- [ ] When `doProduceItem` fails because no valid placement exists:
  - queue `Structure_Slab1` (if available) using `findPlaceLocationSimple`.
  - Retry original structure after slab placement.

### 4. High-Tech Factory Expansion
**Files:** `QuantBot.cpp`
- [ ] During builder evaluation, compute `activeHighTechFactoryCount`.
- [ ] If every high-tech factory is currently busy producing, and we have enough credits, queue an additional High-Tech factory (subject to map placement).

## Testing Checklist
- Campaign start (tech > 4): verify Repair Yard/Radar/Light Factory are queued appropriately.
- Custom skirmish with low spice: confirm harvester limit drops and new refineries/harvesters stop.
- Observe AI base building:
  - Heavy Factory built ASAP; Light/WOR follow only afterward.
  - Palace builds only when requirements satisfied.
  - When placement space is tight, AI lays slabs then resumes structures.
  - Additional High-Tech factories appear when existing ones stay busy.
- Regression: ensure save/load persists new fields, no crash from missing `currentGame`.

## Rollout Notes
- Merge after unit-micro changes to avoid mid-branch merge conflicts (both touch `QuantBot.cpp` heavily).
- When backporting, keep a feature flag (optional) if we need to A/B test old vs new behaviour.
