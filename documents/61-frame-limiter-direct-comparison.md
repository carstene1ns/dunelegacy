# Frame Limiter Direct Comparison
**32ms Software vs VSync vs 16ms Software**

---

## The Simple Question

**"Why not just use VSync? This is a 2001 game, surely modern hardware can handle it at 60 FPS?"**

Fair question! Let's evaluate the reality.

---

## Your Actual Performance Data

From your logs with **current 32ms software limiter:**

```
Average frame time: 29ms (components add up to ~24ms)
  Rendering:      6ms
  Pathfinding:   14.5ms  ← THE BOTTLENECK
  AI:             1.1ms
  Units:          1.5ms
  Structures:     0.9ms
  Other:          1ms
  
Peak frame time: 71.7ms (worst case during heavy battle)
  Pathfinding peaked at: 29.29ms
```

**Key insight:** The game is NOT bottlenecked by rendering (6ms). It's bottlenecked by **pathfinding** (14.5ms average, 29ms peak).

---

## Option 1: Current 32ms Software Limiter

### How It Works
```
Timeline:
  0ms:   Start frame
  6ms:   Rendering done
  20ms:  Game logic done (pathfinding 14ms)
  26ms:  Total work complete
  32ms:  SDL_Delay completes, start next frame
  
Result: Consistent 31 FPS, even frame distribution
```

### Pros
✅ **Rock solid stability** - proven in production  
✅ **Handles pathfinding spikes** - 6ms buffer for overruns  
✅ **Consistent frame times** - always 32ms ±2ms  
✅ **Multiplayer tested** - known good behavior  
✅ **Power efficient** - GPU idles during delay  
✅ **Works on ALL hardware** - even slow PCs

### Cons
❌ **Choppy visuals** - 31 FPS is noticeably jerky on 60Hz+ monitors  
❌ **Feels dated** - users expect 60 FPS minimum in 2025  
❌ **Wastes fast hardware** - modern GPUs could render much faster

### Real-World Behavior
**Best case:** Smooth 31 FPS (rendering + logic = 20ms, delay 12ms)  
**Average:** Smooth 31 FPS (rendering + logic = 26ms, delay 6ms)  
**Worst case:** Smooth 31 FPS (rendering + logic = 30ms, delay 2ms)  
**Extreme spike (71ms):** Frame takes 71ms, next frame catches up

**FPS Range:** 31 FPS (locked, with occasional dips to 14 FPS on extreme spikes)

---

## Option 2: VSync Limiter

### How It Works
```
60Hz Monitor Timeline:
  0ms:    Start frame (VSync releases)
  6ms:    Rendering done
  20ms:   Game logic done (pathfinding 14ms)
  26ms:   SDL_RenderPresent() called
  16.67ms: MISSED VSync deadline!
  33.33ms: VSync releases (half refresh rate)
  
Result: Frame took 26ms, but VSync forces wait to 33.33ms
```

### Pros
✅ **No tearing** - hardware synchronized  
✅ **No manual timing code** - SDL/GPU handles it  
✅ **Adapts to monitor** - 60Hz/75Hz/144Hz automatic  
✅ **Industry standard** - professional approach  
✅ **Cleaner code** - remove SDL_Delay logic

### Cons
❌ **Frame rate halving** - misses 60 FPS → drops to 30 FPS  
❌ **Stuttering** - constantly oscillates 30↔60 FPS  
❌ **Pathfinding causes drops** - 14.5ms average + 6ms render = 20.5ms > 16.67ms  
❌ **Worse than 32ms limiter** - feels more choppy due to inconsistency  
❌ **Different monitors behave differently** - 60Hz vs 144Hz players get different frame rates

### Real-World Behavior Based on Your Data

**On 60Hz Monitor (16.67ms per VBlank):**
```
Light frames (pathfinding <6ms):
  Render 6ms + Path 6ms + Other 3ms = 15ms → Hits 60 FPS ✓
  
Average frames (pathfinding 14.5ms):
  Render 6ms + Path 14.5ms + Other 3ms = 23.5ms → MISS → 30 FPS ✗
  
Heavy frames (pathfinding 29ms):
  Render 6ms + Path 29ms + Other 3ms = 38ms → MISS twice → 20 FPS ✗
  
Peak frames (pathfinding spike):
  Total 71ms → MISS 4 times → 14 FPS ✗
```

**FPS Pattern During Gameplay:**
```
Game start (few units): 60 FPS
First battle starts:     60 → 30 FPS (stutter)
More units move:         30 → 60 → 30 (constant oscillation)
Heavy battle:            30 → 20 → 30 (very choppy)
200 units on map:        30 FPS (locked at half rate)
```

**Perceived Experience:** 
- Feels choppier than consistent 31 FPS
- Unpredictable stuttering during gameplay
- Users complain about "lag spikes" even though game logic is fine

### Why VSync Fails Here

**The Problem:** VSync has **binary outcomes**:
- Hit deadline → 60 FPS ✓
- Miss deadline → 30 FPS ✗

With average work time of 23.5ms:
- **Target:** 16.67ms
- **Reality:** 23.5ms
- **Miss rate:** ~100% during normal gameplay

**VSync would work great IF:**
- Average frame time was <15ms (with 1.67ms buffer)
- Pathfinding was <8ms instead of 14.5ms

---

## Option 3: 16ms (60 FPS) Software Limiter

### How It Works
```
Timeline:
  0ms:  Start frame
  6ms:  Rendering done
  20ms: Game logic done (pathfinding 14ms)
  26ms: Total work complete
  
  SDL_Delay(16 - actualTime)?
  actualTime = 26ms, target = 16ms
  Can't delay negative time!
  
Result: Frame takes 26ms anyway, software limiter does nothing
```

### Pros
✅ **Works when it can** - light frames hit 60 FPS  
✅ **No tearing** - unlike unlimited FPS  
✅ **Predictable behavior** - unlike VSync halving  
✅ **Simple implementation** - just change 32 to 16

### Cons
❌ **Limiter is ineffective** - pathfinding routinely exceeds 16ms  
❌ **Variable frame rate** - 23-60 FPS depending on pathfinding  
❌ **Still stutters** - frame time variance causes perceived lag  
❌ **Doesn't solve core issue** - pathfinding still takes 14.5ms average

### Real-World Behavior Based on Your Data

**Frame time distribution:**
```
Minimum frame time: 23ms (6 render + 14 path + 3 other) = 43 FPS max
Average frame time: 26ms = 38 FPS average  
Peak frame time: 71ms = 14 FPS minimum

Actual FPS range: 14-60 FPS (highly variable)
```

**What you'd see:**
```
Menu/idle:           60 FPS (little pathfinding)
Few units:           50-60 FPS  
Normal gameplay:     35-45 FPS (constantly varying)
Heavy battle:        25-35 FPS
200 units:           20-30 FPS
Pathfinding spike:   14 FPS
```

**Perceived Experience:**
- Smoother than 32ms in light scenarios
- Very inconsistent during gameplay
- Feels "laggy" even though game logic is fine
- Users complain about "stuttering"

---

## Side-by-Side Comparison

| Metric | 32ms Software | VSync (60Hz) | 16ms Software |
|--------|---------------|--------------|---------------|
| **FPS Range** | 31 FPS (locked) | 30-60 FPS (binary) | 14-60 FPS (variable) |
| **Consistency** | Excellent | Poor | Poor |
| **Visual Smoothness** | Medium | Poor (stuttery) | Medium (varies) |
| **Power Usage** | Low | Medium | Medium |
| **Tearing** | Yes | No | Yes |
| **Handles pathfinding spikes** | Yes (buffer) | No (drops to 30) | No (slows down) |
| **Multiplayer stability** | Proven | Risky | Acceptable |
| **User perception** | "Choppy but stable" | "Laggy and stuttery" | "Inconsistent" |

---

## The Harsh Reality

**Your pathfinding data shows:**
- Average: 14.5ms
- 16.67ms VSync deadline: **MISS**
- 16ms software target: **EXCEEDED**

**To reliably hit 60 FPS, you'd need:**
- Pathfinding: <8ms (currently 14.5ms)
- Total work: <15ms (currently 23-26ms)

**Math check:**
```
Available time for 60 FPS: 16.67ms
Current work: 23.5ms
Deficit: 6.83ms (41% too slow)
```

---

## Why VSync Performs Worse Than 32ms Software

**Perception of smoothness:**

**Consistent 31 FPS:**
```
Frame times: 32, 32, 32, 32, 32, 32, 32
Perception: Choppy but predictable
```

**VSync oscillating:**
```
Frame times: 16, 16, 33, 33, 16, 33, 16, 33
Perception: Stuttery and jarring (variance is more noticeable than low FPS)
```

**Human perception:** We notice **variance** more than we notice **absolute frame rate**. Consistent 31 FPS feels better than oscillating 30-60 FPS.

---

## Recommendation Based on Reality

### If Keeping Software Limiter

**Keep 32ms** because:
1. Pathfinding budget (12ms) fits within 32ms frame (37% of budget)
2. 6-9ms buffer for spikes
3. Consistent user experience
4. Proven multiplayer stability

**Don't use 16ms** because:
1. Limiter is ineffective (work time exceeds target)
2. Creates false expectation of 60 FPS
3. Inconsistent performance is more noticeable than low but stable FPS

### If Switching to VSync

**Only viable if:**
1. First reduce pathfinding to <8ms (requires refactoring)
2. Or implement multi-frame pathfinding spread (3ms per frame)
3. Test thoroughly - your data suggests it would oscillate 30↔60 constantly

**VSync with current pathfinding = BAD**
- Constant 30-60 oscillation during gameplay
- Feels worse than consistent 31 FPS
- Users would complain about "lag"

---

## The Real Solution

**Your data proves the bottleneck is pathfinding, not rendering.**

**Three viable paths:**

### Path A: Keep Status Quo (Easiest)
- Limiter: 32ms software
- FPS: Consistent 31
- User: "Choppy but playable"
- Effort: 0 hours

### Path B: Allow Unlimited (Short-term)
- Limiter: OFF (checkbox already exists)
- FPS: 23-200 (pathfinding-limited)
- User: "Smooth when possible, naturally slows during battles"
- Effort: 0 hours (already done)

### Path C: Fix Pathfinding + VSync (Best, but work required)
- Implement multi-frame pathfinding spread (3ms per frame)
- Enable VSync
- FPS: Consistent 60 with no tearing
- User: "Smooth and modern"
- Effort: 1-2 weeks

---

## Pragmatic Recommendation

**Ship Path B immediately:** 
- Checkbox already exists
- Let power users disable limiter
- Pathfinding naturally limits FPS (won't go crazy)
- Satisfies "I want smoother graphics" request
- Zero development time

**Plan Path C for next version:**
- Refactor pathfinding to multi-frame spread
- Then VSync becomes viable
- Professional polish

**Never ship 16ms software limiter:**
- It's ineffective given current pathfinding
- Creates false expectations
- Feels worse than 32ms

---

## Test It Yourself

**Quick experiment to validate:**

1. **Try unlimited FPS** (disable frame limiter)
   - Watch FPS counter during gameplay
   - Note: FPS drops to ~40 during battles (pathfinding limit)
   - Ask: "Does this feel better than locked 31?"

2. **Try VSync** (enable SDL_RENDERER_PRESENTVSYNC)
   - Watch FPS counter
   - Note: Oscillates 30-60 constantly
   - Ask: "Does this feel smoother or more stuttery?"

3. **Try 16ms software** (change 32→16)
   - Watch FPS counter  
   - Note: FPS varies 25-60
   - Ask: "Is inconsistent 40 FPS better than consistent 31?"

**My prediction:** You'll find unlimited FPS (Path B) feels best because pathfinding naturally smooths the experience.

---

## Bottom Line

**Why not VSync?**
- Because pathfinding takes 14.5ms average, missing the 16.67ms deadline
- Result: Constant 30 FPS or oscillating 30↔60
- Feels worse than consistent 31 FPS

**Why not 16ms software?**
- Because limiter is ineffective when work exceeds target
- Result: Variable 25-60 FPS
- Inconsistency is jarring

**Why 32ms works:**
- Buffer accommodates pathfinding variance
- Consistent experience
- Proven stable

**Best short-term answer:**
- **Remove limiter entirely** (already done)
- Let pathfinding naturally limit FPS
- Smooth when possible, naturally throttles during heavy gameplay

---

**TL;DR:** VSync and 16ms software limiter both fail because pathfinding takes 14.5ms average, which exceeds the 16.67ms VSync deadline and 16ms target. Result is stuttering that feels worse than consistent 31 FPS. Best immediate solution: make limiter optional (already done). Best long-term solution: refactor pathfinding, then enable VSync.

