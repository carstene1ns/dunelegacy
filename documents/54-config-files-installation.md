# Configuration Files Installation System

## Summary
Changed the configuration system from programmatic file creation to installing template files with the game. This provides better documentation, easier customization, and clearer default values.

## Files Changed

### New Configuration Template Files Created:
1. **`config/ObjectData.ini`** - Unit and structure stats configuration
   - Enhanced header with full documentation from https://dunelegacy.sourceforge.net/website/development/modding.html
   - House-specific override examples
   - Multiplayer compatibility warnings
   
2. **`config/QuantBot Config.ini`** - AI behavior configuration
   - Per-difficulty settings (attack behavior, limits, etc.)
   - Per-difficulty, per-house unit composition ratios
   - General AI behavior parameters
   - Extensive inline documentation

### Code Changes:

#### `src/players/QuantBotConfig.cpp`
**Old behavior:** Programmatically created config file with `save()` if missing

**New behavior:**
1. Check if config exists in user directory
2. If not, copy template from install directory using `pFileManager->openFile()`
3. Fallback to programmatic creation if template not found
4. Log all actions for debugging

```cpp
// Try to copy template from install directory
auto templateFile = pFileManager->openFile("QuantBot Config.ini");
if (templateFile) {
    INIFile templateINI(templateFile.get());
    templateINI.saveChangesTo(filepath);
} else {
    // Fallback: create programmatically
    return save(filepath);
}
```

#### `CMakeLists.txt`
Added install directive to copy config files to data directory:

```cmake
install(FILES 
    config/ObjectData.ini
    "config/QuantBot Config.ini"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
)
```

#### `Makefile.am`
Added config files to both distribution and installation:

```makefile
# Add to EXTRA_DIST for source distributions
EXTRA_DIST = ... \
             config/ObjectData.ini \
             config/QuantBot\ Config.ini \
             ...

# Install to data directory
configfiles_DATA = config/ObjectData.ini \
                   config/QuantBot\ Config.ini
configfilesdir = $(dunelegacydatadir)
```

## Installation Locations

### Template Files (Installed with Game):
```
Windows:  <install_dir>\dunelegacy\ObjectData.ini
          <install_dir>\dunelegacy\QuantBot Config.ini
          
Linux:    /usr/share/dunelegacy/ObjectData.ini
          /usr/share/dunelegacy/QuantBot Config.ini
          
macOS:    Dune Legacy.app/Contents/Resources/ObjectData.ini
          Dune Legacy.app/Contents/Resources/QuantBot Config.ini
```

### User Config Files (Copied on First Run):
```
Windows:  C:\Users\<user>\AppData\Roaming\dunelegacy\data\ObjectData.ini
          C:\Users\<user>\AppData\Roaming\dunelegacy\QuantBot Config.ini
          
Linux:    ~/.config/dunelegacy/data/ObjectData.ini
          ~/.config/dunelegacy/QuantBot Config.ini
          
macOS:    ~/.config/dunelegacy/data/ObjectData.ini
          ~/.config/dunelegacy/QuantBot Config.ini
```

## File Search Order

The game uses `FileManager::openFile()` which searches in this order:
1. Game installation data directory
2. User config directory (`AppData\Roaming\dunelegacy\data\` or `~/.config/dunelegacy/data/`)
3. Inside PAK files (LEGACY.PAK, OPENSD2.PAK, GFXHD.PAK)

This means:
- Template files are found in installation directory
- User can override by placing custom files in config directory
- Built-in defaults in PAK files provide final fallback

## Benefits

### 1. Better Documentation
Config files now include:
- Complete property reference
- Usage examples
- House-specific override syntax
- Multiplayer warnings
- Direct links to online documentation

### 2. Easier Customization
Users can:
- Open config files in any text editor
- See all available options with descriptions
- Copy and modify template files
- Share custom configs with friends

### 3. Version Control Friendly
- Config templates are now part of source code
- Changes tracked in git
- Easy to diff between versions
- Contributors can propose config changes via PRs

### 4. Reduced Code Complexity
- No need to maintain programmatic config generation in multiple places
- Single source of truth for default values
- Easier to add new config options

### 5. Platform Independent
- Same template files work on all platforms
- Build system handles installation per platform
- User directory location handled by `fnkdat()`

## Testing

### Build System:
```bash
# Linux/macOS
cmake -B build
make -C build install

# Verify files installed:
ls -la /usr/local/share/dunelegacy/*.ini

# Windows
cmake -B build
cmake --build build --target install

# Verify files installed in Program Files
```

### Runtime Behavior:
1. Delete existing user config: `rm ~/.config/dunelegacy/"QuantBot Config.ini"`
2. Run game
3. Check logs for: "Copying QuantBot Config template to user directory..."
4. Verify file copied: `ls -la ~/.config/dunelegacy/"QuantBot Config.ini"`
5. Edit file and restart game to verify changes take effect

## Future Work

Potential additional config files to add:
- `Dune Legacy.ini` - Main game settings template
- `Controls.ini` - Keyboard/mouse bindings
- `Graphics.ini` - Advanced graphics settings
- `Audio.ini` - Sound and music settings

## Migration Notes

### For Users:
- Existing custom configs are **not affected**
- Game only copies template if user config doesn't exist
- Backup your configs before upgrading (recommended)

### For Developers:
- Add new config options to template files in `config/` directory
- Maintain both template file and programmatic defaults (for fallback)
- Update documentation when adding new properties

### For Package Maintainers:
- Config template files now part of standard installation
- Installed to same directory as PAK files
- Source distributions include `config/` directory

## Related Documentation

- [Modding Guide](https://dunelegacy.sourceforge.net/website/development/modding.html)
- [QuantBot AI Configuration](./53-quantbot-external-config.md)
- [ObjectData.ini Property Reference](../config/ObjectData.ini)
- [QuantBot Config.ini Reference](../config/QuantBot%20Config.ini)

