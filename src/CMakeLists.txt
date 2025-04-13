# Configure config.h
configure_file(
    ${CMAKE_SOURCE_DIR}/include/config.h.in
    ${CMAKE_BINARY_DIR}/include/config.h
)

# Main executable
add_executable(dunelegacy)

# Enable Win32 subsystem on Windows
if(WIN32)
    if(MSVC)
        # Add Windows icon for MSVC
        target_sources(dunelegacy PRIVATE "${CMAKE_SOURCE_DIR}/dunelegacy.ico")
        # Set application icon properties for MSVC
        set_target_properties(dunelegacy PROPERTIES
            VS_DPI_AWARE "PerMonitor"
            WIN32_EXECUTABLE TRUE)
        
        target_link_options(dunelegacy PRIVATE
            $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
            $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>
        )
    else()
        # For MinGW, add the resource file with the icon
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/dunelegacy.rc"
            "${CMAKE_CURRENT_BINARY_DIR}/dunelegacy.rc"
            COPYONLY
        )
        target_sources(dunelegacy PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/dunelegacy.rc")
        
        # MinGW equivalent for Windows subsystem
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            target_link_options(dunelegacy PRIVATE -mwindows)
        else()
            target_link_options(dunelegacy PRIVATE -mconsole)
        endif()
    endif()
endif()

# Precompiled headers
if(DUNELEGACY_ENABLE_PCH AND WIN32 AND MSVC)
    target_precompile_headers(dunelegacy
        PRIVATE
            ${PCH_HEADER}
    )
    target_sources(dunelegacy
        PRIVATE
            ${PCH_SOURCE}
    )
endif()

# All source files
target_sources(dunelegacy
    PRIVATE
        # Root sources
        AStarSearch.cpp
        Bullet.cpp
        Choam.cpp
        Command.cpp
        CommandManager.cpp
        Explosion.cpp
        Game.cpp
        GameInitSettings.cpp
        GameInterface.cpp
        House.cpp
        Map.cpp
        MapSeed.cpp
        globals.cpp
        main.cpp
        mmath.cpp
        ObjectBase.cpp
        ObjectData.cpp
        ObjectManager.cpp
        ObjectPointer.cpp
        RadarView.cpp
        ScreenBorder.cpp
        sand.cpp
        SoundPlayer.cpp
        Tile.cpp

        # INIMap
        INIMap/INIMapLoader.cpp
        INIMap/INIMapEditorLoader.cpp
        INIMap/INIMapPreviewCreator.cpp

        # FileClasses
        FileClasses/Animation.cpp
        FileClasses/Cpsfile.cpp
        FileClasses/Decode.cpp
        FileClasses/FileManager.cpp
        FileClasses/FontManager.cpp
        FileClasses/GFXManager.cpp
        FileClasses/Icnfile.cpp
        FileClasses/IndexedTextFile.cpp
        FileClasses/INIFile.cpp
        FileClasses/LoadSavePNG.cpp
        FileClasses/lodepng.cpp
        FileClasses/MentatTextFile.cpp
        FileClasses/Pakfile.cpp
        FileClasses/Palfile.cpp
        FileClasses/PictureFactory.cpp
        FileClasses/POFile.cpp
        FileClasses/SFXManager.cpp
        FileClasses/Shpfile.cpp
        FileClasses/TextManager.cpp
        FileClasses/TTFFont.cpp
        FileClasses/Vocfile.cpp
        FileClasses/Wsafile.cpp
        FileClasses/adl/sound_adlib.cpp
        FileClasses/adl/surroundopl.cpp
        FileClasses/adl/woodyopl.cpp
        FileClasses/music/ADLPlayer.cpp
        FileClasses/music/DirectoryPlayer.cpp
        FileClasses/music/XMIPlayer.cpp
        FileClasses/xmidi/xmidi.cpp

        # GUI
        GUI/Button.cpp
        GUI/GUIStyle.cpp
        GUI/Widget.cpp
        GUI/Window.cpp
        GUI/ScrollBar.cpp
        GUI/ListBox.cpp
        GUI/DropDownBox.cpp
        GUI/TextView.cpp
        GUI/RadioButtonManager.cpp
        GUI/dune/ChatManager.cpp
        GUI/dune/DuneStyle.cpp
        GUI/dune/GameOptionsWindow.cpp
        GUI/dune/LoadSaveWindow.cpp
        GUI/dune/InGameMenu.cpp
        GUI/dune/InGameSettingsMenu.cpp
        GUI/dune/NewsTicker.cpp
        GUI/dune/MessageTicker.cpp
        GUI/dune/BuilderList.cpp
        GUI/dune/WaitingForOtherPlayers.cpp

        # Menu
        Menu/MenuBase.cpp
        Menu/MainMenu.cpp
        Menu/SinglePlayerMenu.cpp
        Menu/SinglePlayerSkirmishMenu.cpp
        Menu/CustomGameMenu.cpp
        Menu/CustomGamePlayers.cpp
        Menu/CustomGameStatsMenu.cpp
        Menu/OptionsMenu.cpp
        Menu/AboutMenu.cpp
        Menu/HouseChoiceMenu.cpp
        Menu/MentatMenu.cpp
        Menu/MultiPlayerMenu.cpp
        Menu/BriefingMenu.cpp
        Menu/HouseChoiceInfoMenu.cpp
        Menu/MentatHelp.cpp
        Menu/MapChoice.cpp
        Menu/CampaignStatsMenu.cpp

        # Network
        Network/LANGameFinderAndAnnouncer.cpp
        Network/NetworkManager.cpp
        Network/ENetHttp.cpp
        Network/MetaServerClient.cpp

        # Players
        players/Player.cpp
        players/HumanPlayer.cpp
        players/PlayerFactory.cpp
        players/AIPlayer.cpp
        players/CampaignAIPlayer.cpp
        players/QuantBot.cpp
        players/SmartBot.cpp

        # Structures
        structures/StructureBase.cpp
        structures/BuilderBase.cpp
        structures/TurretBase.cpp
        structures/Barracks.cpp
        structures/ConstructionYard.cpp
        structures/GunTurret.cpp
        structures/HeavyFactory.cpp
        structures/HighTechFactory.cpp
        structures/IX.cpp
        structures/LightFactory.cpp
        structures/Palace.cpp
        structures/Radar.cpp
        structures/Refinery.cpp
        structures/RepairYard.cpp
        structures/RocketTurret.cpp
        structures/Silo.cpp
        structures/StarPort.cpp
        structures/Wall.cpp
        structures/WindTrap.cpp
        structures/WOR.cpp

        # Units
        units/UnitBase.cpp
        units/AirUnit.cpp
        units/GroundUnit.cpp
        units/InfantryBase.cpp
        units/TrackedUnit.cpp
        units/TankBase.cpp
        units/Carryall.cpp
        units/Devastator.cpp
        units/Deviator.cpp
        units/Frigate.cpp
        units/Harvester.cpp
        units/Launcher.cpp
        units/MCV.cpp
        units/Ornithopter.cpp
        units/Quad.cpp
        units/RaiderTrike.cpp
        units/Saboteur.cpp
        units/SandWorm.cpp
        units/SiegeTank.cpp
        units/Soldier.cpp
        units/SonicTank.cpp
        units/Tank.cpp
        units/Trike.cpp
        units/Trooper.cpp

        # CutScenes
        CutScenes/CutScene.cpp
        CutScenes/Scene.cpp
        CutScenes/Intro.cpp
        CutScenes/Meanwhile.cpp
        CutScenes/Finale.cpp
        CutScenes/VideoEvent.cpp
        CutScenes/WSAVideoEvent.cpp
        CutScenes/FadeOutVideoEvent.cpp
        CutScenes/FadeInVideoEvent.cpp
        CutScenes/HoldPictureVideoEvent.cpp
        CutScenes/CrossBlendVideoEvent.cpp
        CutScenes/TextEvent.cpp
        CutScenes/CutSceneTrigger.cpp

        # MapEditor
        MapEditor/ChoamWindow.cpp
        MapEditor/MapEditor.cpp
        MapEditor/MapEditorInterface.cpp
        MapEditor/MapEditorOperation.cpp
        MapEditor/MapGenerator.cpp
        MapEditor/MapEditorRadarView.cpp
        MapEditor/MapSettingsWindow.cpp
        MapEditor/MapMirror.cpp
        MapEditor/NewMapWindow.cpp
        MapEditor/LoadMapWindow.cpp
        MapEditor/PlayerSettingsWindow.cpp
        MapEditor/ReinforcementsWindow.cpp
        MapEditor/TeamsWindow.cpp

        # Trigger
        Trigger/TriggerManager.cpp
        Trigger/ReinforcementTrigger.cpp
        Trigger/TimeoutTrigger.cpp

        # misc
        misc/draw_util.cpp
        misc/FileSystem.cpp
        misc/fnkdat.cpp
        misc/format.cpp
        misc/IFileStream.cpp
        misc/md5.cpp
        misc/OFileStream.cpp
        misc/Random.cpp
        misc/sound_util.cpp
        misc/string_util.cpp
        misc/Scaler.cpp
)

# C source files
target_sources(dunelegacy
    PRIVATE
        # enet
        enet/callbacks.c
        enet/compress.c
        enet/host.c
        enet/list.c
        enet/packet.c
        enet/peer.c
        enet/protocol.c
        enet/unix.c
        enet/win32.c

        # fixmath
        fixmath/fix16.c
        fixmath/fix16_exp.c
        fixmath/fix16_sqrt.c
        fixmath/fix16_str.c
        fixmath/fix16_trig.c
        fixmath/fix32.c
        fixmath/fix32_exp.c
        fixmath/fix32_sqrt.c
        fixmath/fix32_str.c
        fixmath/fix32_trig.c
)

# Source groups for better organization in IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${sources})

# Include directories
target_include_directories(dunelegacy PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(dunelegacy
    PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
)

if(WIN32)
    target_link_libraries(dunelegacy
        PRIVATE
            ws2_32
            winmm
            imm32
            version
    )
endif()

# Compiler options
target_compile_options(dunelegacy
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# Data directory handling
set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")
set(BUILD_DATA_DIR "${CMAKE_BINARY_DIR}/data")

# Copy data directory to build directory
add_custom_command(
    TARGET dunelegacy
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${DATA_DIR}"
        "${BUILD_DATA_DIR}"
    COMMENT "Copying data directory to build directory"
)

# Install data directory
install(
    DIRECTORY ${DATA_DIR}/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dunelegacy
    FILES_MATCHING PATTERN "*.*"
)

# Install executable
install(TARGETS dunelegacy
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

# Define data directory path for runtime
target_compile_definitions(dunelegacy
    PRIVATE
        DATA_DIRECTORY="${CMAKE_INSTALL_DATADIR}/dunelegacy"
) 