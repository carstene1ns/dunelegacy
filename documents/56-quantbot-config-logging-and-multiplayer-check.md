# QuantBot Configuration Logging and Multiplayer Verification

**Date**: 2025-01-24  
**Status**: ✅ Complete

## Summary

Added comprehensive logging of QuantBot configuration settings at game start and multiplayer config consistency checking.

## Changes Made

### 1. Configuration Logging (`QuantBotConfig::logSettings()`)

**File**: `src/players/QuantBotConfig.cpp`, `include/players/QuantBotConfig.h`

Added a `logSettings()` method that outputs all QuantBot configuration to the debug log when the game starts:

- **Difficulty Settings**: For each difficulty level (Defend, Easy, Medium, Hard, Brutal):
  - Attack enabled flag
  - Ornithopter attack enabled flag
  - Ornithopter attack threshold
  - Military value multiplier
  - Harvester limit multiplier

- **General Behavior**:
  - Attack timer (milliseconds)
  - Attack threshold percentage
  - Minimum money for production

- **Unit Ratios**: Sample ratios for Easy difficulty showing:
  - Tank, Siege Tank, Launcher, Special, Ornithopter percentages
  - Per-house configuration (Atreides, Harkonnen, Ordos)

**Log Output Format**:
```
==================== QUANTBOT CONFIGURATION ====================
Config file: <path>/config/QuantBot Config.ini

=== DIFFICULTY SETTINGS ===
DEFEND:  Attack=0 OrnAttack=0 OrnThresh=999 MilMult=0.60 HarvMult=3
EASY:    Attack=1 OrnAttack=0 OrnThresh=999 MilMult=1.20 HarvMult=3
MEDIUM:  Attack=1 OrnAttack=0 OrnThresh=999 MilMult=1.80 HarvMult=3
...

=== GENERAL BEHAVIOR ===
AttackTimerMs: 15000
AttackThresholdPercent: 0.30
MinMoneyForProduction: 500

=== UNIT RATIOS (showing Easy difficulty sample) ===
Atreides:  Tank=0.35 Siege=0.30 Launcher=0.10 Special=0.20 Orni=0.05
...
===============================================================
```

**Trigger**: Automatically called when `getQuantBotConfig()` is first invoked (i.e., at game start).

### 2. Configuration Hash for Multiplayer Consistency (`QuantBotConfig::getConfigHash()`)

**File**: `src/players/QuantBotConfig.cpp`, `include/players/QuantBotConfig.h`

Added a `getConfigHash()` method that generates a unique hash representing the entire configuration:

- **Hash includes**:
  - All difficulty settings for all 5 difficulty levels
  - All unit ratios for all houses and difficulties
  - All general behavior settings

- **Purpose**: Allows detection of configuration mismatches between multiplayer players

- **Implementation**: 
  - Concatenates all config values into a string
  - Generates 64-bit hash using `std::hash<std::string>`
  - Returns 16-character hex representation

**Example hash**: `00000a3f4b2c1d5e`

### 3. Multiplayer Configuration Check

**File**: `src/Menu/CustomGamePlayers.cpp`

Added configuration hash logging when multiplayer game starts (before the 5-second countdown):

```cpp
if(pNetworkManager != nullptr) {
    // Multiplayer game - log config hash for verification
    QuantBotConfig& config = getQuantBotConfig();
    std::string configHash = config.getConfigHash();
    SDL_Log("==================== MULTIPLAYER CONFIG CHECK ====================");
    SDL_Log("Starting multiplayer game with QuantBot config hash: %s", configHash.c_str());
    SDL_Log("IMPORTANT: All players must have identical QuantBot Config.ini files!");
    SDL_Log("Config file location: %s", getQuantBotConfigFilepath().c_str());
    SDL_Log("If AI behavior differs between players, verify config files match.");
    SDL_Log("================================================================");
    
    // ... start game
}
```

**Multiplayer Log Output**:
```
==================== MULTIPLAYER CONFIG CHECK ====================
Starting multiplayer game with QuantBot config hash: 00000a3f4b2c1d5e
IMPORTANT: All players must have identical QuantBot Config.ini files!
Config file location: <path>/config/QuantBot Config.ini
If AI behavior differs between players, verify config files match.
================================================================
```

**Current Behavior**: 
- Logs the configuration hash for each player
- Players can manually compare hashes in their logs to verify configs match
- Game proceeds regardless of hash (no automatic enforcement)

**Future Enhancement** (TODO in code):
- Add network protocol to automatically exchange and verify config hashes
- Show error dialog if configs don't match before game starts
- Optionally allow override if all players agree

## Files Modified

1. **`include/players/QuantBotConfig.h`**
   - Added `void logSettings() const;`
   - Added `std::string getConfigHash() const;`

2. **`src/players/QuantBotConfig.cpp`**
   - Implemented `logSettings()` with comprehensive output
   - Implemented `getConfigHash()` using string concatenation and hashing
   - Modified `getQuantBotConfig()` to call `logSettings()` on first load

3. **`src/Menu/CustomGamePlayers.cpp`**
   - Added `#include <players/QuantBotConfig.h>`
   - Added config hash logging in `onNext()` for multiplayer games

## Testing Checklist

- [x] Compile successfully on Windows (Visual Studio)
- [ ] Test single-player game start - verify config logged
- [ ] Test multiplayer game start - verify hash logged
- [ ] Modify config file between two instances - verify different hashes
- [ ] Verify log output is readable and helpful
- [ ] Test on Linux
- [ ] Test on macOS

## Benefits

1. **Debugging**: Developers and users can easily see what AI settings are active
2. **Transparency**: Players know exactly what AI behavior to expect
3. **Multiplayer Fairness**: Players can verify they all have the same AI settings
4. **Support**: When reporting issues, users can include config logs
5. **Modding**: Modders can verify their config changes are loaded correctly

## Notes

- Configuration is logged once per game session (when first loaded)
- Multiplayer hash check is informational only (no enforcement yet)
- Full network protocol for automatic hash verification is marked as TODO
- Hash collisions are theoretically possible but extremely unlikely in practice
- Config file path is shown in logs to help users locate the file

## See Also

- `documents/53-quantbot-external-config.md` - QuantBot external configuration system
- `documents/55-config-consolidation.md` - Config file installation and locations
- `config/QuantBot Config.ini` - Template configuration file with documentation

