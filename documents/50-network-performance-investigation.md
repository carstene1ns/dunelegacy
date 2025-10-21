# Network Performance Investigation & Improvement Options

**Date**: 2025-10-21  
**Branch**: `release-0.98.6`  
**Status**: 📋 Planning

## Current Architecture

### Lockstep / Deterministic Simulation
Dune Legacy uses a **deterministic lockstep** network model:

```cpp
// Game.cpp:2694
if(pPlayer->nextExpectedCommandsCycle <= gameCycleCount) {
    bWaitForNetwork = true;  // HALT THE ENTIRE GAME
    SDL_Delay(10);
}
```

**How it works:**
1. Every frame, each player sends their commands (`CommandList`) to all other players
2. Commands are scheduled `networkCycleBuffer` cycles ahead (based on RTT)
3. Game **stops completely** if any player hasn't sent their commands yet
4. Everyone executes the exact same commands on the exact same game cycle
5. Guarantees perfect determinism (no desyncs if simulation is deterministic)

**Pros:**
- ✅ Zero bandwidth (~10KB/s per player - just commands)
- ✅ Perfect synchronization (everyone sees identical game state)
- ✅ Easy to implement replays (just record commands)
- ✅ Works with deterministic simulation
- ✅ Scales well to many units

**Cons:**
- ❌ **Slowest player bottlenecks everyone** (visible as network wait spikes)
- ❌ High latency players ruin experience for everyone
- ❌ Any lag = game freezes for all players
- ❌ Can't support large player counts well (8+ players)
- ❌ Visual stuttering during network waits

### Current Performance Issues

From logs (multiplayer session):
```
[Performance] NetworkWait: min=0.00ms avg=3.04ms max=69.45ms
[Performance] Rendering spike: 1016.62ms (1 second freeze!)
```

**Problems:**
1. Network wait can spike to 70-130ms (4-8 missed frames)
2. Rendering has massive unexplained spikes
3. ~14ms of frame time unaccounted for (60-65% of total)
4. Visual hitching during network synchronization

---

## Improvement Options

### 🟢 **Option 1: Keep UI Alive During Network Wait** (QUICK WIN)

**Effort:** 1-2 days  
**Risk:** Very Low  
**Complexity:** ⭐ Easy

**Changes:**
```cpp
if(bWaitForNetwork) {
    // Keep UI responsive while waiting
    renderUI();  // Just the interface, not the game world
    SDL_PumpEvents();  // Keep cursor responsive
    showWaitingIndicator();  // Visual feedback
    SDL_Delay(10);
}
```

**Benefits:**
- Cursor doesn't freeze
- UI shows "waiting for players" indicator
- Players know the game is working, not crashed

**Risks:**
- Almost none - UI rendering is already separate

---

### 🟢 **Option 2: Re-render Frozen State** (RECOMMENDED)

**Effort:** 2-4 days  
**Risk:** Low  
**Complexity:** ⭐⭐ Moderate

**Changes:**
```cpp
while(bWaitForNetwork) {
    // DON'T update game state (keep simulation frozen)
    // BUT keep rendering the same state at 60 FPS
    renderFrame();  // Re-draw the last valid game state
    processInput();  // Keep UI responsive
    
    // Check if network caught up
    pNetworkManager->update();
    bWaitForNetwork = handleNetworkUpdates();
    
    SDL_Delay(10);  // Prevent CPU spinning
}
```

**Benefits:**
- Game doesn't visually freeze
- Maintains 60 FPS even during network wait
- Units appear "paused" rather than "teleporting"
- Cursor stays smooth
- No desync risk (not advancing simulation)

**Risks:**
- Need to ensure `renderFrame()` is idempotent (safe to call multiple times)
- Must verify no state changes during render

**Implementation Notes:**
- Game state remains frozen (no `updateGameState()` calls)
- Only visual rendering continues
- Network wait time still exists, but hidden visually

---

### 🟡 **Option 3: Improved Lockstep with Lag Tolerance**

**Effort:** 2-4 weeks  
**Risk:** Medium  
**Complexity:** ⭐⭐⭐ Moderate-High

**Changes:**

1. **Adaptive Buffer:**
   ```cpp
   // Dynamically adjust based on actual RTT
   networkCycleBuffer = MILLI2CYCLES(maxPeerRTT * 1.5 + 50);
   ```

2. **Lag Tolerance:**
   ```cpp
   if(nextExpectedCommandsCycle < gameCycleCount - MAX_LAG_TOLERANCE) {
       // Player is too far behind - drop them
       dropPlayer(player);
   } else if(nextExpectedCommandsCycle <= gameCycleCount) {
       // Predict missing commands (e.g., "no action")
       predictPlayerCommands(player);
   }
   ```

3. **Better Player Feedback:**
   - Show each player's ping/lag in-game
   - Visual indicator for "slow" players
   - Auto-pause when lag is too severe
   - Kick players consistently > 500ms behind

4. **Async Rendering:**
   - Decouple rendering from simulation tick
   - Continue rendering at 60 FPS even during short waits

**Benefits:**
- Handles transient lag better
- Doesn't penalize all players for one slow player
- Better user feedback about network state
- Maintains determinism

**Risks:**
- Command prediction could cause visual glitches if wrong
- Need robust desync detection (periodic checksums)
- More complex state management

**Estimated Impact:**
- Reduces perceived stuttering by ~70%
- Allows play with up to 200ms latency
- Better handling of packet loss

---

### 🟡 **Option 4: Visual Extrapolation** (COSMETIC ENHANCEMENT)

**Effort:** 1-2 weeks  
**Risk:** Medium  
**Complexity:** ⭐⭐⭐ Moderate

**Changes:**
```cpp
// Separate visual state from simulation state
struct Unit {
    Coord gamePosition;      // Authoritative (synced)
    Coord visualPosition;    // Visual only (extrapolated)
    Coord velocity;
};

if(bWaitForNetwork) {
    // Don't update REAL game state
    // But extrapolate visual positions for smoothness
    
    for(Unit* unit : unitList) {
        if(unit->isMoving()) {
            // Predict next visual position along path
            unit->visualPosition += unit->velocity * deltaTime;
        }
    }
    
    renderFrame();  // Draw the extrapolated positions
}

// When network arrives, snap back to truth
updateGameState();
for(Unit* unit : unitList) {
    unit->visualPosition = unit->gamePosition;  // Reset
}
```

**Benefits:**
- Units continue moving smoothly during brief network waits
- Hides latency < 100ms almost completely
- Professional "AAA game" feel

**Risks:**
- Visual state can diverge from game state (units might "rubber-band")
- More memory (two positions per entity)
- Complexity in render code

**Best Used With:** Option 2 or 3

---

### 🔴 **Option 5: Client-Side Prediction + Server Authority** (FULL REWRITE)

**Effort:** 6-12 months  
**Risk:** Very High  
**Complexity:** ⭐⭐⭐⭐⭐ Very High

**Architecture:**
```
Client A                Server                Client B
  |                       |                      |
  | 1. Predict & Send --> |                      |
  | 2. Show immediate     | 3. Validate          |
  |    result             | 4. Broadcast ------> | 5. Interpolate
  | 6. Reconcile <--------|                      |
```

**Major Changes Required:**

1. **Server Authority:**
   - Server becomes authoritative
   - Move game logic to server
   - Clients send inputs, not commands
   - Server validates and broadcasts state updates

2. **Client Prediction:**
   - Duplicate game logic on client
   - Store last N predicted states
   - Apply local inputs immediately
   - Rollback/replay when server correction arrives

3. **State Synchronization:**
   - Send full game state snapshots (20-60Hz)
   - Delta compression for bandwidth
   - Snapshot interpolation for smooth playback

4. **Reconciliation:**
   - Store input history
   - When server state arrives, rewind and replay
   - Detect mispredictions and correct

**Benefits:**
- ✅ Instant local response (0ms perceived latency)
- ✅ Handles high latency well (200-300ms playable)
- ✅ No waiting for slow players
- ✅ Server can validate actions (anti-cheat)
- ✅ Modern multiplayer experience

**Drawbacks:**
- ❌ **Massive bandwidth increase** (100-500KB/s vs 10KB/s)
- ❌ **Complete rewrite** of network code
- ❌ Lose perfect determinism
- ❌ Replay system needs redesign
- ❌ Visual artifacts (prediction errors, rubber-banding)
- ❌ 6-12 months development time

**Not Recommended For:**
- RTS games (lockstep is standard)
- Projects with limited resources
- Games requiring perfect sync

---

### 🟡 **Option 6: Hybrid Approach** (BEST OF BOTH WORLDS)

**Effort:** 2-3 months  
**Risk:** High  
**Complexity:** ⭐⭐⭐⭐ High

**Architecture:**
- Keep lockstep for core simulation
- Add client-side prediction for **local player only**
- Server validates and corrects if needed
- Other players use interpolation

**Benefits:**
- Local player gets instant feedback
- Maintains determinism for recording/replay
- Other players see smooth movement
- More tolerant of lag

**Drawbacks:**
- Complex dual-system
- Prediction errors can be jarring
- Still requires significant development

---

## Recommendations (Priority Order) 📋

### Phase 1: Quick Wins (1-2 weeks)
1. ✅ **Option 1: Keep UI Alive** - Immediate improvement
2. ✅ **Option 2: Re-render Frozen State** - Huge perceived improvement
3. ✅ **Investigate Rendering Spike** - Fix 1016ms freeze
4. ✅ **Better Network Indicators** - Show ping/lag in-game

**Expected Impact:** 70% reduction in perceived stuttering

### Phase 2: Lockstep Improvements (1-2 months)
5. 🔄 **Option 3: Lag Tolerance** - Handle transient lag better
6. 🔄 **Option 4: Visual Extrapolation** - Smooth out brief waits
7. 🔄 **Adaptive Buffers** - Optimize for actual network conditions

**Expected Impact:** Playable up to 200ms latency, fewer disconnects

### Phase 3: Future Consideration (6+ months)
8. ⏸️ **Option 6: Hybrid System** - If player feedback demands it
9. ⏸️ **Option 5: Full Rewrite** - Only if absolutely necessary (probably never)

---

## Testing Plan 🧪

For each option implemented:

1. **Unit Tests:**
   - Verify no state changes during render
   - Ensure determinism maintained
   - Test network buffer calculations

2. **Integration Tests:**
   - 2 players, varying latency (50ms, 100ms, 200ms)
   - Packet loss simulation (1%, 5%, 10%)
   - One player with high latency (300ms+)
   - 4+ player games

3. **Metrics to Track:**
   - Network wait time (min/avg/max)
   - Perceived frame rate during waits
   - Desync frequency
   - Player disconnects
   - User feedback on "smoothness"

---

## Current Unknowns ❓

1. **What causes 1016ms rendering spike?** (Priority: High)
2. **Where is the missing 14ms of frame time?** (Priority: Medium)
3. **Why is pathfinding barely used?** (0.32ms max vs 12ms budget)
4. **What is optimal networkCycleBuffer?** (Currently RTT-based)

---

## References 📚

- **Lockstep RTS Networking:** https://gafferongames.com/post/deterministic_lockstep/
- **Client-Side Prediction:** https://gabrielgambetta.com/client-side-prediction-server-reconciliation.html
- **Age of Empires Networking:** GDC talks on AoE2 netcode
- **StarCraft II Lag Compensation:** Blizzard engineering blogs

---

## Next Steps 🎯

1. **Immediate:** Investigate rendering spike (1016ms)
2. **This Week:** Implement Option 1 + Option 2 (keep UI alive, re-render frozen state)
3. **This Month:** Option 3 (lag tolerance) if Phase 1 isn't sufficient
4. **Re-evaluate:** Based on player feedback after Phase 1

---

**Status:** Ready for implementation  
**Decision Needed:** Approve Phase 1 quick wins?

