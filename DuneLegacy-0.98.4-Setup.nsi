;NSIS Modern User Interface
;Dune Legacy 0.98.4 Setup
;Updated for version 0.98.4

!include "MUI2.nsh"

;--------------------------------
;General

  Name "Dune Legacy"
  OutFile "build\installer\Dune Legacy 0.98.4 Setup.exe"
  Unicode True

  ;Default installation folder
  InstallDir "$PROGRAMFILES64\Dune Legacy"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Dune Legacy" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !define MUI_ICON "dunelegacy.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "nsis\modern-header.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP "nsis\modern-wizard.bmp"
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Dune Legacy" GameFiles

  SetOutPath "$INSTDIR"

  ; Main executable and DLLs
  File "bin\Release-x64\DuneLegacy.exe"
  File "bin\Release-x64\SDL2.dll"
  File "bin\Release-x64\SDL2_mixer.dll"
  File "bin\Release-x64\SDL2_ttf.dll"

  ; Game data files
  File "bin\Release-x64\*.PAK"

  ; Documentation
  File "COPYING"
  File "AUTHORS"
  File "README"

  ; Maps
  SetOutPath "$INSTDIR\maps\singleplayer"
  File "data\maps\singleplayer\*.*"

  SetOutPath "$INSTDIR\maps\multiplayer"
  File "data\maps\multiplayer\*.ini"

  ; Locale files
  SetOutPath "$INSTDIR\locale"
  File "data\locale\*.po"

  ; Store installation folder
  WriteRegStr HKCU "Software\Dune Legacy" "" $INSTDIR

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Create desktop shortcut
  CreateShortCut "$DESKTOP\Dune Legacy.lnk" "$INSTDIR\DuneLegacy.exe"

  ; Create start menu entries
  CreateDirectory "$SMPROGRAMS\Dune Legacy"
  CreateShortCut "$SMPROGRAMS\Dune Legacy\Dune Legacy.lnk" "$INSTDIR\DuneLegacy.exe"
  CreateShortCut "$SMPROGRAMS\Dune Legacy\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; Remove files
  Delete "$INSTDIR\DuneLegacy.exe"
  Delete "$INSTDIR\SDL2.dll"
  Delete "$INSTDIR\SDL2_mixer.dll"
  Delete "$INSTDIR\SDL2_ttf.dll"
  Delete "$INSTDIR\*.PAK"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\AUTHORS"
  Delete "$INSTDIR\README"
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove directories
  RMDir /r "$INSTDIR\maps"
  RMDir /r "$INSTDIR\locale"
  RMDir "$INSTDIR"

  ; Remove shortcuts
  Delete "$DESKTOP\Dune Legacy.lnk"
  Delete "$SMPROGRAMS\Dune Legacy\Dune Legacy.lnk"
  Delete "$SMPROGRAMS\Dune Legacy\Uninstall.lnk"
  RMDir "$SMPROGRAMS\Dune Legacy"

  ; Remove registry entries
  DeleteRegKey HKCU "Software\Dune Legacy"

SectionEnd

