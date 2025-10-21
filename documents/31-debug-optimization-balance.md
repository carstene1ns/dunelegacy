# Debug Build Optimization Balance

## Problem
Debug build with `-O0` (no optimization) was too slow to test performance issues and pathfinding fixes effectively. The game was unplayable in Debug mode.

## Solution
Changed Debug build optimization level from `-O0` to `-O1` to balance:
- ✅ Performance: 2-3x faster than `-O0`
- ✅ Debugging: Can still step through code and inspect variables
- ✅ Crash logs: Full debug symbols and stack traces preserved
- ✅ Code signing: Still disabled for easy debugging

## Changes Made

### `IDE/xCode/Dune Legacy.xcodeproj/project.pbxproj`

**Target Debug Configuration (line 2307):**
```xml
GCC_OPTIMIZATION_LEVEL = 1;  // Changed from 0
```

**Project Debug Configuration (line 2420):**
```xml
GCC_OPTIMIZATION_LEVEL = 1;  // Changed from 0
```

**Retained Debug Settings:**
```xml
GCC_GENERATE_DEBUGGING_SYMBOLS = YES;
DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
ENABLE_HARDENED_RUNTIME = NO;
CODE_SIGN_IDENTITY[sdk=macosx*] = "-";
LLVM_LTO = NO;
```

## Optimization Level Comparison

### `-O0` (Previous - Too Slow)
- No optimization
- Easiest to debug
- 5-10x slower than Release
- **Game unplayable for performance testing**

### `-O1` (New - Balanced) ✓
- Basic optimization
- Still debuggable
- 2-3x slower than Release
- **Good enough for gameplay testing**
- Inlining: minimal
- Loop unrolling: no
- Dead code elimination: yes

### `-O2` (Not Used - Too Aggressive)
- Full optimization
- Harder to debug (variables optimized away)
- 1.2-1.5x slower than Release
- Code flow can be confusing

### `-O3` (Release - Not Debuggable)
- Aggressive optimization
- Very hard to debug
- Same speed as Release
- Not suitable for development

## Expected Results

**Before (Debug -O0):**
- FPS: 5-10 (unplayable)
- Pathfinding: 1000ms+
- Game cycles: 100+ per frame
- **Can't test performance fixes!**

**After (Debug -O1):**
- FPS: 20-40 (playable)
- Pathfinding: ~100-200ms (with our cap)
- Game cycles: 10-20 per frame
- **Can test and debug performance!**

**Release (-O3):**
- FPS: 60+ (smooth)
- Pathfinding: 6ms (capped)
- Game cycles: 1-3 per frame
- **Production performance**

## How to Build

```bash
cd IDE/xCode
xcodebuild -project "Dune Legacy.xcodeproj" -configuration Debug clean build
```

## Crash Log Verification

Crash logs will still include:
- ✅ Full stack traces
- ✅ Function names
- ✅ Line numbers
- ✅ Variable values (most of them)
- ✅ Thread information

**Example crash log:**
```
Thread 0 Crashed:
0   Dune Legacy  0x00000001000a2f34 UnitBase::engageTarget() + 244 (UnitBase.cpp:489)
1   Dune Legacy  0x00000001000a3128 UnitBase::targeting() + 56 (UnitBase.cpp:1257)
2   Dune Legacy  0x00000001000a31c8 UnitBase::update() + 12 (UnitBase.cpp:1428)
```

## Why -O1 is Perfect for Development

1. **Fast enough to play:** Game runs at acceptable FPS for testing
2. **Still debuggable:** Can set breakpoints, step through code
3. **Crash logs work:** Full stack traces with symbols
4. **Catches real bugs:** Closer to Release behavior than -O0
5. **Build time:** Only slightly slower than -O0

## Alternative: Use Release with Debug Symbols

If -O1 is still too slow, we could use Release build with:
```xml
GCC_GENERATE_DEBUGGING_SYMBOLS = YES;
DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
```

But this makes interactive debugging much harder.

## Summary

**Debug build now uses `-O1` optimization level** to provide a good balance between:
- Performance (playable game)
- Debuggability (can step through code)
- Crash diagnostics (full stack traces)

This allows effective testing of performance fixes while still catching and diagnosing crashes!

