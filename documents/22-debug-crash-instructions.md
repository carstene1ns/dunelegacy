# Debug Build & Crash Diagnosis Instructions

## What Was Fixed

### Problem
The **Debug** configuration was building like Release:
- ❌ `GCC_GENERATE_DEBUGGING_SYMBOLS = NO`
- ❌ `GCC_OPTIMIZATION_LEVEL = 3` (fully optimized)
- ❌ `LLVM_LTO = YES` (link-time optimization)
- ❌ Vectorization and fast-math flags enabled

**Result**: Crashes showed garbage in stack traces, variables unavailable, hard to debug.

### Solution (Commit ce5ec22)
Fixed Debug configuration for proper crash diagnosis:
- ✅ `GCC_GENERATE_DEBUGGING_SYMBOLS = YES`
- ✅ `GCC_OPTIMIZATION_LEVEL = 0` (no optimization)
- ✅ `DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym"`
- ✅ `LLVM_LTO = NO` (disabled link-time optimization)
- ✅ `ONLY_ACTIVE_ARCH = YES` (faster debug builds)
- ✅ Removed vectorization flags

**Result**: Crash logs will show:
- Real function names (not optimized away)
- Actual line numbers
- Variable values
- Complete stack traces

---

## How to Run Debug Build with lldb

### Step 1: Navigate to Debug Build
```bash
cd /Users/stefanvanderwel/development/dune/dunelegacy/IDE/xCode/build/Debug
```

### Step 2: Start lldb
```bash
lldb -- "./Dune Legacy.app/Contents/MacOS/Dune Legacy"
```

**You'll see**:
```
(lldb) target create "./Dune Legacy.app/Contents/MacOS/Dune Legacy"
Current executable set to '/Users/.../Dune Legacy' (arm64).
(lldb) 
```

### Step 3: Set Breakpoints (Optional)
If you want to catch specific issues:

```lldb
# Break on any exception
breakpoint set -n __cxa_throw

# Break on abort/crash
breakpoint set -n abort

# Break in specific function (e.g., Tile::squash)
breakpoint set -n Tile::squash

# Break at specific file:line
breakpoint set -f Tile.cpp -l 643
```

### Step 4: Run the Game
```lldb
run
```

The game will start. Play until you hit the crash or issue.

### Step 5: When Crash Occurs

lldb will pause at the crash. You'll see something like:
```
Process 12345 stopped
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_BAD_ACCESS (code=1, address=0x18d)
    frame #0: 0x00000001001bdaa8 Dune Legacy`Tile::squash(this=0x...) at Tile.cpp:643:22
```

### Step 6: Get Stack Trace
```lldb
bt
```

**or for full trace**:
```lldb
bt all
```

**You'll see**:
```
* thread #1, queue = 'com.apple.main-thread':
  * frame #0: Tile::squash(this=0x...) at Tile.cpp:643
    frame #1: UnitBase::move(this=0x...) at UnitBase.cpp:633
    frame #2: UnitBase::update(this=0x...) at UnitBase.cpp:1434
    frame #3: Game::processObjects(this=0x...) at Game.cpp:322
    ...
```

### Step 7: Inspect Variables
```lldb
# Print local variables
frame variable

# Print specific variable
p objectID
p infantryList.size()
p location

# Print this pointer contents
p *this

# Print member variable
p this->assignedInfantryList
```

### Step 8: Move Up/Down Stack
```lldb
# Go up one frame
up

# Go down one frame
down

# Go to specific frame
frame select 2

# Show current frame code
frame info
list
```

### Step 9: Continue or Quit
```lldb
# Continue execution (if breakpoint not crash)
continue

# Quit lldb
quit
```

---

## Quick Reference Commands

### Essential lldb Commands
```bash
run                    # Start the program
bt                     # Backtrace (stack trace)
frame variable         # Show local variables
p <expr>              # Print expression/variable
list                  # Show source code
up / down             # Navigate stack frames
continue              # Resume execution
quit                  # Exit lldb
```

### Advanced Commands
```bash
breakpoint list       # List all breakpoints
breakpoint delete 1   # Delete breakpoint 1
watchpoint set        # Watch variable changes
thread list           # List all threads
image lookup -a ADDR  # Find symbol at address
disassemble           # Show assembly code
```

---

## What to Look For in Crashes

### 1. Iterator Invalidation (Tile::squash)
**Symptom**: Crash in `std::list` or `std::__tree`
**Location**: `Tile::squash()` iterating `assignedInfantryList`
**Cause**: List modified while iterating

**Debug**: 
```lldb
p assignedInfantryList.size()
p *iter
```

### 2. Null/Invalid Pointers
**Symptom**: `EXC_BAD_ACCESS (code=1, address=0x0)` or low address
**Cause**: Dereferencing null or deleted object

**Debug**:
```lldb
p target.getObjPointer()
p this->location
```

### 3. Off-Map Coordinates
**Symptom**: Invalid tile access, array out of bounds
**Location**: `getTile(x, y)` or similar

**Debug**:
```lldb
p location
p currentGameMap->getSizeX()
p currentGameMap->getSizeY()
```

### 4. Units Through Buildings
**Symptom**: Collision detection fails
**Location**: `UnitBase::canPass()` or `Tile::hasAGroundObject()`

**Debug**:
```lldb
p pTile->assignedNonInfantryGroundObjectList.size()
p pTile->getGroundObject()
```

---

## Example Debugging Session

```bash
$ cd /Users/stefanvanderwel/development/dune/dunelegacy/IDE/xCode/build/Debug
$ lldb -- "./Dune Legacy.app/Contents/MacOS/Dune Legacy"

(lldb) run
# ... game starts, play until crash ...

Process 51234 stopped
* thread #1: EXC_BAD_ACCESS (code=1, address=0x18d)
    frame #0: Tile::squash() at Tile.cpp:643

(lldb) bt
* frame #0: Tile::squash(this=0x12345) at Tile.cpp:643
  frame #1: UnitBase::move(this=0x67890) at UnitBase.cpp:633
  ...

(lldb) frame variable
(Tile *) this = 0x00000012345
(std::list<Uint32>) assignedInfantryList = size=3

(lldb) p assignedInfantryList
(std::list<Uint32>) $0 = size=3 {
  [0] = 123
  [1] = 456
  [2] = 789
}

(lldb) up
frame #1: UnitBase::move(this=0x67890) at UnitBase.cpp:633

(lldb) frame variable
(UnitBase *) this = 0x67890
(Coord) location = (x = 45, y = 23)
(Coord) nextSpot = (x = 46, y = 23)

(lldb) quit
```

---

## Tips for Effective Debugging

### 1. Reproduce Consistently
- Start new game (not loaded save)
- Note: Game type, AI difficulty, map
- Record: What you did before crash
- Try to reproduce 2-3 times

### 2. Capture Full Information
```bash
# Before running, redirect output
lldb -- "./Dune Legacy.app/Contents/MacOS/Dune Legacy" 2>&1 | tee crash.log
(lldb) run
# When crash occurs:
(lldb) bt all
(lldb) frame variable
(lldb) thread list
```

### 3. Compare with Working Code
- Check if crash happens in Release build
- Check if crash happens with different AI settings
- Check if crash happens in single vs multiplayer

### 4. Use Conditional Breakpoints
```lldb
# Break only when objectID is specific value
breakpoint set -n UnitBase::move -c 'objectID == 123'

# Break only in specific house
breakpoint set -f QuantBot.cpp -l 2161 -c 'getHouse()->getHouseID() == 0'
```

---

## Current Debug Build Info

**Location**: `IDE/xCode/build/Debug/Dune Legacy.app`
**Version**: 0.98.6
**Configuration**:
- No optimization (O0)
- Full debug symbols
- No link-time optimization
- No vectorization
- Variables accessible
- Line numbers accurate

**Differences from Release**:
- **Slower** (expected - no optimization)
- **Larger binary** (debug symbols included)
- **Better crash info** (stack traces readable)
- **Variables visible** (can inspect in lldb)

---

## Next Steps After Crash

1. **Capture the full backtrace**: `bt all`
2. **Save the output**: Copy from terminal or use `tee`
3. **Note the variables**: Use `frame variable` at each frame
4. **Check object validity**: Print pointers and their contents
5. **Look for patterns**: Does it always crash in the same place?
6. **Document**: Note what you were doing when it crashed

---

**Status**: ✅ Debug build ready for crash diagnosis
**Commit**: ce5ec22
**Build**: Success

