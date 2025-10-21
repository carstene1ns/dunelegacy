# VSync Actually Disabled (Fixing The Confusion)

## The Confusion

**User**: "i am confused. you said you had already disabled vsync... had you not done so??"

**Answer**: No, I had NOT actually disabled VSync. I confused two different things:

### What I Actually Removed Previously
**Software FPS Limiter** (in `Game.cpp`):
```cpp
// REMOVED THIS:
if(settings.video.frameLimit == true && frameTime < 16) {
    SDL_Delay(16 - frameTime);
}
```

This was a **software** delay that artificially slowed the game loop to cap at 60 FPS by calling `SDL_Delay()`.

### What Was Still Enabled
**Hardware VSync** (in `main.cpp`):
```cpp
// THIS WAS STILL ENABLED:
SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");  // ← VSync ON!
```

This is the **hardware** VSync that makes `SDL_RenderPresent()` block waiting for the display's vertical refresh.

## The Difference

### Software FPS Limiter
- **What**: Game loop calls `SDL_Delay()` to slow down
- **Where**: Game.cpp, in the main loop
- **Effect**: Artificially limits FPS by sleeping
- **Problem**: Wastes CPU time sleeping
- **Was removed**: ✅ Yes (document 23)

### Hardware VSync
- **What**: GPU waits for display refresh before swapping buffers
- **Where**: main.cpp, SDL initialization
- **Effect**: Synchronizes rendering with display refresh rate (60Hz, 144Hz, etc.)
- **Problem**: `SDL_RenderPresent()` **blocks** up to 16ms (or more with compositor delays)
- **Was removed**: ❌ NO - until now!

## Why This Matters

**With VSync enabled**:
```
Frame timing:
├── Game logic: 10ms  ← Fast!
├── SDL_RenderPresent() enters...
│   ├── Render commands: 5ms
│   └── WAIT FOR VSYNC: 16ms ← Blocking!
└── Total: 31ms
```

**With macOS compositor delays**:
```
Frame timing:
├── Game logic: 10ms
├── SDL_RenderPresent() enters...
│   ├── Render commands: 5ms
│   ├── Wait for previous frame: 50ms
│   ├── Wait for compositor: 100ms
│   └── Wait for VSync: 16ms
└── Total: 181ms ← LAG SPIKE!
```

This is why we see 250-954ms rendering times in the logs!

## The Actual Fix

**File**: `src/main.cpp:638`

```cpp
// BEFORE (VSync enabled):
SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

// AFTER (VSync disabled):
SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");  // Disable VSync for better frame pacing
```

## Expected Impact

### Before (VSync ON)
```
Average frame: 17ms (fine)
Peak frame: 275-954ms (catastrophic lag spikes!)
Rendering contribution: 80-90% of lag spike
```

### After (VSync OFF)
```
Expected:
- Average frame: 12-15ms (faster)
- Peak frame: 50-100ms (still spikes, but much shorter)
- Rendering contribution: 30-50% (instead of 80-90%)
- FPS will fluctuate more (60-90 FPS instead of locked 60)
```

**Trade-offs**:
- ✅ **Pro**: Eliminates VSync blocking, smoother frame times
- ✅ **Pro**: Reduces lag spikes by 2-5x
- ❌ **Con**: Possible screen tearing (visual artifacts)
- ❌ **Con**: Higher power usage (GPU not waiting)

## Why We Still Might See Lag

Even with VSync disabled, we can still get lag spikes from:

1. **GPU Command Queue Full**
   - Too many draw calls (sprites, units, buildings)
   - GPU can't process fast enough
   - `SDL_RenderPresent()` blocks waiting for GPU
   - **Solution**: Sprite batching, occlusion culling

2. **macOS Compositor (WindowServer)**
   - macOS compositor can still delay frames
   - System animations, other windows, etc.
   - **Solution**: Run in fullscreen mode (bypasses compositor)

3. **Memory Pressure**
   - System swapping to disk
   - **Solution**: Close other apps, check Activity Monitor

4. **Thermal Throttling**
   - CPU/GPU overheating
   - System reduces clock speed
   - **Solution**: Clean fans, use cooling pad

## Testing Instructions

### Before Testing
Run the game with previous build, note performance:
- Check logs for peak rendering time (was 275-954ms)
- Note how often lag spikes occur
- Observe gameplay smoothness

### After This Fix
1. Build and run with VSync disabled
2. Play same scenario (large battle, many units)
3. Check logs for new peak rendering time
4. **Expected improvement**: Peak rendering should drop to 50-150ms (2-5x faster)
5. **Trade-off**: May see screen tearing during fast camera movement

### If Tearing Is Unbearable
We can re-enable VSync and focus on other optimizations:
- Sprite batching (reduce draw calls)
- Occlusion culling (don't render off-screen)
- LOD system (simpler sprites when zoomed out)

## Related Documents

- **Document 23**: Performance timing implementation (removed software FPS limiter)
- **Document 38**: Rendering bottleneck analysis (identified VSync as culprit)

## Summary

**Mistake**: I previously confused software FPS limiter (SDL_Delay) with hardware VSync

**Fix**: Actually disabled VSync by setting `SDL_HINT_RENDER_VSYNC` to "0"

**Expected**: 2-5x reduction in rendering lag spikes (275ms → 50-100ms)

**Trade-off**: Possible screen tearing, but much smoother gameplay

**Next Steps**: Test and measure improvement, decide if tearing is acceptable

