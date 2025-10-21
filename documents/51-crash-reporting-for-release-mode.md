# Crash Reporting for Release Mode

**Date**: 2025-10-21  
**Branch**: `release-0.98.6`  
**Status**: 📋 Proposal

## Problem 🚨

**User Report:** "Game just crashed. Is there anyway of telling why if it was run in release mode? Having a game that just crashes to desktop isn't great."

### Current Situation

**What Works:**
- ✅ Global `try/catch` in `main()` catches C++ exceptions
- ✅ Shows error message box with exception details
- ✅ Logs go to file via SDL (`stderr` → log file)

**What Doesn't Work:**
- ❌ **No crash dumps** for native crashes (SIGSEGV, SIGABRT, etc.)
- ❌ **No stack traces** in release mode
- ❌ **No crash metadata** (game state, player count, map, etc.)
- ❌ **Silent desktop crashes** leave no trace
- ❌ **Debug symbols not shipped** with release builds

### Types of Crashes

1. **C++ Exceptions** (Handled):
   ```cpp
   throw std::runtime_error("error");  // ✅ Caught, shows message
   ```

2. **Segmentation Faults** (Not Handled):
   ```cpp
   Unit* unit = nullptr;
   unit->update();  // ❌ SIGSEGV → Silent crash to desktop
   ```

3. **Assertion Failures** (Not Handled):
   ```cpp
   assert(unit != nullptr);  // ❌ SIGABRT → Silent crash
   ```

4. **Pure Virtual Calls** (Not Handled):
   ```cpp
   VirtualBase* obj = ...;
   obj->pureVirtual();  // ❌ SIGILL → Silent crash
   ```

5. **Stack Overflow** (Not Handled):
   ```cpp
   void recursive() { recursive(); }  // ❌ SIGSEGV → Silent crash
   ```

---

## Solution Options 🛠️

### 🟢 **Option 1: Signal Handlers + Basic Logging** (RECOMMENDED)

**Effort:** 2-3 days  
**Complexity:** ⭐⭐ Moderate  
**Platform Support:** ✅ macOS, Linux, Windows

#### Implementation

**1. Install Signal Handlers**

Create `src/CrashHandler.cpp`:

```cpp
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <execinfo.h>  // For backtrace (POSIX)
#include <SDL.h>

static FILE* crashLogFile = nullptr;
static const char* crashLogPath = nullptr;

void writeCrashLog(const char* format, ...) {
    if(!crashLogFile) return;
    
    va_list args;
    va_start(args, format);
    vfprintf(crashLogFile, format, args);
    va_end(args);
    fflush(crashLogFile);
}

void signalHandler(int signal) {
    // Re-raise original signal after cleanup
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signal, &sa, nullptr);
    
    writeCrashLog("\n========================================\n");
    writeCrashLog("CRASH DETECTED\n");
    writeCrashLog("========================================\n");
    writeCrashLog("Signal: %d (%s)\n", signal, strsignal(signal));
    writeCrashLog("Time: %s\n", getTimeStamp());
    writeCrashLog("Version: %s\n", VERSION);
    writeCrashLog("Platform: %s\n", SDL_GetPlatform());
    writeCrashLog("\n");
    
    // Get stack trace (POSIX only)
    #ifndef _WIN32
    void* callstack[128];
    int frames = backtrace(callstack, 128);
    char** symbols = backtrace_symbols(callstack, frames);
    
    writeCrashLog("Stack Trace (%d frames):\n", frames);
    for(int i = 0; i < frames; i++) {
        writeCrashLog("  [%d] %s\n", i, symbols[i]);
    }
    free(symbols);
    #else
    writeCrashLog("Stack trace not available on Windows\n");
    #endif
    
    writeCrashLog("\n========================================\n");
    writeCrashLog("Please report this crash with the log file:\n");
    writeCrashLog("%s\n", crashLogPath);
    writeCrashLog("========================================\n\n");
    
    if(crashLogFile) {
        fclose(crashLogFile);
        crashLogFile = nullptr;
    }
    
    // Show message to user
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "Dune Legacy Crashed",
        "The game has crashed unexpectedly.\n\n"
        "A crash report has been saved to:\n"
        "Dune Legacy.log\n\n"
        "Please report this on GitHub or the forums.",
        nullptr
    );
    
    // Re-raise signal to trigger default behavior (core dump, etc.)
    raise(signal);
}

void installCrashHandlers(const char* logPath) {
    crashLogPath = logPath;
    
    // Open log file for crash reporting
    crashLogFile = fopen(logPath, "a");
    if(!crashLogFile) {
        fprintf(stderr, "Warning: Could not open crash log file: %s\n", logPath);
        return;
    }
    
    // Install handlers for common crash signals
    signal(SIGSEGV, signalHandler);  // Segmentation fault
    signal(SIGABRT, signalHandler);  // Abort
    signal(SIGFPE,  signalHandler);  // Floating point exception
    signal(SIGILL,  signalHandler);  // Illegal instruction
    signal(SIGBUS,  signalHandler);  // Bus error (macOS)
    
    #ifdef _WIN32
    signal(SIGTERM, signalHandler);  // Termination request
    #endif
    
    SDL_Log("Crash handlers installed");
}
```

**2. Call From main()**

```cpp
int main(int argc, char *argv[]) {
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
    
    // NEW: Install crash handlers
    std::string logPath = getConfigFilePath() + "/Dune Legacy.log";
    installCrashHandlers(logPath.c_str());
    
    try {
        // ... existing code ...
```

#### What You Get

**On Crash:**
```
========================================
CRASH DETECTED
========================================
Signal: 11 (Segmentation fault: 11)
Time: 2025-10-21 15:32:45
Version: 0.98.6
Platform: macOS

Stack Trace (15 frames):
  [0] 0x102a3c000 signalHandler + 45
  [1] 0x7ff800001234 _sigtramp + 29
  [2] 0x102b45678 UnitBase::update() + 234
  [3] 0x102b45abc Game::processObjects() + 156
  [4] 0x102b45def Game::updateGameState() + 89
  [5] 0x102b46012 Game::runMainLoop() + 345
  ...

========================================
Please report this crash with the log file:
/Users/.../Dune Legacy.log
========================================
```

**Benefits:**
- ✅ Captures all native crashes
- ✅ Shows stack trace (function names, not full symbols)
- ✅ Logs to existing log file
- ✅ Shows user-friendly message
- ✅ Works on all platforms

**Limitations:**
- ⚠️ Stack traces show mangled names, not source lines
- ⚠️ No local variables visible
- ⚠️ Limited Windows support (no backtrace)

---

### 🟡 **Option 2: Crash Handler + Symbolication** (BETTER)

**Effort:** 1 week  
**Complexity:** ⭐⭐⭐ Moderate-High  
**Platform Support:** ✅ macOS, Linux, 🟡 Windows (partial)

#### Additional Features

**1. Ship Debug Symbols Separately**

- Build release with `-g` (debug symbols)
- Strip symbols from binary: `strip DuneLegacy.app`
- Keep `.dSYM` bundle (macOS) or `.pdb` (Windows)
- Ship symbols in separate download or server

**2. Symbolicate Stack Traces Post-Crash**

macOS:
```bash
atos -o DuneLegacy.dSYM -l 0x102a3c000 0x102b45678
# Output: UnitBase::update() at UnitBase.cpp:234
```

Linux:
```bash
addr2line -e dunelegacy -f -C 0x102b45678
# Output: UnitBase::update() UnitBase.cpp:234
```

**3. Automatic Crash Reporting (Optional)**

- Upload crash logs to server
- Aggregate crashes by signature
- Track which crashes are most common

#### Benefits Over Option 1:
- ✅ Full source file + line numbers
- ✅ Readable function names
- ✅ Can analyze crashes offline
- ✅ Aggregate crash statistics

---

### 🟡 **Option 3: Platform-Specific Crash Reporters** (PROFESSIONAL)

**Effort:** 2-3 weeks  
**Complexity:** ⭐⭐⭐⭐ High  
**Platform Support:** Per-platform

#### macOS: Use System CrashReporter

macOS automatically generates crash reports:
- Location: `~/Library/Logs/DiagnosticReports/`
- Format: `.crash` files with full stack traces
- Includes: CPU registers, thread states, libraries loaded

**Integration:**
```cpp
// No code needed! macOS does it automatically.
// Just tell users where to find crash logs.
```

**Benefits:**
- ✅ Zero code required
- ✅ System-generated, reliable
- ✅ Includes full crash context
- ✅ Works even if app can't catch it

**User Instructions:**
```
If the game crashes:
1. Open Finder
2. Go to ~/Library/Logs/DiagnosticReports/
3. Find "Dune Legacy_YYYY-MM-DD-HHMMSS.crash"
4. Attach to bug report
```

#### Windows: Windows Error Reporting (WER)

Use `SetUnhandledExceptionFilter()`:

```cpp
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>

LONG WINAPI exceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    // Generate minidump
    HANDLE hFile = CreateFile("DuneLegacy_crash.dmp", ...);
    
    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = exceptionInfo;
    mdei.ClientPointers = FALSE;
    
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                     hFile, MiniDumpNormal, &mdei, NULL, NULL);
    
    CloseHandle(hFile);
    
    return EXCEPTION_EXECUTE_HANDLER;
}

void installCrashHandlers() {
    SetUnhandledExceptionFilter(exceptionHandler);
}
#endif
```

**Benefits:**
- ✅ Full minidumps with all context
- ✅ Can debug with Visual Studio
- ✅ Industry standard

#### Linux: Core Dumps

Enable core dumps:
```bash
ulimit -c unlimited  # Allow core dumps
```

**Benefits:**
- ✅ Full crash context
- ✅ Can analyze with `gdb`
- ✅ No code changes needed

---

### 🔴 **Option 4: Third-Party Crash Reporting** (OVERKILL)

**Effort:** 2-4 weeks  
**Complexity:** ⭐⭐⭐⭐⭐ Very High  
**Cost:** $$ Monthly fees

#### Services:
- **Sentry** - https://sentry.io
- **BugSnag** - https://www.bugsnag.com
- **Crashlytics** (Firebase)

#### Features:
- Automatic crash collection
- Web dashboard
- Aggregation by signature
- Release tracking
- User impact analysis
- Email notifications

#### Why NOT Recommended:
- ❌ Monthly cost ($29-99+/month)
- ❌ Privacy concerns (sends data to 3rd party)
- ❌ Overkill for open-source game
- ❌ Adds external dependency

---

## Recommended Implementation Plan 📋

### Phase 1: Basic Crash Logging (Week 1)

**Goal:** Never have silent crashes again

1. ✅ Implement Option 1 (Signal Handlers + Basic Logging)
2. ✅ Test on macOS, Linux, Windows
3. ✅ Update documentation with crash reporting instructions

**Files to Create/Modify:**
- `src/CrashHandler.cpp` (new)
- `include/CrashHandler.h` (new)
- `src/main.cpp` (add installCrashHandlers call)
- `CMakeLists.txt` (add CrashHandler.cpp)
- `IDE/xCode/Dune Legacy.xcodeproj/project.pbxproj` (add files)

### Phase 2: Enhanced Crash Context (Week 2)

**Goal:** Make crashes easier to debug

Add game state to crash logs:
```cpp
void writeCrashGameState() {
    if(!currentGame) return;
    
    writeCrashLog("\nGame State:\n");
    writeCrashLog("  Game Mode: %s\n", getModeString());
    writeCrashLog("  Game Cycle: %d\n", currentGame->getGameCycleCount());
    writeCrashLog("  Players: %d\n", getPlayerCount());
    writeCrashLog("  Map: %s\n", getMapName());
    writeCrashLog("  Units: %d\n", getUnitCount());
    writeCrashLog("  Multiplayer: %s\n", isMultiplayer() ? "Yes" : "No");
    writeCrashLog("\n");
}
```

### Phase 3: Platform-Specific Improvements (Week 3-4)

**macOS:**
- Instruct users on finding system crash reports
- Add README with crash reporting instructions

**Windows:**
- Implement minidump generation
- Ship debug symbols separately

**Linux:**
- Enable core dumps by default
- Add instructions for analyzing with `gdb`

---

## Testing Plan 🧪

### Trigger Test Crashes

Add debug command to intentionally crash:

```cpp
// For testing only - remove in release!
void testCrash(int type) {
    switch(type) {
        case 1: {
            // Segmentation fault
            int* ptr = nullptr;
            *ptr = 42;
        } break;
        
        case 2: {
            // Abort
            abort();
        } break;
        
        case 3: {
            // Division by zero
            int x = 1 / 0;
        } break;
        
        case 4: {
            // Stack overflow
            testCrash(4);
        } break;
        
        case 5: {
            // Uncaught exception
            throw std::runtime_error("Test exception");
        } break;
    }
}
```

### Verification Checklist

For each crash type:
- [ ] Crash is caught and logged
- [ ] Stack trace is written to log file
- [ ] User sees error message
- [ ] Log file includes game state
- [ ] Crash report is readable

---

## User Impact 📊

### Before (Current):
```
[Game crashes]
User: "WTF? Game just closed!"
Developer: "Can you describe what you were doing?"
User: "I don't remember..."
Developer: 🤷 "Can't reproduce, closing issue"
```

### After (With Crash Reporting):
```
[Game crashes]
User: "Game crashed, here's the log file"
Developer: *reads crash log*
Developer: "Ah, NULL pointer in UnitBase::update() line 234"
Developer: *fixes bug*
Developer: "Fixed in next release, thanks!"
```

---

## Implementation Pseudocode 💻

```cpp
// include/CrashHandler.h
#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

void installCrashHandlers(const char* logPath);
void writeCrashGameState();  // Called from Game class

#endif

// src/CrashHandler.cpp
#include "CrashHandler.h"
#include <csignal>
#include <execinfo.h>
// ... (implementation from Option 1 above)

// src/main.cpp
#include "CrashHandler.h"

int main(int argc, char *argv[]) {
    // Initialize SDL logging first
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
    
    // Install crash handlers BEFORE anything can crash
    std::string logPath = "..."; // Get from config system
    installCrashHandlers(logPath.c_str());
    
    // Rest of main()
    try {
        // ...
    } catch(...) {
        // ...
    }
}

// src/Game.cpp (optional enhancement)
#include "CrashHandler.h"

Game::Game() {
    // Register game instance for crash reporting
    registerGameForCrashReporting(this);
}
```

---

## Risks & Mitigations ⚠️

### Risk 1: Signal Handler Crashes
**Problem:** Signal handler itself could crash  
**Mitigation:** Keep signal handler minimal, no allocations, no complex logic

### Risk 2: Log File Not Writable
**Problem:** No permission to write crash log  
**Mitigation:** Fall back to stderr, show message box with instructions

### Risk 3: Incomplete Stack Traces
**Problem:** Release builds have limited symbols  
**Mitigation:** Phase 2 ships debug symbols separately

### Risk 4: Platform-Specific Issues
**Problem:** Different behavior on Windows vs macOS vs Linux  
**Mitigation:** Test on all platforms, have platform-specific code paths

---

## Decision Required 🎯

**Recommended:** Implement Phase 1 (Signal Handlers + Basic Logging)

**Estimated Effort:** 2-3 days  
**Impact:** HIGH - Never lose crash information again  
**Risk:** LOW - Well-tested approach  

**Next Steps:**
1. ✅ Approve implementation plan
2. ✅ Create CrashHandler.cpp/.h files
3. ✅ Integrate into build systems
4. ✅ Test on all platforms
5. ✅ Document crash reporting for users

---

**Status:** Awaiting approval to implement  
**Priority:** HIGH (user-reported issue)

