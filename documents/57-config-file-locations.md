# Configuration File Locations - Dune Legacy.ini

**Date**: October 24, 2024  
**Version**: 0.98.6a  
**Commit**: c644103

## Summary

Changed `Dune Legacy.ini` loading behavior to follow standard application practices:
- **User config** stored in OS-specific user directories (AppData, ~/.config, etc.)
- **Template file** remains in game installation directory as read-only reference
- On first run, template is copied to user directory with personalized defaults

## Problem

Previously, `Dune Legacy.ini` was loaded from `<game_install>/config/Dune Legacy.ini`, which:
- Mixed user settings with application files
- Made settings difficult to find for users
- Could be overwritten during game updates
- Didn't follow OS-specific conventions

## Solution

### User Config Locations (Actual Settings)

**Windows**:  
`C:\Users\<username>\AppData\Roaming\dunelegacy\Dune Legacy.ini`

**Linux**:  
`~/.config/dunelegacy/Dune Legacy.ini`

**macOS**:  
`~/Library/Application Support/dunelegacy/Dune Legacy.ini`

### Template Location (Read-Only Reference)

**All Platforms**:  
`<game_install>/config/Dune Legacy.ini`

## Implementation Details

### Code Changes

#### `src/main.cpp`

1. **Updated `getConfigFilepath()`**:
```cpp
std::string getConfigFilepath()
{
    // User config file is stored in user directory
    char tmp[FILENAME_MAX];
    if(fnkdat(CONFIGFILENAME, tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT) < 0) {
        THROW(std::runtime_error, "fnkdat() failed for config file!");
    }
    return std::string(tmp);
}
```

2. **Added `getConfigTemplateFilepath()`**:
```cpp
std::string getConfigTemplateFilepath()
{
    // Template config file is in config subdirectory of game directory
    return getDuneLegacyDataDir() + "/config/" + CONFIGFILENAME;
}
```

3. **Updated `createDefaultConfigFile()`**:
   - Now copies from `config/Dune Legacy.ini` template
   - Logs both template and user config locations
   - Falls back to programmatic creation if template not found

#### `config/Dune Legacy.ini`

Updated header to clearly indicate:
- This is a template file
- Where the actual user config will be created
- How to reset settings (delete user config file)
- Platform-specific paths for each OS

## Benefits

### For Users
- ✅ Settings persist across game updates
- ✅ Easy to find and back up config files
- ✅ Can reset to defaults by deleting user config
- ✅ Multiple game installations can share same settings

### For Developers
- ✅ Can update template without affecting user settings
- ✅ Follows OS conventions for application data
- ✅ Cleaner separation of read-only and user-writable data
- ✅ Easier to troubleshoot config issues

## First Run Behavior

1. Game checks for user config file in AppData (or equivalent)
2. If not found, copies template from `config/Dune Legacy.ini`
3. Personalizes template with:
   - System username as player name
   - System language preference
   - Optimal screen resolution
4. Saves personalized config to user directory
5. All subsequent runs read from user directory

## Resetting Settings

Users can reset to default settings:
1. Exit the game
2. Delete user config file from AppData
3. Restart game - template will be copied again

## Other Config Files

The following config files remain in game directory (as intended):
- `config/ObjectData.ini` - Unit/structure stats (read-only, game data)
- `config/QuantBot Config.ini` - AI behavior (read-only, game balance)

These are game data files that should not be user-modified by default.

## Testing

Tested on Windows:
- ✅ Existing `AppData/Roaming/dunelegacy/Dune Legacy.ini` is correctly loaded
- ✅ Template file exists in `bin/Release-x64/config/Dune Legacy.ini`
- ✅ Template header correctly explains usage
- ✅ Game loads and saves settings to AppData location

## Related Files

- `src/main.cpp` - Config file loading logic
- `config/Dune Legacy.ini` - Template file with documentation
- `CMakeLists.txt` - Installs template to config directory
- `Makefile.am` - Installs template to config directory
- `IDE/VC/DuneLegacy.vcxproj` - Copies template in post-build

## Notes

- Save games, replays, and logs continue to use user directories (as before)
- The `fnkdat()` function handles OS-specific path resolution
- Template copying includes error handling and fallback to programmatic creation
- This change brings Dune Legacy in line with modern application standards

