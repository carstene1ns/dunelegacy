; NSIS script NSIS-2 BadCmd=11
; Install

SetCompressor /SOLID lzma
SetCompressorDictSize 8

; --------------------
; HEADER SIZE: 49078
; START HEADER SIZE: 300
; MAX STRING LENGTH: 1024
; STRING CHARS: 27702

OutFile [NSIS].exe
!include WinMessages.nsh

LicenseBkColor /windows


; --------------------
; LANG TABLES: 2
; LANG STRINGS: 82

Name "Dune Legacy"
BrandingText " http://dunelegacy.sourceforge.net"

; LANG: 1031
LangString LSTR_0 1031 " http://dunelegacy.sourceforge.net"
LangString LSTR_1 1031 "$(LSTR_2) Installation"
LangString LSTR_2 1031 "Dune Legacy"
LangString LSTR_3 1031 "Verfügbarer Speicher: "
LangString LSTR_4 1031 "Benötigter Speicher: "
LangString LSTR_5 1031 "Fehler beim Schreiben: "
LangString LSTR_6 1031 "Kopieren fehlgeschlagen"
LangString LSTR_7 1031 "Kopiere nach "
LangString LSTR_8 1031 "Symbol ist nicht vorhanden: "
LangString LSTR_9 1031 "Fehler beim Laden von "
LangString LSTR_10 1031 "Erstelle Verzeichnis: "
LangString LSTR_11 1031 "Erstelle Verknüpfung: "
LangString LSTR_12 1031 "Erstelle Deinstallations-Programm: "
LangString LSTR_13 1031 "Lösche Datei: "
LangString LSTR_14 1031 "Lösche Datei nach Neustart: "
LangString LSTR_15 1031 "Fehler beim Erstellen der Verknüpfung: "
LangString LSTR_16 1031 "Fehler beim Erstellen: "
LangString LSTR_17 1031 "Fehler beim Dekomprimieren. Beschädigtes Installations-Programm?"
LangString LSTR_21 1031 "Dekomprimiere: "
LangString LSTR_22 1031 "Dekomprimierung: Fehler beim Schreiben der Datei "
LangString LSTR_23 1031 "Beschädigtes Installations-Programm: ungültiger Befehlscode"
LangString LSTR_24 1031 "Kein OLE für: "
LangString LSTR_25 1031 "Zielverzeichnis: "
LangString LSTR_26 1031 "Entferne Verzeichnis: "
LangString LSTR_29 1031 "Übersprungen: "
LangString LSTR_30 1031 "Details in die Zwischenablage kopieren"
LangString LSTR_32 1031 B
LangString LSTR_33 1031 K
LangString LSTR_34 1031 M
LangString LSTR_35 1031 G
LangString LSTR_36 1031 "Fehler beim Überschreiben der Datei: $\r$\n$\t$\"$0$\"$\r$\nKlicken Sie auf Abbrechen, um abzubrechen,$\r$\nauf Wiederholen, um den Schreibvorgang erneut zu versuchen$\r$\noder auf Ignorieren, um diese Datei zu überspringen."
LangString LSTR_37 1031 0
LangString LSTR_38 1031 "Willkommen beim Installations-$\r$\nAssistenten für $(LSTR_81)"
LangString LSTR_39 1031 "MS Shell Dlg"
LangString LSTR_40 1031 "Dieser Assistent wird Sie durch die Installation von $(LSTR_81) begleiten.$\r$\n$\r$\nEs wird empfohlen, vor der Installation alle anderen Programme zu schließen, damit bestimmte Systemdateien ohne Neustart ersetzt werden können.$\r$\n$\r$\n$_CLICK"
LangString LSTR_41 1031 "Falls Sie alle Bedingungen des Abkommens akzeptieren, klicken Sie auf Annehmen. Sie müssen die Lizenzvereinbarungen anerkennen, um $(LSTR_81) installieren zu können."
LangString LSTR_42 1031 Lizenzabkommen
LangString LSTR_43 1031 "Bitte lesen Sie die Lizenzbedingungen durch, bevor Sie mit der Installation fortfahren."
LangString LSTR_44 1031 "Drücken Sie die Bild-nach-unten Taste, um den Rest des Abkommens zu sehen."
LangString LSTR_45 1031 "Zielverzeichnis auswählen"
LangString LSTR_46 1031 "Wählen Sie das Verzeichnis aus, in das $(LSTR_81) installiert werden soll."
LangString LSTR_47 1031 "Dune Legacy benötigt die PAK-Dateien des Originalspiels, welche im Dune II Verzeichnis liegen. Die folgenden Dateien werden von dort in das Dune Legacy Verzeichnis kopiert:$\n$\tHARK.PAK$\t$\tSCENARIO.PAK$\t$\tINTRO.PAK$\n$\tATRE.PAK$\t$\tMENTAT.PAK$\t$\tINTROVOC.PAK$\n$\tORDOS.PAK$\t$\tVOC.PAK$\t$\tSOUND.PAK$\n$\tENGLISH.PAK$\t$\tMERC.PAK$\t$\tGERMAN.PAK (falls vorhanden)$\n$\tDUNE.PAK$\t$\tFINALE.PAK$\t$\tFRENCH.PAK (falls vorhanden)"
LangString LSTR_48 1031 "Verzeichnis mit Dune II Pak-Dateien"
LangString LSTR_49 1031 "Dune II Pak-Dateien"
LangString LSTR_50 1031 "Wählen Sie das Verzeichnis aus, von dem der Installer die Dune II Pak-Dateien kopieren kann."
LangString LSTR_51 1031 Installiere...
LangString LSTR_52 1031 "Bitte warten Sie, während $(LSTR_81) installiert wird."
LangString LSTR_53 1031 "Die Installation ist vollständig"
LangString LSTR_54 1031 "Die Installation wurde erfolgreich abgeschlossen."
LangString LSTR_55 1031 "Abbruch der Installation"
LangString LSTR_56 1031 "Die Installation wurde nicht vollständig abgeschlossen."
LangString LSTR_57 1031 "&Fertig stellen"
LangString LSTR_58 1031 "Die Installation von $(LSTR_81) wird abgeschlossen"
LangString LSTR_59 1031 "Windows muss neu gestartet werden, um die Installation von $(LSTR_81) zu vervollständigen. Möchten Sie Windows jetzt neu starten?"
LangString LSTR_60 1031 "Jetzt neu starten"
LangString LSTR_61 1031 "Windows später selbst neu starten"
LangString LSTR_62 1031 "$(LSTR_81) wurde auf Ihrem Computer installiert.$\r$\n$\r$\nKlicken Sie auf Fertig stellen, um den Installations-Assistenten zu schließen."
LangString LSTR_63 1031 8
LangString LSTR_64 1031 "Dune Legacy deinstallieren"
LangString LSTR_65 1031 Benutzerdefiniert
LangString LSTR_66 1031 Abbrechen
LangString LSTR_67 1031 "< &Zurück"
LangString LSTR_68 1031 "&Weiter >"
LangString LSTR_69 1031 "Klicken Sie auf Weiter, um fortzufahren."
LangString LSTR_70 1031 &Annehmen
LangString LSTR_71 1031 "$(LSTR_81) wird in das unten angegebene Verzeichnis installiert. Falls Sie in ein anderes Verzeichnis installieren möchten, klicken Sie auf Durchsuchen und wählen Sie ein anderes Verzeichnis aus. $_CLICK"
LangString LSTR_72 1031 Zielverzeichnis
LangString LSTR_73 1031 &Durchsuchen...
LangString LSTR_74 1031 "Wählen Sie das Verzeichnis aus, in das Sie $(LSTR_81) installieren möchten:"
LangString LSTR_75 1031 &Installieren
LangString LSTR_76 1031 "Klicken Sie auf Installieren, um die Installation zu starten."
LangString LSTR_77 1031 "&Details anzeigen"
LangString LSTR_78 1031 Fertig
LangString LSTR_79 1031 " "
LangString LSTR_80 1031 &Beenden
LangString LSTR_81 1031 "Dune Legacy"


; LANG: 1033
LangString LSTR_0 1033 " http://dunelegacy.sourceforge.net"
LangString LSTR_1 1033 "$(LSTR_2) Setup"
LangString LSTR_2 1033 "Dune Legacy"
LangString LSTR_3 1033 "Space available: "
LangString LSTR_4 1033 "Space required: "
LangString LSTR_5 1033 "Can't write: "
LangString LSTR_6 1033 "Copy failed"
LangString LSTR_7 1033 "Copy to "
LangString LSTR_8 1033 "Could not find symbol: "
LangString LSTR_9 1033 "Could not load: "
LangString LSTR_10 1033 "Create folder: "
LangString LSTR_11 1033 "Create shortcut: "
LangString LSTR_12 1033 "Created uninstaller: "
LangString LSTR_13 1033 "Delete file: "
LangString LSTR_14 1033 "Delete on reboot: "
LangString LSTR_15 1033 "Error creating shortcut: "
LangString LSTR_16 1033 "Error creating: "
LangString LSTR_17 1033 "Error decompressing data! Corrupted installer?"
LangString LSTR_21 1033 "Extract: "
LangString LSTR_22 1033 "Extract: error writing to file "
LangString LSTR_23 1033 "Installer corrupted: invalid opcode"
LangString LSTR_24 1033 "No OLE for: "
LangString LSTR_25 1033 "Output folder: "
LangString LSTR_26 1033 "Remove folder: "
LangString LSTR_29 1033 "Skipped: "
LangString LSTR_30 1033 "Copy Details To Clipboard"
LangString LSTR_32 1033 B
LangString LSTR_33 1033 K
LangString LSTR_34 1033 M
LangString LSTR_35 1033 G
LangString LSTR_36 1033 "Error opening file for writing: $\r$\n$\r$\n$0$\r$\n$\r$\nClick Abort to stop the installation,$\r$\nRetry to try again, or$\r$\nIgnore to skip this file."
LangString LSTR_37 1033 0
LangString LSTR_38 1033 "Welcome to the $(LSTR_81) Setup Wizard"
LangString LSTR_39 1033 "MS Shell Dlg"
LangString LSTR_40 1033 "This wizard will guide you through the installation of $(LSTR_81).$\r$\n$\r$\nIt is recommended that you close all other applications before starting Setup. This will make it possible to update relevant system files without having to reboot your computer.$\r$\n$\r$\n$_CLICK"
LangString LSTR_41 1033 "If you accept the terms of the agreement, click I Agree to continue. You must accept the agreement to install $(LSTR_81)."
LangString LSTR_42 1033 "License Agreement"
LangString LSTR_43 1033 "Please review the license terms before installing $(LSTR_81)."
LangString LSTR_44 1033 "Press Page Down to see the rest of the agreement."
LangString LSTR_45 1033 "Choose Install Location"
LangString LSTR_46 1033 "Choose the folder in which to install $(LSTR_81)."
LangString LSTR_47 1033 "Dune Legacy needs the PAK-Files from original Dune II. These files can be found in the Dune II folder. The installer will copy the following files from there to the Dune Legacy folder:$\n$\tHARK.PAK$\t$\tSCENARIO.PAK$\t$\tINTRO.PAK$\n$\tATRE.PAK$\t$\tMENTAT.PAK$\t$\tINTROVOC.PAK$\n$\tORDOS.PAK$\t$\tVOC.PAK$\t$\tSOUND.PAK$\n$\tENGLISH.PAK$\t$\tMERC.PAK$\t$\tGERMAN.PAK (if available)$\n$\tDUNE.PAK$\t$\tFINALE.PAK$\t$\tFRENCH.PAK (if available)"
LangString LSTR_48 1033 "Dune II Pak-Files Directory"
LangString LSTR_49 1033 "Dune II Pak-Files"
LangString LSTR_50 1033 "Choose the directory where the installer can copy the Dune II Pak-Files from."
LangString LSTR_51 1033 Installing
LangString LSTR_52 1033 "Please wait while $(LSTR_81) is being installed."
LangString LSTR_53 1033 "Installation Complete"
LangString LSTR_54 1033 "Setup was completed successfully."
LangString LSTR_55 1033 "Installation Aborted"
LangString LSTR_56 1033 "Setup was not completed successfully."
LangString LSTR_57 1033 &Finish
LangString LSTR_58 1033 "Completing the $(LSTR_81) Setup Wizard"
LangString LSTR_59 1033 "Your computer must be restarted in order to complete the installation of $(LSTR_81). Do you want to reboot now?"
LangString LSTR_60 1033 "Reboot now"
LangString LSTR_61 1033 "I want to manually reboot later"
LangString LSTR_62 1033 "$(LSTR_81) has been installed on your computer.$\r$\n$\r$\nClick Finish to close this wizard."
LangString LSTR_63 1033 8
LangString LSTR_64 1033 "Uninstall Dune Legacy"
LangString LSTR_65 1033 Custom
LangString LSTR_66 1033 Cancel
LangString LSTR_67 1033 "< &Back"
LangString LSTR_68 1033 "&Next >"
LangString LSTR_69 1033 "Click Next to continue."
LangString LSTR_70 1033 "I &Agree"
LangString LSTR_71 1033 "Setup will install $(LSTR_81) in the following folder. To install in a different folder, click Browse and select another folder. $_CLICK"
LangString LSTR_72 1033 "Destination Folder"
LangString LSTR_73 1033 B&rowse...
LangString LSTR_74 1033 "Select the folder to install $(LSTR_81) in:"
LangString LSTR_75 1033 &Install
LangString LSTR_76 1033 "Click Install to start the installation."
LangString LSTR_77 1033 "Show &details"
LangString LSTR_78 1033 Completed
LangString LSTR_79 1033 " "
LangString LSTR_80 1033 &Close
LangString LSTR_81 1033 "Dune Legacy"


; --------------------
; VARIABLES: 49

Var _0_
Var _1_
Var _2_
Var _3_
Var _4_
Var _5_
Var _6_
Var _7_
Var _8_
Var _9_
Var _10_
Var _11_
Var _12_
Var _13_
Var _14_
Var _15_
Var _16_
Var _17_
Var _18_
Var _19_
Var _20_
Var _21_
Var _22_
Var _23_
Var _24_
Var _25_
Var _26_
Var _27_
Var _28_
Var _29_
Var _30_
Var _31_
Var _32_
Var _33_
Var _34_
Var _35_
Var _36_
Var _37_
Var _38_
Var _39_
Var _40_
Var _41_
Var _42_
Var _43_
Var _44_
Var _45_
Var _46_
Var _47_
Var _48_


InstType $(LSTR_65)    ;  Custom
InstallDir "$PROGRAMFILES\Dune Legacy"
; install_directory_auto_append = "Dune Legacy"
; wininit = $WINDIR\wininit.ini


; --------------------
; PAGES: 7

/*
; Page 0
; Page custom func_24 func_139 /ENABLECANCEL


PageEx
custom func_24 func_139 /ENABLECANCEL
PageExEnd

PageEx license
  LicenseText "Readme"
  LicenseData [LICENSE].txt
PageExEnd

/*
; Page 1
Page license func_140 func_143 func_149 /ENABLECANCEL
  LicenseText $(LSTR_41) $(LSTR_70)    ;  "If you accept the terms of the agreement, click I Agree to continue. You must accept the agreement to install $(LSTR_81)." "I &Agree" "Dune Legacy"
  LicenseData [LICENSE].txt
*/
/*
; Page 2
Page directory func_150 func_153 func_161 /ENABLECANCEL
  DirText $(LSTR_71) $(LSTR_72) $(LSTR_73) $(LSTR_74)    ;  "Setup will install $(LSTR_81) in the following folder. To install in a different folder, click Browse and select another folder. $_CLICK" "Destination Folder" B&rowse... "Select the folder to install $(LSTR_81) in:" "Dune Legacy" "Dune Legacy"
  DirVar $CMDLINE

; Page 3
Page directory func_162 func_166 func_174 /ENABLECANCEL
  DirText $(LSTR_47) $(LSTR_48) $(LSTR_73) $(LSTR_74)    ;  "Dune Legacy needs the PAK-Files from original Dune II. These files can be found in the Dune II folder. The installer will copy the following files from there to the Dune Legacy folder:$\n$\tHARK.PAK$\t$\tSCENARIO.PAK$\t$\tINTRO.PAK$\n$\tATRE.PAK$\t$\tMENTAT.PAK$\t$\tINTROVOC.PAK$\n$\tORDOS.PAK$\t$\tVOC.PAK$\t$\tSOUND.PAK$\n$\tENGLISH.PAK$\t$\tMERC.PAK$\t$\tGERMAN.PAK (if available)$\n$\tDUNE.PAK$\t$\tFINALE.PAK$\t$\tFRENCH.PAK (if available)" "Dune II Pak-Files Directory" B&rowse... "Select the folder to install $(LSTR_81) in:" "Dune Legacy"
  DirVar $_28_

; Page 4
Page instfiles func_176 func_179 func_185
  CompletedText $(LSTR_78)    ;  Completed
  DetailsButtonText $(LSTR_77)    ;  "Show &details"
*/
/*
; Page 5
Page COMPLETED
*/

; Page 6
Page custom func_199 func_381


; --------------------
; SECTIONS: 2
; COMMANDS: 636

Function func_0
  InitPluginsDir
    ; Call Initialize_____Plugins
    ; SetDetailsPrint lastused
  File $PLUGINSDIR\modern-wizard.bmp
FunctionEnd


Function func_4
  LockWindow on
  ShowWindow $_6_ ${SW_HIDE}
  ShowWindow $_5_ ${SW_HIDE}
  ShowWindow $_0_ ${SW_HIDE}
  ShowWindow $_2_ ${SW_HIDE}
  ShowWindow $_4_ ${SW_HIDE}
  ShowWindow $_7_ ${SW_HIDE}
  ShowWindow $_8_ ${SW_SHOWNORMAL}
  LockWindow off
FunctionEnd


Function func_14
  LockWindow on
  ShowWindow $_6_ ${SW_SHOWNORMAL}
  ShowWindow $_5_ ${SW_SHOWNORMAL}
  ShowWindow $_0_ ${SW_SHOWNORMAL}
  ShowWindow $_2_ ${SW_SHOWNORMAL}
  ShowWindow $_4_ ${SW_SHOWNORMAL}
  ShowWindow $_7_ ${SW_SHOWNORMAL}
  ShowWindow $_8_ ${SW_HIDE}
  LockWindow off
FunctionEnd


Function func_24    ; Page 0, Pre
  nsDialogs::Create 1044
    ; Call Initialize_____Plugins
    ; SetOverwrite off
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push 1044
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll Create
  Pop $_12_
  nsDialogs::SetRTL $(LSTR_37)    ;  0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_37)    ;  0
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll SetRTL
  SetCtlColors $_12_ "" 0xFFFFFF
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x0000000E|0x00000100 0 0u 0u 109u 193u ""
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push ""
    ; Push 193u
    ; Push 109u
    ; Push 0u
    ; Push 0u
    ; Push 0
    ; Push 0x40000000|0x10000000|0x04000000|0x0000000E|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_13_
  Push $0
  Push $1
  Push $2
  Push $R0
  StrCpy $R0 $_13_
  StrCpy $1 ""
  StrCpy $2 ""
  System::Call "*(i, i, i, i) i.s"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "*(i, i, i, i) i.s"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $0
  IntCmp $0 0 label_80
  System::Call "user32::GetClientRect(iR0, ir0)"
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "user32::GetClientRect(iR0, ir0)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call "*$0(i, i, i .s, i .s)"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "*$0(i, i, i .s, i .s)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Free $0
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; CallInstDLL $PLUGINSDIR\System.dll Free
  Pop $1
  Pop $2
label_80:
  System::Call "user32::LoadImage(i0, ts, i 0, ir1, ir2, i0x0010) i.s" $PLUGINSDIR\modern-wizard.bmp
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $PLUGINSDIR\modern-wizard.bmp
    ; Push "user32::LoadImage(i0, ts, i 0, ir1, ir2, i0x0010) i.s"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $0
  SendMessage $R0 0x0172 0 $0
  Pop $R0
  Pop $2
  Pop $1
  Exch $0
    ; Push $0
    ; Exch
    ; Pop $0
  Pop $_14_
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x00000100 0x00000020 120u 10u 195u 28u $(LSTR_38)    ;  "Welcome to the $(LSTR_81) Setup Wizard" "Dune Legacy"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_38)    ;  "Welcome to the $(LSTR_81) Setup Wizard" "Dune Legacy"
    ; Push 28u
    ; Push 195u
    ; Push 10u
    ; Push 120u
    ; Push 0x00000020
    ; Push 0x40000000|0x10000000|0x04000000|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_15_
  SetCtlColors $_15_ "" 0xFFFFFF
  CreateFont $_16_ $(LSTR_39) 12 700    ;  "MS Shell Dlg"
  SendMessage $_15_ ${WM_SETFONT} $_16_ 0
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x00000100 0x00000020 120u 45u 195u 130u $(LSTR_40)    ;  "This wizard will guide you through the installation of $(LSTR_81).$\r$\n$\r$\nIt is recommended that you close all other applications before starting Setup. This will make it possible to update relevant system files without having to reboot your computer.$\r$\n$\r$\n$_CLICK" "Dune Legacy"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_40)    ;  "This wizard will guide you through the installation of $(LSTR_81).$\r$\n$\r$\nIt is recommended that you close all other applications before starting Setup. This will make it possible to update relevant system files without having to reboot your computer.$\r$\n$\r$\n$_CLICK" "Dune Legacy"
    ; Push 130u
    ; Push 195u
    ; Push 45u
    ; Push 120u
    ; Push 0x00000020
    ; Push 0x40000000|0x10000000|0x04000000|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_17_
  SetCtlColors $_17_ "" 0xFFFFFF
  Call func_4
  nsDialogs::Show
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll Show
  Call func_14
  IntCmp $_14_ 0 label_138
  System::Call gdi32::DeleteObject(is) $_14_
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $_14_
    ; Push gdi32::DeleteObject(is)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_138:
FunctionEnd


Function func_139    ; Page 0, Leave
FunctionEnd


Function func_140    ; Page 1, Pre
  SendMessage $_0_ ${WM_SETTEXT} 0 STR:$(LSTR_42)    ;  "License Agreement"
  SendMessage $_2_ ${WM_SETTEXT} 0 STR:$(LSTR_43)    ;  "Please review the license terms before installing $(LSTR_81)." "Dune Legacy"
FunctionEnd


Function func_143    ; Page 1, Show
  FindWindow $_18_ "#32770" "" $HWNDPARENT
  GetDlgItem $_19_ $_18_ 1040
  GetDlgItem $_20_ $_18_ 1006
  GetDlgItem $_21_ $_18_ 1000
  SendMessage $_19_ ${WM_SETTEXT} 0 STR:$(LSTR_44)    ;  "Press Page Down to see the rest of the agreement."
FunctionEnd


Function func_149    ; Page 1, Leave
FunctionEnd


Function func_150    ; Page 2, Pre
  SendMessage $_0_ ${WM_SETTEXT} 0 STR:$(LSTR_45)    ;  "Choose Install Location"
  SendMessage $_2_ ${WM_SETTEXT} 0 STR:$(LSTR_46)    ;  "Choose the folder in which to install $(LSTR_81)." "Dune Legacy"
FunctionEnd


Function func_153    ; Page 2, Show
  FindWindow $_22_ "#32770" "" $HWNDPARENT
  GetDlgItem $_23_ $_22_ 1006
  GetDlgItem $_24_ $_22_ 1020
  GetDlgItem $_25_ $_22_ 1019
  GetDlgItem $_26_ $_22_ 1001
  GetDlgItem $_27_ $_22_ 1023
  GetDlgItem $_28_ $_22_ 1024
FunctionEnd


Function func_161    ; Page 2, Leave
FunctionEnd


Function func_162    ; Page 3, Pre
  Call func_436
  SendMessage $_0_ ${WM_SETTEXT} 0 STR:$(LSTR_49)    ;  "Dune II Pak-Files"
  SendMessage $_2_ ${WM_SETTEXT} 0 STR:$(LSTR_50)    ;  "Choose the directory where the installer can copy the Dune II Pak-Files from."
FunctionEnd


Function func_166    ; Page 3, Show
  FindWindow $_22_ "#32770" "" $HWNDPARENT
  GetDlgItem $_23_ $_22_ 1006
  GetDlgItem $_24_ $_22_ 1020
  GetDlgItem $_25_ $_22_ 1019
  GetDlgItem $_26_ $_22_ 1001
  GetDlgItem $_27_ $_22_ 1023
  GetDlgItem $_28_ $_22_ 1024
FunctionEnd


Function func_174    ; Page 3, Leave
  Call func_451
FunctionEnd


Function func_176    ; Page 4, Pre
  SendMessage $_0_ ${WM_SETTEXT} 0 STR:$(LSTR_51)    ;  Installing
  SendMessage $_2_ ${WM_SETTEXT} 0 STR:$(LSTR_52)    ;  "Please wait while $(LSTR_81) is being installed." "Dune Legacy"
FunctionEnd


Function func_179    ; Page 4, Show
  FindWindow $_30_ "#32770" "" $HWNDPARENT
  GetDlgItem $_31_ $_30_ 1006
  GetDlgItem $_32_ $_30_ 1004
  GetDlgItem $_33_ $_30_ 1027
  GetDlgItem $_34_ $_30_ 1016
FunctionEnd


Function func_185    ; Page 4, Leave
  IfAbort label_189
  SendMessage $_0_ ${WM_SETTEXT} 0 STR:$(LSTR_53)    ;  "Installation Complete"
  SendMessage $_2_ ${WM_SETTEXT} 0 STR:$(LSTR_54)    ;  "Setup was completed successfully."
  Goto label_191
label_189:
  SendMessage $_0_ ${WM_SETTEXT} 0 STR:$(LSTR_55)    ;  "Installation Aborted"
  SendMessage $_2_ ${WM_SETTEXT} 0 STR:$(LSTR_56)    ;  "Setup was not completed successfully."
label_191:
  IfAbort label_192
label_192:
FunctionEnd


Function func_193
  InitPluginsDir
    ; Call Initialize_____Plugins
    ; SetDetailsPrint lastused
  SetOverwrite on
  AllowSkipFiles on
  File $PLUGINSDIR\modern-wizard.bmp
  Call func_0
  SetAutoClose true
FunctionEnd


Function func_199    ; Page 6, Pre
  SendMessage $_9_ ${WM_SETTEXT} 0 STR:$(LSTR_57)    ;  &Finish
  nsDialogs::Create 1044
    ; Call Initialize_____Plugins
    ; SetOverwrite off
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push 1044
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll Create
  Pop $_35_
  nsDialogs::SetRTL $(LSTR_37)    ;  0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_37)    ;  0
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll SetRTL
  SetCtlColors $_35_ "" 0xFFFFFF
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x0000000E|0x00000100 0 0u 0u 109u 193u ""
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push ""
    ; Push 193u
    ; Push 109u
    ; Push 0u
    ; Push 0u
    ; Push 0
    ; Push 0x40000000|0x10000000|0x04000000|0x0000000E|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_36_
  Push $0
  Push $1
  Push $2
  Push $R0
  StrCpy $R0 $_36_
  StrCpy $1 ""
  StrCpy $2 ""
  System::Call "*(i, i, i, i) i.s"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "*(i, i, i, i) i.s"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $0
  IntCmp $0 0 label_256
  System::Call "user32::GetClientRect(iR0, ir0)"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "user32::GetClientRect(iR0, ir0)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call "*$0(i, i, i .s, i .s)"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "*$0(i, i, i .s, i .s)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Free $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; CallInstDLL $PLUGINSDIR\System.dll Free
  Pop $1
  Pop $2
label_256:
  System::Call "user32::LoadImage(i0, ts, i 0, ir1, ir2, i0x0010) i.s" $PLUGINSDIR\modern-wizard.bmp
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $PLUGINSDIR\modern-wizard.bmp
    ; Push "user32::LoadImage(i0, ts, i 0, ir1, ir2, i0x0010) i.s"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $0
  SendMessage $R0 0x0172 0 $0
  Pop $R0
  Pop $2
  Pop $1
  Exch $0
    ; Push $0
    ; Exch
    ; Pop $0
  Pop $_37_
  IfRebootFlag 0 label_337
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x00000100 0x00000020 120u 10u 195u 28u $(LSTR_58)    ;  "Completing the $(LSTR_81) Setup Wizard" "Dune Legacy"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_58)    ;  "Completing the $(LSTR_81) Setup Wizard" "Dune Legacy"
    ; Push 28u
    ; Push 195u
    ; Push 10u
    ; Push 120u
    ; Push 0x00000020
    ; Push 0x40000000|0x10000000|0x04000000|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_38_
  SetCtlColors $_38_ "" 0xFFFFFF
  CreateFont $_39_ $(LSTR_39) 12 700    ;  "MS Shell Dlg"
  SendMessage $_38_ ${WM_SETFONT} $_39_ 0
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x00000100 0x00000020 120u 45u 195u 40u $(LSTR_59)    ;  "Your computer must be restarted in order to complete the installation of $(LSTR_81). Do you want to reboot now?" "Dune Legacy"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_59)    ;  "Your computer must be restarted in order to complete the installation of $(LSTR_81). Do you want to reboot now?" "Dune Legacy"
    ; Push 40u
    ; Push 195u
    ; Push 45u
    ; Push 120u
    ; Push 0x00000020
    ; Push 0x40000000|0x10000000|0x04000000|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_40_
  SetCtlColors $_40_ "" 0xFFFFFF
  nsDialogs::CreateControl BUTTON 0x40000000|0x10000000|0x04000000|0x00010000|0x00000000|0x00000C00|0x00000009|0x00002000 0 120u 90u 195u 10u $(LSTR_60)    ;  "Reboot now"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_60)    ;  "Reboot now"
    ; Push 10u
    ; Push 195u
    ; Push 90u
    ; Push 120u
    ; Push 0
    ; Push 0x40000000|0x10000000|0x04000000|0x00010000|0x00000000|0x00000C00|0x00000009|0x00002000
    ; Push BUTTON
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_42_
  SetCtlColors $_42_ "" 0xFFFFFF
  nsDialogs::CreateControl BUTTON 0x40000000|0x10000000|0x04000000|0x00010000|0x00000000|0x00000C00|0x00000009|0x00002000 0 120u 115u 195u 10u $(LSTR_61)    ;  "I want to manually reboot later"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_61)    ;  "I want to manually reboot later"
    ; Push 10u
    ; Push 195u
    ; Push 115u
    ; Push 120u
    ; Push 0
    ; Push 0x40000000|0x10000000|0x04000000|0x00010000|0x00000000|0x00000C00|0x00000009|0x00002000
    ; Push BUTTON
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_43_
  SetCtlColors $_43_ "" 0xFFFFFF
  SendMessage $_42_ 0x00F1 1 0
  System::Call user32::SetFocus(i$_42_)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push user32::SetFocus(i$_42_)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Goto label_367
label_337:
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x00000100 0x00000020 120u 10u 195u 28u $(LSTR_58)    ;  "Completing the $(LSTR_81) Setup Wizard" "Dune Legacy"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_58)    ;  "Completing the $(LSTR_81) Setup Wizard" "Dune Legacy"
    ; Push 28u
    ; Push 195u
    ; Push 10u
    ; Push 120u
    ; Push 0x00000020
    ; Push 0x40000000|0x10000000|0x04000000|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_38_
  SetCtlColors $_38_ "" 0xFFFFFF
  CreateFont $_39_ $(LSTR_39) 12 700    ;  "MS Shell Dlg"
  SendMessage $_38_ ${WM_SETFONT} $_39_ 0
  nsDialogs::CreateControl STATIC 0x40000000|0x10000000|0x04000000|0x00000100 0x00000020 120u 45u 195u 130u $(LSTR_62)    ;  "$(LSTR_81) has been installed on your computer.$\r$\n$\r$\nClick Finish to close this wizard." "Dune Legacy"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; Push $(LSTR_62)    ;  "$(LSTR_81) has been installed on your computer.$\r$\n$\r$\nClick Finish to close this wizard." "Dune Legacy"
    ; Push 130u
    ; Push 195u
    ; Push 45u
    ; Push 120u
    ; Push 0x00000020
    ; Push 0x40000000|0x10000000|0x04000000|0x00000100
    ; Push STATIC
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll CreateControl
  Pop $_40_
  SetCtlColors $_40_ "" 0xFFFFFF
label_367:
  Call func_4
  nsDialogs::Show
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDialogs.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDialogs.dll Show
  Call func_14
  IntCmp $_37_ 0 label_380
  System::Call gdi32::DeleteObject(is) $_37_
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $_37_
    ; Push gdi32::DeleteObject(is)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_380:
FunctionEnd


Function func_381    ; Page 6, Leave
  IfRebootFlag 0 label_388
  SendMessage $_42_ 0x00F0 0 0 $_41_
  IntCmp $_41_ 1 0 label_387 label_387
  Reboot
    ; Quit
  Goto label_388
label_387:
  Return

label_388:
FunctionEnd


Function .onGUIInit
  GetDlgItem $_0_ $HWNDPARENT 1037
  CreateFont $_1_ $(LSTR_39) $(LSTR_63) 700    ;  "MS Shell Dlg" 8
  SendMessage $_0_ ${WM_SETFONT} $_1_ 0
  GetDlgItem $_2_ $HWNDPARENT 1038
  SetCtlColors $_0_ "" 0xFFFFFF
  SetCtlColors $_2_ "" 0xFFFFFF
  InitPluginsDir
    ; Call Initialize_____Plugins
    ; SetDetailsPrint lastused
  SetOverwrite on
  AllowSkipFiles on
  File $PLUGINSDIR\modern-header.bmp
  SetBrandingImage /IMGID=1046 /RESIZETOFIT $PLUGINSDIR\modern-header.bmp
  GetDlgItem $_3_ $HWNDPARENT 1034
  SetCtlColors $_3_ "" 0xFFFFFF
  GetDlgItem $_4_ $HWNDPARENT 1039
  SetCtlColors $_4_ "" 0xFFFFFF
  GetDlgItem $_6_ $HWNDPARENT 1028
  SetCtlColors $_6_ /BRANDING ""
  GetDlgItem $_5_ $HWNDPARENT 1256
  SetCtlColors $_5_ /BRANDING ""
  SendMessage $_5_ ${WM_SETTEXT} 0 "STR:$(LSTR_0) "    ;  " http://dunelegacy.sourceforge.net"
  GetDlgItem $_7_ $HWNDPARENT 1035
  GetDlgItem $_8_ $HWNDPARENT 1045
  GetDlgItem $_9_ $HWNDPARENT 1
  GetDlgItem $_10_ $HWNDPARENT 2
  GetDlgItem $_11_ $HWNDPARENT 3
  Call func_193
FunctionEnd


Function .onUserAbort
FunctionEnd


Function .onInit
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; SetOverwrite off
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_48_
  StrCmp $_48_ 0 label_435
  System::Call kernel32::Wow64EnableWow64FsRedirection(i0)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::Wow64EnableWow64FsRedirection(i0)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  SetRegView 64
  StrCpy $INSTDIR "$PROGRAMFILES64\Dune Legacy"
label_435:
FunctionEnd


Function func_436
  IfFileExists $INSTDIR\HARK.PAK 0 label_450
  IfFileExists $INSTDIR\ATRE.PAK 0 label_450
  IfFileExists $INSTDIR\ORDOS.PAK 0 label_450
  IfFileExists $INSTDIR\ENGLISH.PAK 0 label_450
  IfFileExists $INSTDIR\DUNE.PAK 0 label_450
  IfFileExists $INSTDIR\SCENARIO.PAK 0 label_450
  IfFileExists $INSTDIR\MENTAT.PAK 0 label_450
  IfFileExists $INSTDIR\VOC.PAK 0 label_450
  IfFileExists $INSTDIR\MERC.PAK 0 label_450
  IfFileExists $INSTDIR\FINALE.PAK 0 label_450
  IfFileExists $INSTDIR\INTRO.PAK 0 label_450
  IfFileExists $INSTDIR\INTROVOC.PAK 0 label_450
  IfFileExists $INSTDIR\SOUND.PAK 0 label_450
  Abort
label_450:
FunctionEnd


Function func_451
  IfFileExists $_29_\HARK.PAK 0 label_483
  IfFileExists $_29_\ATRE.PAK 0 label_483
  IfFileExists $_29_\ORDOS.PAK 0 label_483
  IfFileExists $_29_\ENGLISH.PAK 0 label_483
  IfFileExists $_29_\DUNE.PAK 0 label_483
  IfFileExists $_29_\SCENARIO.PAK 0 label_483
  IfFileExists $_29_\MENTAT.PAK 0 label_483
  IfFileExists $_29_\VOC.PAK 0 label_483
  IfFileExists $_29_\MERC.PAK 0 label_483
  IfFileExists $_29_\FINALE.PAK 0 label_483
  IfFileExists $_29_\INTRO.PAK 0 label_483
  IfFileExists $_29_\INTROVOC.PAK 0 label_483
  IfFileExists $_29_\SOUND.PAK 0 label_483
  CreateDirectory $INSTDIR
  CopyFiles /SILENT /FILESONLY $_29_\HARK.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\ATRE.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\ORDOS.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\ENGLISH.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\DUNE.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\SCENARIO.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\MENTAT.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\VOC.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\MERC.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\FINALE.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\INTRO.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\INTROVOC.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  CopyFiles /SILENT /FILESONLY $_29_\SOUND.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
  IfFileExists $_29_\GERMAN.PAK 0 label_480
  CopyFiles /SILENT /FILESONLY $_29_\GERMAN.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
label_480:
  IfFileExists $_29_\FRENCH.PAK 0 label_482
  CopyFiles /SILENT /FILESONLY $_29_\FRENCH.PAK $INSTDIR    ; $(LSTR_7)$INSTDIR    ;  "Copy to "
label_482:
  Return

label_483:
  MessageBox MB_OK "Cannot find the needed PAK-Files in $\"$_29_$\""
  Abort
FunctionEnd


Section ; Section_0
  ; AddSize 21897
  SetOutPath $INSTDIR\maps\singleplayer
  SetOverwrite on
  AllowSkipFiles on
  MessageBox MB_OK "OUTDIR $OUTDIR"

  /*

  File "2P - 32x128 - Canyon.ini"
  File "2P - 64x64 - Duality.ini"
  File "2P - 64x64 - North vs. South.ini"
  File "2P - 64x64 - Twin Fists.ini"
  File "3P - 64x32 - Middle Man.ini"
  File "4P - 64x64 - 3 vs 1.ini"
  File "5P - 128x128 - All against Atreides.ini"
  File "5P - 128x128 - Sardaukar Base Easy.ini"
  File "5P - 128x128 - Sardaukar Base.ini"
  SetOutPath $INSTDIR\maps\multiplayer
  File "2P - 32x128 - Gatekeeper.ini"
  File "2P - 32x32 - X-Factor.ini"
  File "2P - 64x32 - Cliffs Of Rene.ini"
  File "2P - 64x64 - Bottle Neck.ini"
  File "2P - 64x64 - Broken Mountains.ini"
  File "2P - 64x64 - David's Pass.ini"
  File "2P - 64x64 - Face Off.ini"
  File "2P - 64x64 - Great Divide.ini"
  File "2P - 64x64 - Sanctuarys.ini"
  File "4P - 128x128 - Deserted.ini"
  File "4P - 128x128 - Equilibrium.ini"
  File "4P - 128x128 - Four Cities.ini"
  File "4P - 128x128 - Hungry Hippos.ini"
  File "4P - 128x128 - Silicon Valley XL.ini"
  File "4P - 128x128 - Snake Pass.ini"
  File "4P - 128x128 - Spicestorm.ini"
  File "4P - 128x128 - The Sardaukar Outpost.ini"
  File "4P - 128x128 - Worm Investation.ini"
  File "4P - 128x128 - Wormhole.ini"
  File "4P - 128x64 - Gamma Sector.ini"
  File "4P - 64x64 - Channels.ini"
  File "4P - 64x64 - Clear Path.ini"
  File "4P - 64x64 - Combed.ini"
  File "4P - 64x64 - Four Chambers.ini"
  File "4P - 64x64 - Four Courners.ini"
  File "4P - 64x64 - Sietch Stefan.ini"
  File "4P - 64x64 - Silicon Valley.ini"
  File "4P - 64x64 - Stronghold.ini"
  File "4P - 64x64 - Vast Armies Have Arrived.ini"
  File "5P - 128x128 - Fortress.ini"
  File "5P - 128x128 - Gridlocked.ini"
  File "5P - 128x128 - Hellvetika.ini"
  File "5P - 128x128 - Kragetam.ini"
  File "5P - 128x128 - Meadow.ini"
  File "5P - 128x64 - Watch Your Track.ini"
  File "6P - 128x128 - Gargantuan Mountains.ini"
  File "6P - 64x128 - Rocking Fields.ini"
  File "6P - 64x64 - Fertile Basin.ini"
  SetOutPath $INSTDIR\locale
  File English.en.po
  File French.fr.po
  File German.de.po
  File Spanish.es.po
  SetOutPath $INSTDIR
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; SetOverwrite off
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_48_
  StrCmp $_48_ 0 label_563
  SetOverwrite on
  AllowSkipFiles on
  File dunelegacy.exe
  File SDL2.dll
  File SDL2_mixer.dll
  File libogg-0.dll
  File libvorbis-0.dll
  File libvorbisfile-3.dll
  File libmodplug-1.dll
  File libFLAC-8.dll
  File smpeg2.dll
  Goto label_572
label_563:
  File dunelegacy.exe
  File SDL2.dll
  File SDL2_mixer.dll
  File libogg-0.dll
  File libvorbis-0.dll
  File libvorbisfile-3.dll
  File libmodplug-1.dll
  File libFLAC-8.dll
  File smpeg2.dll
label_572:
  File LEGACY.PAK
  File OPENSD2.PAK
  File GFXHD.PAK
  File Dune2-Versions.txt
  File COPYING
  Push $INSTDIR\COPYING
  Push $INSTDIR\License.txt
  Call func_603
  File AUTHORS
  Push $INSTDIR\AUTHORS
  Push $INSTDIR\Authors.txt
  Call func_603
  File README
  Push $INSTDIR\README
  Push $INSTDIR\Readme.txt



  Call func_603
  WriteUninstaller $INSTDIR\uninstall.exe ;  $INSTDIR\$INSTDIR\uninstall.exe    ; !!! ERROR: SKIP possible BadCmd
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dune Legacy" DisplayName "Dune Legacy"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dune Legacy" UninstallString $\"$INSTDIR\uninstall.exe$\"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dune Legacy" DisplayIcon $\"$INSTDIR\dunelegacy.exe$\",0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dune Legacy" NoModify 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dune Legacy" NoRepair 1
*/

SectionEnd


Section "Start Menu Shortcuts" ; Section_1
  CreateDirectory "$SMPROGRAMS\Dune Legacy"
  CreateShortCut "$SMPROGRAMS\Dune Legacy\Dune Legacy.lnk" $INSTDIR\dunelegacy.exe "" $INSTDIR\dunelegacy.exe
  CreateShortCut "$SMPROGRAMS\Dune Legacy\Readme.lnk" $INSTDIR\Readme.txt
  CreateShortCut "$SMPROGRAMS\Dune Legacy\License.lnk" $INSTDIR\License.txt
  WriteINIStr "$INSTDIR\Dune Legacy Website.URL" InternetShortcut URL http://dunelegacy.sourceforge.net/
  CreateShortCut "$SMPROGRAMS\Dune Legacy\Dune Legacy Website.lnk" "$INSTDIR\Dune Legacy Website.URL"
  CreateShortCut "$SMPROGRAMS\Dune Legacy\$(LSTR_64).lnk" $INSTDIR\uninstall.exe "" $INSTDIR\uninstall.exe    ;  "Uninstall Dune Legacy"
SectionEnd


Function func_603
  ClearErrors
  Pop $2
  FileOpen $1 $2 w
  Pop $2
  FileOpen $0 $2 r
  Push $2
  IfErrors label_617
label_610:
  FileReadByte $0 $2
  IfErrors label_617
  StrCmp $2 13 label_610
  StrCmp $2 10 label_614 label_615
label_614:
  FileWriteByte $1 13
label_615:
  FileWriteByte $1 $2
  Goto label_610
label_617:
  FileClose $0
  FileClose $1
  Pop $0
  Delete $0
FunctionEnd


/*
Function Initialize_____Plugins
  SetDetailsPrint none
  StrCmp $PLUGINSDIR "" 0 label_632
  Push $0
  SetErrors
  GetTempFileName $0
  Delete $0
  CreateDirectory $0 ; !!!! Unknown Params:  $0 "" ProgramFilesDir   ; 274 0 1
  IfErrors label_633
  StrCpy $PLUGINSDIR $0
  Pop $0
label_632:
  Return

label_633:
  MessageBox MB_OK|MB_ICONSTOP "Error! Can't initialize plug-ins directory. Please try again later." /SD IDOK
  Quit
FunctionEnd
*/



; --------------------
; UNREFERENCED STRINGS:

/*
38 CommonFilesDir
53 "$PROGRAMFILES\Common Files"
70 $COMMONFILES
*/