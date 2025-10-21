# Fix Network Multiplayer

This note captures the work done to restore multiplayer synchronization after the FPS optimisation regressions.

## Issue Overview
- The frame pacing refactor removed the original frame limiting behaviour, letting the game loop run disconnected from network pacing.
- During multiplayer matches both clients entered a permanent wait state because the simulation advanced even while network commands were still outstanding.
- The command buffer stopped broadcasting local commands while the “Waiting for other players…” overlay was active, so peers never caught up and the overlay never dismissed.

## Changes Applied
1. **Respect network readiness before advancing game state**  
   - `processNetwork()` now returns a boolean so the update loops skip `updateGameState()` if any peer is still catching up (`src/Game.cpp:945`, `src/Game.cpp:960`, `src/Game.cpp:1079`).  
   - Prevents `gameCycleCount` from moving ahead while we wait on remote input.

2. **Continue sharing buffered commands while waiting**  
   - `processInput()` keeps the command manager ticking every frame, even with menus or the wait overlay visible (`src/Game.cpp:1076`).  
   - Ensures local commands are flushed to peers so they can catch up.

3. **Reset menu state once peers recover**  
   - When the wait dialog auto-closes, `bMenu` clears unless another menu remains open (`src/Game.cpp:2412`).  
   - Removes the stuck “menu” flag that previously blocked simulation from resuming.

4. **Housekeeping for the updated loop**  
   - `updateGameState()` now assumes the command manager already ran this frame, keeping command execution single-sourced (`src/Game.cpp:1096`).  

5. **Loop simplification to avoid over-engineering**  
   - Reverted the accumulator-based fixed timestep and `std::pow` speed math to the legacy frame-time loop (`src/Game.cpp:905`).  
   - Removed the redundant `processNetwork()` helper and now call `updateGameState()` directly once the frame-time budget allows, keeping network waits in a single place (`include/Game.h:618`, `src/Game.cpp:935`).  
   - Maintains the earlier network safeguards while making the main loop easier to reason about.

6. **Restored multiplayer pause**  
   - `pauseGame()` now always sets `bPause`, enabling the spacebar toggle during multiplayer sessions (`src/Game.cpp:1137`, `include/Game.h:203`).  
   - Toggling pause sends in-game notifications and broadcasts a chat message so all peers see when the game stops/resumes (`src/Game.cpp:2024`).  

## Verification Notes
- Builds compile cleanly after the changes.
- Multiplayer sessions should be tested on at least two machines to confirm the waiting overlay clears when both peers receive pending commands.
- Additional regression checks: pause/resume flow, in-game menus, and replay playback (all rely on the same loop).
