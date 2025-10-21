# Version Bump to 0.98.6

## Date
October 21, 2025

## Actions Taken

### 1. Created New Branch
```bash
git checkout -b release-0.98.6
```

**Why**: Clean separation from previous work. We reverted all changes on release-0.98.4, now starting fresh development on release-0.98.6.

---

### 2. Updated Version Numbers

#### include/config.h
```c
#define VERSION "0.98.4"  →  "0.98.6"
```

#### CMakeLists.txt
```cmake
project(DuneLegacy VERSION 0.98.4 ...)  →  VERSION 0.98.6
```

**Why**: Indicate this is a new development version after the revert.

---

### 3. Cleaned Build Artifacts
```bash
cd IDE/xCode
xcodebuild clean -project "Dune Legacy.xcodeproj" -scheme "Dune Legacy" -configuration GameDebug
```

**Why**: Previous GameDebug binary still showed 0.99.0 because Xcode cached build artifacts. Clean removes all cached binaries.

**Result**: ✅ CLEAN SUCCEEDED

---

### 4. Rebuilt GameDebug
```bash
xcodebuild -project "Dune Legacy.xcodeproj" -scheme "Dune Legacy" -configuration GameDebug build
```

**Result**: ✅ BUILD SUCCEEDED

---

### 5. Committed Changes
```bash
git add include/config.h CMakeLists.txt
git commit -m "Bump version to 0.98.6"
```

**Commit**: 1949ed0

---

## Current State

### Branch
✅ **release-0.98.6** (new)

### Version
✅ **0.98.6** (updated in source and binary)

### Build Status
✅ GameDebug: Clean and rebuilt
✅ Release: Will show 0.98.6 on next build

### Git Status
- Clean working directory
- Only untracked files: `.vscode/`, `0.98.5/`, `documents/`, `include/SpatialGridHandle.h`

---

## Why This Was Necessary

### The Problem
After reverting from the complex changes:
1. Source code showed 0.98.4 (correct after revert)
2. Release binary showed 0.98.4 (correct - was rebuilt)
3. **GameDebug binary still showed 0.99.0** (STALE - not rebuilt)

### Why GameDebug Was Stale
- When we reverted, we only rebuilt **Release** configuration
- **GameDebug** binary was last built when `config.h` said 0.99.0
- Xcode didn't see a reason to rebuild GameDebug (no source changes since its last build)
- Version is compiled into the binary at build time

### The Solution
1. Clean the build (removes all cached binaries)
2. Rebuild GameDebug with new version
3. Now GameDebug will show 0.98.6 ✅

---

## Important Workflow (SAVED TO MEMORY)

**After any version change or major revert:**
1. Create new branch (optional but recommended)
2. Update version in `include/config.h` and `CMakeLists.txt`
3. **Clean the build** (critical!)
4. Rebuild the configuration you're testing
5. Commit the version change

**DO NOT skip the clean step!** Xcode caches binaries and you'll end up with mismatched versions.

---

## Version History

- **0.98.4**: Stable release (remote branch)
- **0.99.0**: Development version (broken, reverted)
- **0.98.6**: New development version (current, clean slate)

---

## Next Steps

### Immediate
✅ Version correctly shows 0.98.6 in all builds
✅ Ready for testing or development

### If Developing
- Make changes incrementally
- Test after each change
- Refer to `documents/18-changes-analysis.md` for what was attempted before
- Refer to `documents/19-problems-found.md` for lessons learned

### If Testing
- Run the game
- Verify version shows 0.98.6
- Confirm no bugs from previous development

---

**Status**: ✅ Complete
**Branch**: release-0.98.6
**Version**: 0.98.6
**Build**: Clean and working

