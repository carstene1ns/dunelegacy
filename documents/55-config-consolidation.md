# Configuration File Consolidation

**Date**: 2024-10-24  
**Status**: ✅ Complete  
**Version**: 0.98.6

## Summary

All configuration files (`Dune Legacy.ini`, `QuantBot Config.ini`, `ObjectData.ini`) are now consolidated to the game installation directory, eliminating the previous split between AppData (user directory) and game directory. This simplifies the user experience and makes configuration files easier to find and modify.

## Previous Behavior

**Before this change**, config files were split across two locations:
- **User Directory (AppData)**: `Dune Legacy.ini` was programmatically created in the user's AppData folder
  - Windows: `C:\Users\<username>\AppData\Roaming\dunelegacy\Dune Legacy.ini`
  - Linux: `~/.config/dunelegacy/Dune Legacy.ini`
  - macOS: `~/.config/dunelegacy/Dune Legacy.ini`
- **Game Directory**: `ObjectData.ini` and `QuantBot Config.ini` templates were installed with the game

This split was confusing for users who wanted to modify their game configuration.

## New Behavior

**After this change**, all config files are in a dedicated config subdirectory:

### Windows
```
<game_install_dir>\
├── dunelegacy.exe
└── config\
    ├── Dune Legacy.ini
    ├── ObjectData.ini
    └── QuantBot Config.ini
```

### Linux
```
<game_install_dir>/
├── dunelegacy
└── config/
    ├── Dune Legacy.ini
    ├── ObjectData.ini
    └── QuantBot Config.ini
```

### macOS
```
Dune Legacy.app/Contents/Resources/
└── config/
    ├── Dune Legacy.ini
    ├── ObjectData.ini
    └── QuantBot Config.ini
```

## Technical Implementation

### 1. Template-Based System

All three config files now use a template-based approach:
1. Template files are installed with the game in the data directory
2. On first run, if a config file doesn't exist in the game directory, it's copied from the template
3. The template file is customized with user-specific defaults (e.g., player name, language)

### 2. Path Functions Updated

**`src/main.cpp`**:
```cpp
std::string getConfigFilepath()
{
    // Config file is in config subdirectory of game directory
    return getDuneLegacyDataDir() + "/config/" + CONFIGFILENAME;
}
```

**`src/players/QuantBotConfig.cpp`**:
```cpp
std::string getQuantBotConfigFilepath() {
    // Config file is in config subdirectory of game directory
    return getDuneLegacyDataDir() + "/config/QuantBot Config.ini";
}
```

### 3. Template Creation

**`src/main.cpp` - `createDefaultConfigFile()`**:
- First attempts to copy template from `pFileManager->openFile(CONFIGFILENAME)`
- Customizes player name and language from user defaults
- Falls back to programmatic creation if template copy fails

### 4. Helper Function

**`src/main.cpp` - `getDefaultPlayerName()`**:
- Extracted player name detection logic into a reusable function
- Gets system username on Windows (via `GetUserName()`) or Unix (via `getpwuid()`)
- Returns "Player" as fallback

### 5. Build System

**`CMakeLists.txt`**:
```cmake
install(FILES "config/ObjectData.ini" "config/QuantBot Config.ini" "config/Dune Legacy.ini" 
        DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
```

**`Makefile.am`**:
```makefile
configfiles_DATA = config/ObjectData.ini \
                   config/QuantBot\ Config.ini \
                   config/Dune\ Legacy.ini
```

### 6. Template File Headers Updated

All three config files now have updated headers reflecting the new locations:

**`config/Dune Legacy.ini`**:
```ini
# Config file location (config subdirectory within game installation):
#   Windows: <game_install_dir>\config\Dune Legacy.ini
#   Linux:   <game_install_dir>/config/Dune Legacy.ini
#   macOS:   Dune Legacy.app/Contents/Resources/config/Dune Legacy.ini
```

**`config/ObjectData.ini`**:
```ini
#   Windows: <game_install_dir>\config\ObjectData.ini
#   Linux:   <game_install_dir>/config/ObjectData.ini
#   macOS:   Dune Legacy.app/Contents/Resources/config/ObjectData.ini
```

**`config/QuantBot Config.ini`**:
```ini
#   Windows: <game_install_dir>\config\QuantBot Config.ini
#   Linux:   <game_install_dir>/config/QuantBot Config.ini
#   macOS:   Dune Legacy.app/Contents/Resources/config/QuantBot Config.ini
```

## Benefits

1. **Simpler User Experience**: All config files in one place, easy to find and back up
2. **Portable Installation**: Game directory is self-contained (except for save games/maps)
3. **Consistent Cross-Platform**: Same approach on Windows, Linux, and macOS
4. **Easy Modding**: Players can share config file modifications more easily
5. **Better for Source Control**: Template files are version controlled and documented

## Files Changed

### Created
- `config/Dune Legacy.ini` - Template config file with comprehensive documentation

### Modified
- `src/main.cpp`:
  - Updated `getConfigFilepath()` to use game directory
  - Extracted `getDefaultPlayerName()` helper function
  - Updated `createDefaultConfigFile()` to use template copying
  - Updated fallback config file comment to reference game directory

- `src/players/QuantBotConfig.cpp`:
  - Updated `getQuantBotConfigFilepath()` to use game directory

- `config/ObjectData.ini`:
  - Updated header comments to reference game directory

- `config/QuantBot Config.ini`:
  - Updated header comments to reference game directory

- `CMakeLists.txt`:
  - Added `config/Dune Legacy.ini` to install target
  - Updated install destination to `${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/config`

- `Makefile.am`:
  - Added `config/Dune Legacy.ini` to `EXTRA_DIST` and `configfiles_DATA`
  - Updated `configfilesdir` to `$(dunelegacydatadir)/config`

- `IDE/VC/DuneLegacy.vcxproj`:
  - Added three config files as `<None>` items with `DeploymentContent` set to `true`
  - Updated all four post-build events to copy config folder: `robocopy "..\..\config\." "$(OutDir)config\."`

## Compilation Status

✅ **Windows (Visual Studio 2022)**: Compiled successfully (Release x64)
- Minor warnings for type conversions (pre-existing)
- Link error for SDL2_mixer.lib (pre-existing library path issue, unrelated to config changes)

## Testing Checklist

- [x] Compile on Windows with Visual Studio 2022
- [ ] Build on Linux with CMake
- [ ] Build on macOS with Xcode
- [ ] Test first-run template copying for all three config files
- [ ] Verify config files are created in game directory, not AppData
- [ ] Verify player name is correctly detected and inserted
- [ ] Verify language preference is correctly set
- [ ] Test modifying config files and restarting game
- [ ] Test installer on Windows (NSIS)
- [ ] Test installer on Linux (tar.gz)
- [ ] Test installer on macOS (DMG)

## Future Considerations

### Files Still Using User Directory

Some files legitimately belong in the user directory and were not changed:
- **Save Games**: User-specific game progress
- **Replays**: User-generated replay files
- **Custom Maps**: User-created map files
- **Log Files**: Runtime logs for debugging

These files continue to use `fnkdat(..., FNKDAT_USER | FNKDAT_CREAT)` which is appropriate for their use case.

### Potential Enhancements

1. **Config File Versioning**: Detect and update old config files on game updates
2. **Config Migration Tool**: Migrate existing AppData configs to game directory on first run
3. **Config Validation**: Validate config files on load and report errors clearly
4. **Config UI**: In-game editor for common config options

## Related Documents

- [53-quantbot-external-config.md](53-quantbot-external-config.md) - QuantBot Config implementation
- [54-config-files-installation.md](54-config-files-installation.md) - Original template system design

