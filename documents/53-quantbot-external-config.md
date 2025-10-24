# QuantBot External Configuration System - Implementation Complete

## Summary
Implemented a comprehensive external configuration system for QuantBot AI that allows players to customize AI behavior without recompiling the game. Configuration is stored in `QuantBot Config.ini` in the user's config directory (e.g., `C:\Users\<username>\AppData\Roaming\dunelegacy\`).

### Key Feature: Per-Difficulty Unit Ratios
**Each difficulty level now has its own unit composition ratios for each house.** This allows ornithopter spam to be progressively reduced on lower difficulties:
- **Very Easy**: 0% ornithopters
- **Easy**: 5% ornithopters  
- **Medium**: 10% ornithopters
- **Hard/Brutal**: 15-25% ornithopters (original ratios)

This addresses the community feedback that qBotVeryEasy was too aggressive with ornithopter spam.

## Problem Addressed
User feedback indicated that qBotVeryEasy (Defend mode) was too aggressive with ornithopter spam, wiping out players who expected an easy difficulty. The hardcoded behavior made it impossible for users to adjust AI difficulty without modifying source code and recompiling.

## Solution Implemented

### 1. Created QuantBotConfig System

#### Files Created:
- **`include/players/QuantBotConfig.h`** - Configuration structure definitions
- **`src/players/QuantBotConfig.cpp`** - Load/save implementation using INIFile system

#### Configuration Structure:
```cpp
struct DifficultySettings {
    // Attack behavior
    bool attackEnabled;                     // Can this difficulty attack at all?
    bool ornithopterAttackEnabled;          // Can ornithopters attack?
    int ornithopterAttackThreshold;         // Minimum ornithopters needed to attack
    
    // Military limits
    float militaryValueMultiplier;          // Multiplier for initial military value (Campaign)
    int militaryValueLimitCustomSmallMap;   // Military value limit for small maps (Custom)
    int militaryValueLimitCustomMediumMap;  // Military value limit for medium maps (Custom)
    int militaryValueLimitCustomLargeMap;   // Military value limit for large maps (Custom)
    
    // Harvester limits
    int harvesterLimitPerRefineryMultiplier; // Harvesters per refinery
    int harvesterLimitCustomSmallMap;       // Harvester limit for small maps (Custom)
    int harvesterLimitCustomMediumMap;      // Harvester limit for medium maps (Custom)
    int harvesterLimitCustomLargeMap;       // Harvester limit for large maps (Custom)
};

struct UnitRatios {
    float tank;
    float siegeTank;
    float launcher;
    float special;      // Devastator/Deviator/Sonic Tank
    float ornithopter;
};
```

### 2. Default Configuration Values

#### Defend Difficulty (Very Easy):
- **attackEnabled**: `false` - Never attacks
- **ornithopterAttackEnabled**: `false` - No ornithopter attacks
- **ornithopterAttackThreshold**: `999` - Effectively disabled
- **militaryValueMultiplier**: `1.8`
- **harvesterLimitPerRefineryMultiplier**: `2`
- Custom map limits: Small=4000, Medium=8000, Large=12000

#### Easy Difficulty:
- **attackEnabled**: `true` - Can attack with ground units
- **ornithopterAttackEnabled**: `false` - NO ornithopter attacks ✅
- **ornithopterAttackThreshold**: `999` - Disabled
- **militaryValueMultiplier**: `1.0`
- **harvesterLimitPerRefineryMultiplier**: `1`
- Custom map limits: Small=6000, Medium=12000, Large=18000

#### Medium Difficulty:
- **attackEnabled**: `true` - Can attack with ground units
- **ornithopterAttackEnabled**: `false` - NO ornithopter attacks ✅
- **ornithopterAttackThreshold**: `999` - Disabled
- **militaryValueMultiplier**: `1.5`
- **harvesterLimitPerRefineryMultiplier**: `2`
- Custom map limits: Small=8000, Medium=16000, Large=24000

#### Hard Difficulty:
- **attackEnabled**: `true`
- **ornithopterAttackEnabled**: `true`
- **ornithopterAttackThreshold**: `4` - Needs 4+ ornithopters to attack
- **militaryValueMultiplier**: `2.0`
- **harvesterLimitPerRefineryMultiplier**: `2`
- Custom map limits: Small=10000, Medium=20000, Large=30000

#### Brutal Difficulty:
- **attackEnabled**: `true`
- **ornithopterAttackEnabled**: `true`
- **ornithopterAttackThreshold**: `4`
- **militaryValueMultiplier**: `3.0`
- **harvesterLimitPerRefineryMultiplier**: `3`
- Custom map limits: Small=15000, Medium=30000, Large=45000

### 3. House-Specific Unit Ratios (Per Difficulty)

All unit composition ratios are now configurable **per house AND per difficulty level**. This allows ornithopter ratios to scale with difficulty:

#### Ornithopter Ratio by Difficulty:
- **Very Easy (Defend)**: 0% ornithopters (all houses except Harkonnen who can't build them)
- **Easy**: 5% ornithopters maximum
- **Medium**: 10% ornithopters maximum  
- **Hard**: 10-25% ornithopters (original balanced ratios)
- **Brutal**: Same as Hard

#### Example: Atreides Unit Ratios Across Difficulties
| Difficulty | Tank | Siege | Launcher | Special | Ornithopter |
|------------|------|-------|----------|---------|-------------|
| **Defend** | 5%   | 5%    | 25%      | 65%     | **0%** |
| **Easy**   | 5%   | 0%    | 25%      | 65%     | **5%** |
| **Medium** | 0%   | 0%    | 25%      | 65%     | **10%** |
| **Hard**   | 0%   | 0%    | 20%      | 65%     | **15%** |
| **Brutal** | 0%   | 0%    | 20%      | 65%     | **15%** |

#### Example: Ordos Unit Ratios Across Difficulties
| Difficulty | Tank | Siege | Launcher | Special | Ornithopter |
|------------|------|-------|----------|---------|-------------|
| **Defend** | 35%  | 35%   | 0%       | 30%     | **0%** |
| **Easy**   | 30%  | 30%   | 0%       | 35%     | **5%** |
| **Medium** | 30%  | 30%   | 0%       | 30%     | **10%** |
| **Hard**   | 25%  | 25%   | 0%       | 25%     | **25%** |
| **Brutal** | 25%  | 25%   | 0%       | 25%     | **25%** |

**Note**: Harkonnen cannot build ornithopters (0% across all difficulties)

### 4. General AI Behavior Settings

- **attackTimerMs**: 15000 (15 seconds between attacks)
- **attackThresholdPercent**: 0.30 (attack when military >= 30% of limit)
- **minMoneyForProduction**: 500 (minimum money to produce units)

## Code Changes

### Modified Files:

#### `src/players/QuantBot.cpp`:
1. Added `#include <players/QuantBotConfig.h>`
2. Modified `init()` to load config on startup
3. Replaced hardcoded difficulty settings (lines 291-425) with config values
4. Replaced hardcoded unit ratios (lines 1105-1157) with config values
5. Modified `attack()` function (lines 1810-1867) to use config flags:
   - Check `attackEnabled` before any attacks
   - Check `ornithopterAttackEnabled` and `ornithopterAttackThreshold` for ornithopter attacks
   - Use `attackThresholdPercent` for military strength check
   - Use `attackTimerMs` for timer reset

#### `src/CMakeLists.txt`:
- Added `players/QuantBotConfig.cpp` to build sources

#### `src/Makefile.am`:
- Added `players/QuantBotConfig.cpp` to build sources

## How It Works

### 1. Automatic Config File Creation
When the game starts, if `QuantBot Config.ini` doesn't exist in the user's config directory, it's automatically created with all default values.

### 2. User Customization
Players can edit `QuantBot Config.ini` with any text editor to customize:
- Which difficulties can attack
- Ornithopter behavior per difficulty
- Military and harvester limits
- Unit composition ratios per house
- General attack timings

### 3. Runtime Loading
Config is loaded once on game startup via `getQuantBotConfig()` singleton. All QuantBot instances reference this shared configuration.

## Benefits

### For Players:
1. **Easy balancing** - Edit text file to adjust AI difficulty
2. **No recompilation** - Changes take effect on next game start
3. **House customization** - Adjust unit ratios per faction
4. **Preserved across updates** - Config file separate from game code

### For Developers:
1. **Easy tuning** - Test balance changes without code edits
2. **Community mods** - Players can share config files
3. **Debugging** - All behavior explicitly logged with config values

## Example Use Cases

### Make qBotEasy Even Easier:
```ini
[Difficulty Settings]
Easy_OrnithopterAttackThreshold=12
Easy_MilitaryValueLimitMediumMap=8000
```

### Disable Ornithopters on All Difficulties:
```ini
[Difficulty Settings]
Defend_OrnithopterAttackEnabled=0
Easy_OrnithopterAttackEnabled=0
Medium_OrnithopterAttackEnabled=0
Hard_OrnithopterAttackEnabled=0
Brutal_OrnithopterAttackEnabled=0
```

### Make Atreides Build More Tanks on Hard Difficulty:
```ini
[Unit Ratios Hard]
Atreides_Tank=0.40
Atreides_Launcher=0.30
Atreides_Special=0.20
Atreides_Ornithopter=0.10
```

### Completely Disable Ornithopters for Ordos on All Difficulties:
```ini
[Unit Ratios Defend]
Ordos_Ornithopter=0.00

[Unit Ratios Easy]
Ordos_Ornithopter=0.00

[Unit Ratios Medium]
Ordos_Ornithopter=0.00

[Unit Ratios Hard]
Ordos_Ornithopter=0.00

[Unit Ratios Brutal]
Ordos_Ornithopter=0.00
```

## Testing Recommendations

1. **Test Very Easy (Defend)**: Should NEVER attack
2. **Test Easy**: Should need 8+ ornithopters before attacking
3. **Test config file creation**: Delete config and verify it's recreated with defaults
4. **Test config modifications**: Edit values and verify they're applied in-game
5. **Test all houses**: Verify unit ratios work correctly per faction
6. **Test campaign vs custom**: Verify different limits for each mode

## Future Enhancements

Possible additions to config system:
1. Per-house difficulty settings
2. Tech progression speeds
3. Build priorities and timings
4. Defensive behavior parameters
5. Harvester management settings
6. Retreat/repair thresholds

## Files Modified/Created

### Created:
- `include/players/QuantBotConfig.h`
- `src/players/QuantBotConfig.cpp`
- `documents/53-quantbot-external-config.md` (this file)

### Modified:
- `src/players/QuantBot.cpp`
- `src/CMakeLists.txt`
- `src/Makefile.am`

## Final Configuration Summary

### Ornithopter Attack Behavior by Difficulty:

| Difficulty | Ornithopters Build? | Ornithopters Attack? | Ground Attack? |
|------------|-------------------|---------------------|----------------|
| **Very Easy (Defend)** | ❌ No (0% ratio) | ❌ Never | ❌ Never |
| **Easy** | ✅ Yes (5% ratio) | ❌ Never | ✅ Yes |
| **Medium** | ✅ Yes (10% ratio) | ❌ Never | ✅ Yes |
| **Hard** | ✅ Yes (15-25% ratio) | ✅ Yes (threshold: 4+) | ✅ Yes |
| **Brutal** | ✅ Yes (15-25% ratio) | ✅ Yes (threshold: 4+) | ✅ Yes |

### Result:
- **Lower difficulties (Very Easy, Easy, Medium)**: Ornithopters will be built in small numbers but **will NOT attack players** - they'll only defend
- **Higher difficulties (Hard, Brutal)**: Full ornithopter aggression as originally designed
- **All settings customizable** via `QuantBot Config.ini`

## Compilation Status
✅ No linter errors
✅ Build system updated
✅ Ready for testing

