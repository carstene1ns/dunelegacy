# Detailed Revert Plan

## Goal
Document ALL business logic changes with WHY each was made, then revert to remote branch after review.

## Step 1: Create Detailed Analysis Document

Create `documents/18-changes-analysis.md` with sections for EACH changed file.

For each file, document:
- **WHAT** changed (code/logic)
- **WHY** it was changed (reason/goal)
- **CATEGORY**: Logging, Bug Fix, Game Logic, Performance, Infrastructure

### Files to Analyze (32 total):

#### Major Changes (High Impact)
1. **src/players/QuantBot.cpp** (~1,844 lines) - CRITICAL
2. **src/Game.cpp** (~437 lines) - CRITICAL
3. **src/units/UnitBase.cpp** (~111 lines) - CRITICAL
4. **src/AStarSearch.cpp** (~66 lines)

#### Medium Impact
5. **src/structures/TurretBase.cpp** (~35 lines)
6. **src/misc/IFileStream.cpp** (~33 lines)
7. **src/StarPort.cpp** (~23 lines)
8. **include/Game.h** (~48 lines)
9. **include/units/UnitBase.h** (~14 lines)
10. **include/players/QuantBot.h** (~12 lines)
11. **src/ObjectBase.cpp** (~11 lines)
12. **src/Menu/CustomGamePlayers.cpp** (~10 lines)

#### Low Impact (Infrastructure/Config)
13. **include/AStarSearch.h** (~7 lines)
14. **include/misc/IMemoryStream.h** (~7 lines)
15. **include/Network/ENetPacketIStream.h** (~7 lines)
16. **src/main.cpp** (~6 lines)
17. **src/SpatialGrid.cpp** (~4 lines)
18. **src/units/Frigate.cpp** (~4 lines)
19. **CMakeLists.txt** (~4 lines)
20. **include/config.h.in** (~4 lines)
21. **nsis/dunelegacy_mingw.nsi** (~4 lines)
22. **include/misc/IFileStream.h** (~3 lines)
23. **src/structures/RocketTurret.cpp** (~3 lines)
24. **IDE/xCode/MacFunctions.m** (~3 lines)
25. **nsis/dunelegacy.nsi** (~3 lines)
26. **include/Definitions.h** (~4 lines)
27. **include/misc/InputStream.h** (~2 lines)
28. **include/structures/TurretBase.h** (~2 lines)
29. **include/config.h** (~2 lines)
30. **src/GameInitSettings.cpp** (~2 lines)
31. **src/Tile.cpp** - Tile::squash() fix
32. **IDE/xCode/** - Binary/config files

## Step 2: Analysis Template for Each File

For each file, I will document:

```markdown
### filename.cpp

#### Change 1: [Brief description]
**Lines**: X-Y
**What**: [Describe the code change]
**Why**: [Why was this done? What problem did it try to solve?]
**Category**: [Logging / Bug Fix / Game Logic / Performance / Infrastructure]
**Impact**: [What effect did this have?]
**Problems**: [Did this cause issues?]

#### Change 2: [Brief description]
...
```

## Step 3: Create Problems Document

Create `documents/19-problems-found.md`:
- Visual symptoms (units through buildings, off-map)
- Failed fixes that masked root causes
- Root cause analysis
- Lessons learned

## Step 4: Review Phase (WAIT FOR USER)
- User reviews documents 18 and 19
- User approves or requests changes
- **DO NOT REVERT YET**

## Step 5: Revert (Only After Approval)
```bash
git reset --hard origin/release-0.98.4
```

## Step 6: Verify Clean State
- Run build
- Test game
- Confirm working

## Execution Plan

1. Start with CRITICAL files (QuantBot, Game, UnitBase)
2. Then medium impact files
3. Then low impact/infrastructure
4. Create problems document
5. **STOP and wait for review**
6. Only revert after approval

## Time Estimate
- Document 18 (detailed analysis): 2-3 hours
- Document 19 (problems): 30 minutes
- Review phase: (user decides)
- Revert & verify: 5 minutes
