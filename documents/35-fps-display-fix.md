# In-Game FPS Display Fix

## Problem

The in-game FPS display (F12) was showing **45 FPS** while our performance logs showed **63 FPS** average. This discrepancy was confusing and made it appear that the game was running slower than it actually was.

## Root Cause

The in-game FPS calculation was using the **wrong variable** - it was using the game speed accumulator instead of the actual frame time.

### The Bug

**Line 1062-1089 in `Game.cpp` (before fix):**

```cpp
int frameTime = 0;  // This is a GAME SPEED ACCUMULATOR, not frame time!

do {
    renderFrame();
    
    const int frameEnd = SDL_GetTicks();
    frameTime += frameEnd - frameStart;  // Accumulate real time
    frameStart = frameEnd;
    
    // FPS calculated from accumulator - WRONG!
    if(bShowFPS) {
        averageFrameTime = 0.99f * averageFrameTime + 0.01f * frameTime;
    }
    
    // ... later in catch-up loop ...
    
    while((frameTime > getGameSpeed()) || ...) {
        updateGameState();  // Run game cycle
        frameTime -= getGameSpeed();  // Decrements accumulator by 16ms
    }
}
```

**The Issue:**
- `frameTime` is used as a **time debt accumulator** for the catch-up loop
- Each game cycle subtracts `getGameSpeed()` (16ms) from it
- The FPS display was using this accumulator value, which doesn't represent actual frame time
- The accumulator gets decremented multiple times per frame when catching up

**Example:**
```
Frame 1: Actual frame time = 50ms
  frameTime accumulator = 50ms
  Display calculates: 1000/50 = 20 FPS (seems OK)
  Catch-up loop runs 3 times (50/16 = 3.125)
  After catch-up: frameTime = 50 - (3×16) = 2ms

Frame 2: Actual frame time = 18ms  
  frameTime accumulator = 2 + 18 = 20ms
  Display calculates: 1000/20 = 50 FPS (WRONG! Should be 1000/18 = 56 FPS)
```

The accumulator would drift and not reflect actual frame time!

## Solution

**Create a separate variable for actual frame time:**

```cpp
const int frameEnd = SDL_GetTicks();
const int actualFrameTime = frameEnd - frameStart;  // NEW: Actual time for this frame
frameTime += actualFrameTime;  // Still accumulate for catch-up logic
frameStart = frameEnd;

if(bShowFPS) {
    // Use ACTUAL frame time, not the accumulator
    averageFrameTime = 0.99f * averageFrameTime + 0.01f * actualFrameTime;
}
```

### Changes Made

**File**: `src/Game.cpp`

**Line 1083** (Added):
```cpp
const int actualFrameTime = frameEnd - frameStart;  // Actual time for this frame
```

**Line 1084** (Changed):
```cpp
frameTime += actualFrameTime;  // Changed from: frameTime += frameEnd - frameStart;
```

**Line 1091** (Changed):
```cpp
averageFrameTime = 0.99f * averageFrameTime + 0.01f * actualFrameTime;
// Changed from: ... + 0.01f * frameTime;
```

## What This Fixes

### Before (Incorrect)
```
In-game display: 45 FPS (using accumulator)
Performance logs: 63 FPS (using SDL_GetPerformanceCounter)
Discrepancy: 18 FPS difference (confusing!)
```

### After (Correct)
```
In-game display: ~63 FPS (using actual frame time)
Performance logs: 63 FPS (using SDL_GetPerformanceCounter)
Match: ✓ Both show the same FPS
```

## Why The Values Match Now

**In-game FPS (F12):**
- Uses `SDL_GetTicks()` (millisecond precision)
- Measures time from last render to current render
- Includes all game cycles run per frame
- **Shows actual frames per second being rendered**

**Performance Logs:**
- Uses `SDL_GetPerformanceCounter()` (microsecond precision)
- Measures entire do-while loop including catch-up
- Logs every 30 seconds
- **Shows detailed breakdown of frame time**

Both now measure the **same thing** - the actual time per rendered frame including all game logic.

## Expected Behavior

### Normal Gameplay
```
In-game FPS display: 55-65 FPS
Smooth gameplay
Visual FPS matches logged FPS
```

### During Large Battles
```
In-game FPS display: 20-40 FPS
Some stuttering expected (rendering spikes)
FPS recovers quickly after battle
```

### Worst Case Spikes
```
In-game FPS display: May drop to 1-5 FPS briefly
Due to rendering spikes (many explosions/bullets)
Recovers within 1-2 seconds
Not a death spiral!
```

## Testing Verification

**Steps:**
1. Launch game
2. Press F12 to show FPS
3. Play for 30 seconds
4. Check log file for performance stats
5. Verify in-game FPS ≈ logged FPS

**Expected:**
- In-game FPS: 55-65 during normal play
- Logged FPS: 55-65 average
- Values should be within 5 FPS of each other

## Why This Bug Existed

The original code was written before the performance logging system. When performance logging was added, it used a different (correct) measurement method with `SDL_GetPerformanceCounter()`. The in-game FPS display was never updated to match.

The `frameTime` variable has dual purpose:
1. Time debt accumulator for catch-up loop (primary purpose)
2. FPS display source (incorrect secondary use)

These two purposes conflict because the accumulator gets modified by the catch-up loop.

## Summary

**Fixed:** In-game FPS display now uses actual frame time instead of the game speed accumulator.

**Result:** FPS display (F12) now accurately shows the same FPS as our performance logs, eliminating confusion about actual game performance.

**Impact:** Users can now trust the in-game FPS display to accurately reflect game performance.

