# Configuration Logging to Debug Log

**Date**: October 24, 2024  
**Version**: 0.98.6.2  
**Commit**: 0a318f7

## Summary

Added comprehensive logging of all configuration files to the debug log when they are loaded during game initialization. This provides immediate visibility into active configuration values for troubleshooting and verification.

## Problem

Previously, config files were loaded silently with no visibility into:
- What values were actually being used
- Whether custom configs were loading correctly
- What differences existed between houses in ObjectData
- Whether multiplayer configs matched between players

This made troubleshooting configuration issues difficult and time-consuming.

## Solution

### 1. ObjectData.ini Logging

Added `logSettings()` method to the `ObjectData` class that logs:
- All key units and structures (32 items) with their properties
- House-specific differences from Atreides baseline
- Only logs significant differences to keep output manageable

**Example Output:**
```
==================== OBJECTDATA CONFIGURATION ====================
Loaded from: config/ObjectData.ini

=== KEY UNITS AND STRUCTURES (Atreides) ===
Soldier: HP=50 Price=60 Dmg=10 Rng=3 Build=30 ViewRng=2
Trooper: HP=50 Price=75 Dmg=20 Rng=5 Build=45 ViewRng=3
Harvester: HP=300 Price=400 Dmg=0 Rng=0 Build=120 ViewRng=6
...

=== HOUSE-SPECIFIC DIFFERENCES ===
Tank differences:
  Harkonnen: HP=180 Price=120 Dmg=35 Build=90
  Ordos: HP=120 Price=100 Dmg=30 Build=75
...
===============================================================
```

### 2. QuantBot Config.ini Logging

Already implemented in previous work (commit from document 56). Logs:
- Difficulty settings (attack flags, limits)
- Unit ratios for all houses and difficulties
- General behavior parameters
- Configuration hash for multiplayer verification

**Example Output:**
```
==================== QUANTBOT CONFIGURATION ====================
Config file: <game_dir>/config/QuantBot Config.ini

=== DIFFICULTY SETTINGS ===
DEFEND:  Attack=0 OrnAttack=0 OrnThresh=999 MilMult=1.80 HarvMult=2
EASY:    Attack=1 OrnAttack=0 OrnThresh=999 MilMult=1.00 HarvMult=1
MEDIUM:  Attack=1 OrnAttack=0 OrnThresh=999 MilMult=1.50 HarvMult=2
HARD:    Attack=1 OrnAttack=1 OrnThresh=4 MilMult=2.00 HarvMult=2
BRUTAL:  Attack=1 OrnAttack=1 OrnThresh=4 MilMult=3.00 HarvMult=3

=== GENERAL BEHAVIOR ===
AttackTimerMs: 15000
AttackThresholdPercent: 0.30
MinMoneyForProduction: 500

=== UNIT RATIOS (showing Easy difficulty sample) ===
Atreides:  Tank=0.05 Siege=0.00 Launcher=0.25 Special=0.65 Orni=0.05
Harkonnen: Tank=0.10 Siege=0.10 Launcher=0.70 Special=0.10 Orni=0.00
...
===============================================================
```

## Implementation Details

### ObjectData Logging

**File**: `src/ObjectData.cpp`

Added `logSettings()` method that:
1. Logs header with source file path
2. Iterates through key units/structures (32 items):
   - Units: Soldier, Trooper, Harvester, MCV, Trike, RaiderTrike, Quad, Tank, SiegeTank, Launcher, Devastator, SonicTank, Deviator, Ornithopter, Carryall
   - Structures: Slab, Wall, ConYard, WindTrap, Refinery, Barracks, WOR, Light/Heavy/HighTech Factories, IX, Palace, RepairYard, StarPort, Silo, Gun/Rocket Turrets
3. Logs Atreides values as baseline
4. Shows only house-specific differences (not all houses if identical)
5. Properties logged: HitPoints, Price, WeaponDamage, WeaponRange, BuildTime, ViewRange

**Called From**: `Game::initGame()` immediately after loading ObjectData.ini

```cpp
objectData.loadFromINIFile("config/ObjectData.ini");
objectData.logSettings();
```

### QuantBot Config Logging

**File**: `src/players/QuantBotConfig.cpp`

Existing `logSettings()` method called from `getQuantBotConfig()`:
```cpp
g_quantBotConfig->load(configPath);
g_quantBotConfig->logSettings();
```

## When Logging Occurs

Both config logs are written to the debug log when:
- A new game is started (Campaign, Skirmish, Custom, Multiplayer)
- Immediately after the config file is loaded
- Before any units/structures are created
- Before AI players are initialized

**Note**: Logs are NOT generated when:
- Loading a saved game (uses saved config state)
- Loading a replay (uses replay's config state)

## Benefits

### For Users
- ✅ Verify custom ObjectData.ini changes are working
- ✅ See if config files loaded from correct location
- ✅ Understand AI difficulty differences
- ✅ Troubleshoot balance issues

### For Modders
- ✅ Immediate feedback on ObjectData.ini modifications
- ✅ Quick comparison of house-specific differences
- ✅ Verify QuantBot behavior changes
- ✅ Debug unit/structure property issues

### For Developers
- ✅ Troubleshoot bug reports with actual values used
- ✅ Verify config system working correctly
- ✅ Track down balance issues
- ✅ Ensure multiplayer config consistency

### For Multiplayer
- ✅ Config hash shows if players have matching settings
- ✅ Can compare debug logs to verify identical configs
- ✅ Helps enforce fair play in competitive games

## Log File Location

Debug logs are written to:
- **Windows**: `C:\Users\<username>\AppData\Roaming\dunelegacy\dunelegacy.log`
- **Linux**: `~/.config/dunelegacy/dunelegacy.log`
- **macOS**: `~/Library/Application Support/dunelegacy/dunelegacy.log`

## Output Size

The logging is designed to be comprehensive but not overwhelming:
- **ObjectData**: ~50-100 lines (32 items + differences)
- **QuantBot Config**: ~40-60 lines (settings + ratios)
- **Total**: ~100-150 lines per game start

This is reasonable for debug logging and doesn't impact performance.

## Future Enhancements

Potential improvements:
- Add option to log ALL units/structures (not just key 32)
- Log Dune Legacy.ini settings on game start
- Add config hash to ObjectData for multiplayer verification
- Log when config values differ from defaults
- Add config export feature for sharing/backup

## Testing

Tested on Windows:
- ✅ ObjectData.ini logs all key units/structures
- ✅ House differences correctly identified
- ✅ QuantBot config logs all difficulty settings
- ✅ Logs written to dunelegacy.log
- ✅ No performance impact
- ✅ Logs clear and readable

## Related Files

- `include/ObjectData.h` - Added logSettings() declaration
- `src/ObjectData.cpp` - Implemented ObjectData logging
- `src/Game.cpp` - Call objectData.logSettings() after load
- `src/players/QuantBotConfig.cpp` - QuantBot logging (existing)
- `include/players/QuantBotConfig.h` - QuantBot logging declaration (existing)

## Related Documents

- `documents/53-quantbot-external-config.md` - QuantBot config system
- `documents/56-quantbot-config-logging-and-multiplayer-check.md` - QuantBot logging implementation
- `documents/57-config-file-locations.md` - Config file locations

## Notes

- ObjectData logging focuses on key items to keep output manageable
- Only logs house differences (not all 6 houses for every item)
- QuantBot config was already logging (this document just confirms it)
- Both logs are automatically generated on every game start
- Logs can be shared for troubleshooting without revealing sensitive data

