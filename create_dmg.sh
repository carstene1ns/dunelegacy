#!/bin/bash

# Set variables
APP_PATH="IDE/xCode/build/Release/Dune Legacy.app"
DMG_NAME="DuneLegacy-0.97.4-macOS.dmg"
VOLUME_NAME="Dune Legacy"

# Create a temporary directory for DMG contents
TEMP_DIR=$(mktemp -d)
mkdir -p "$TEMP_DIR/Dune Legacy"

# Copy the app bundle
cp -R "$APP_PATH" "$TEMP_DIR/Dune Legacy/"

# Create symlink to Applications folder
ln -s /Applications "$TEMP_DIR/Dune Legacy/"

# Create DMG
hdiutil create -volname "$VOLUME_NAME" -srcfolder "$TEMP_DIR/Dune Legacy" -ov -format UDZO "$DMG_NAME"

# Clean up
rm -rf "$TEMP_DIR" 