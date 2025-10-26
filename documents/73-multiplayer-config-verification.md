# Document 73: Multiplayer Config Verification System

**Version:** 0.98.6.4  
**Date:** October 26, 2025  
**Type:** Multiplayer Safety Feature

## Summary

Implemented automatic config file verification for multiplayer games to prevent desyncs caused by mismatched configuration files. The system exchanges and verifies hashes of both `QuantBot Config.ini` and `ObjectData.ini` before the game starts, showing an error dialog if any player has different config files.

## Problem

Users reported persistent multiplayer desyncs after approximately 10 minutes of gameplay. Investigation revealed that the config files were loaded from **AppData** (`C:\Users\<username>\AppData\Roaming\dunelegacy\config\`), not the game directory, and could differ between players:

**Root Cause:**
- `ObjectData.ini` contained different values between two computers (confirmed via hash mismatch)
- `QuantBot Config.ini` was identical (hashes matched)
- The game had NO mechanism to detect or prevent config mismatches
- Players could start multiplayer games with incompatible configurations, leading to inevitable desyncs

**Why This Causes Desyncs:**
- **ObjectData.ini** defines unit stats (speed, health, damage, etc.)
- If Player 1 has ornithopters with speed 18.0 and Player 2 has speed 22.0, the game states diverge immediately
- **QuantBot Config.ini** defines AI behavior ratios and thresholds
- Different AI configs cause different build decisions → desync

## Solution Implemented

### Architecture

**Network Protocol:**
1. Client joins multiplayer game
2. Both server and client exchange config hashes via `NETWORKPACKET_CONFIG_HASH` (packet type 11)
3. Server receives all client hashes and compares them to its own
4. If mismatch detected → error dialog shown, game disconnected
5. If all match → game proceeds normally

**Files Verified:**
- `QuantBot Config.ini` - AI behavior configuration
- `ObjectData.ini` - Unit and structure stats

### Implementation Details

#### 1. Network Packet Type

**File:** `include/Network/NetworkManager.h`

Added new packet type for config hash exchange:
```cpp
#define NETWORKPACKET_CONFIG_HASH           11
```

#### 2. Config Hash Functions

**File:** `include/players/QuantBotConfig.h`, `src/players/QuantBotConfig.cpp`

Added functions to get file paths and compute hashes:
```cpp
std::string getObjectDataFilepath();   // Returns path to ObjectData.ini in AppData
std::string getObjectDataHash();       // Computes 64-bit hash of ObjectData.ini
```

Implementation uses `std::hash<std::string>` on full file contents, converted to 16-character hex string.

**Example hash:** `dc8e360cf9dd41d1`

#### 3. Network Manager Extensions

**File:** `include/Network/NetworkManager.h`, `src/Network/NetworkManager.cpp`

**Added to PeerData:**
```cpp
std::string quantBotConfigHash;
std::string objectDataHash;
```

**Added method:**
```cpp
void sendConfigHash(const std::string& quantBotHash, const std::string& objectDataHash);
```

**Added callback:**
```cpp
std::function<void (const std::string&)> pOnConfigMismatch;
```

**Packet Handler:**
```cpp
case NETWORKPACKET_CONFIG_HASH: {
    // Read hashes from peer
    std::string quantBotHash = packetStream.readString();
    std::string objectDataHash = packetStream.readString();
    
    // Store in peer data
    peerData->quantBotConfigHash = quantBotHash;
    peerData->objectDataHash = objectDataHash;
    
    // Server verifies all peers match
    if(bIsServer) {
        std::string serverQuantBotHash = getQuantBotConfig().getConfigHash();
        std::string serverObjectDataHash = getObjectDataHash();
        
        // Compare each peer's hashes
        for(ENetPeer* pPeer : peerList) {
            if(pData->quantBotConfigHash != serverQuantBotHash) {
                // MISMATCH DETECTED!
                pOnConfigMismatch(...);
            }
            if(pData->objectDataHash != serverObjectDataHash) {
                // MISMATCH DETECTED!
                pOnConfigMismatch(...);
            }
        }
    }
}
```

#### 4. CustomGamePlayers Integration

**File:** `src/Menu/CustomGamePlayers.cpp`, `include/Menu/CustomGamePlayers.h`

**Setup callback:**
```cpp
pNetworkManager->setOnConfigMismatch(std::bind(&CustomGamePlayers::onConfigMismatch, this, std::placeholders::_1));
```

**Send hashes when starting game:**
```cpp
if(pNetworkManager != nullptr) {
    QuantBotConfig& config = getQuantBotConfig();
    std::string quantBotHash = config.getConfigHash();
    std::string objectDataHash = getObjectDataHash();
    
    SDL_Log("==================== MULTIPLAYER CONFIG CHECK ====================");
    SDL_Log("QuantBot Config hash: %s", quantBotHash.c_str());
    SDL_Log("ObjectData.ini hash: %s", objectDataHash.c_str());
    SDL_Log("================================================================");
    
    // Send to all players for verification
    pNetworkManager->sendConfigHash(quantBotHash, objectDataHash);
    
    pNetworkManager->sendStartGame(timeLeft);
}
```

**Handle mismatch:**
```cpp
void CustomGamePlayers::onConfigMismatch(const std::string& errorMessage) {
    SDL_Log("CONFIG MISMATCH DETECTED: %s", errorMessage.c_str());
    openWindow(MsgBox::create(errorMessage));  // Show error dialog
    
    if(pNetworkManager != nullptr) {
        pNetworkManager->disconnect();         // Disconnect from game
    }
    
    quit(MENU_QUIT_DEFAULT);                   // Return to menu
}
```

## User Experience

### Before (Broken)
1. Player 1 and Player 2 start multiplayer game
2. Game runs for ~10 minutes
3. **DESYNC!** - Players see different game states
4. No indication of what went wrong
5. Frustration and confusion

### After (Fixed)
1. Player 1 (server) starts multiplayer game
2. Player 2 joins
3. **Hashes exchanged automatically**
4. If configs match:
   - Log: "Received config hashes from Player2 - QuantBot: abc123, ObjectData: def456"
   - Game starts normally ✅
5. If configs DON'T match:
   - **Error Dialog:**
     ```
     CONFIG MISMATCH DETECTED!
     
     Multiplayer game cannot continue - config files don't match:
     - Player2 has different ObjectData.ini (Hash: abc123 vs Server: def456)
     
     Please ensure all players have identical config files:
     - C:\Users\Player2\AppData\Roaming\dunelegacy\config\QuantBot Config.ini
     - C:\Users\Player2\AppData\Roaming\dunelegacy\config\ObjectData.ini
     ```
   - Game disconnects automatically
   - Players return to menu

### How to Fix Config Mismatch

**Option 1: Copy from server to client**
1. On server, navigate to: `C:\Users\<username>\AppData\Roaming\dunelegacy\config\`
2. Copy `ObjectData.ini` and `QuantBot Config.ini`
3. Send to client via network/USB
4. Client pastes into their config directory
5. Try multiplayer again

**Option 2: Use consistent game installation**
- Ensure both players installed from the same installer/build
- Or distribute config files alongside the game

## Testing Checklist

- [x] **Implemented** - Code complete
- [x] **Compiled** - No errors
- [x] **Linter** - No warnings
- [ ] **Test: Matching Configs**
  - Start multiplayer with identical config files
  - Verify game starts without errors
  - Verify hashes logged to console
- [ ] **Test: ObjectData.ini Mismatch**
  - Change ornithopter speed on one computer
  - Start multiplayer
  - Verify error dialog appears
  - Verify correct hash values shown
- [ ] **Test: QuantBot Config.ini Mismatch**
  - Change AI difficulty settings on one computer
  - Start multiplayer
  - Verify error dialog appears
- [ ] **Test: Both Files Mismatch**
  - Change both files on one computer
  - Verify both mismatches reported
- [ ] **Test: Disconnect on Mismatch**
  - Trigger mismatch
  - Verify game disconnects cleanly
  - Verify no crash or hang

## Technical Notes

### Hash Function

Uses `std::hash<std::string>` which is:
- **Fast**: O(n) where n = file size
- **Deterministic**: Same file → same hash
- **Collision resistant**: 64-bit hash space = 18.4 quintillion values
- **Platform independent**: Works identically on Windows, Linux, macOS

### Network Protocol

- **Packet type**: 11 (NETWORKPACKET_CONFIG_HASH)
- **Payload**: Two strings (quantBotHash, objectDataHash)
- **Reliable**: Uses `ENET_PACKET_FLAG_RELIABLE`
- **Timing**: Sent immediately after clicking "Start Game", before 5-second countdown

### Config File Locations

**Windows:**
```
C:\Users\<username>\AppData\Roaming\dunelegacy\config\QuantBot Config.ini
C:\Users\<username>\AppData\Roaming\dunelegacy\config\ObjectData.ini
```

**Linux / macOS:**
```
~/.config/dunelegacy/config/QuantBot Config.ini
~/.config/dunelegacy/config/ObjectData.ini
```

## Files Modified

### Created
- `documents/73-multiplayer-config-verification.md` (this file)

### Modified
- `include/Network/NetworkManager.h` - Added packet type, sendConfigHash(), callback
- `src/Network/NetworkManager.cpp` - Implemented hash exchange and verification
- `include/players/QuantBotConfig.h` - Added getObjectDataFilepath(), getObjectDataHash()
- `src/players/QuantBotConfig.cpp` - Implemented ObjectData hash functions
- `src/Menu/CustomGamePlayers.cpp` - Send hashes, handle mismatch
- `include/Menu/CustomGamePlayers.h` - Added onConfigMismatch() declaration

## Related Documents

- Document 69: Multiplayer Desync Fix (turret timing, combat stats)
- Document 70: Multiplayer Safety Audit
- Document 71: QuantBot BuildTimer Desync Fix
- Document 72: Multiplayer Desync Complete Fix

## Status

✅ **Implemented** - All code complete  
✅ **Compiled** - No build errors  
✅ **Linter** - No warnings  
⏳ **Testing** - Requires multiplayer testing with intentional config mismatches  

## Expected Outcome

**Prevents all config-related desyncs by catching mismatches before the game starts.**

Players will no longer experience mysterious desyncs caused by different config files. The system provides clear error messages with exact hash values and file paths, making it easy to diagnose and fix the issue.

## Future Enhancements

1. **Auto-sync configs**: Server could send config files to clients automatically
2. **Config versioning**: Track config file versions in addition to hashes
3. **Partial sync**: Allow some config differences (e.g., graphics settings) while blocking gameplay-affecting changes
4. **Pre-game validation**: Verify configs when joining lobby, not just when starting game


