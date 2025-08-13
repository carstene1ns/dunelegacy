cmake_minimum_required(VERSION 3.15)

# Project name and version
project(DuneLegacy VERSION 0.98.4 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include cmake modules
include(CMakeDependentOption)
include(CTest)
include(GNUInstallDirs)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Release build optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(APPLE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum OS X deployment version" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -ffast-math -march=native -mtune=native")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -flto -ffast-math -march=native -mtune=native")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
    endif()
endif()

# Check for modern C++ support
if(MSVC)
    # Use C++17 consistently
    set(CMAKE_CXX_STANDARD 17)
else()
    # Check if compiler supports C++17
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
    
    if(COMPILER_SUPPORTS_CXX17)
        set(CMAKE_CXX_STANDARD 17)
    else()
        set(CMAKE_CXX_STANDARD 14)
        message(STATUS "C++17 not supported, falling back to C++14")
    endif()
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(DUNELEGACY_BUILD_TESTS "Build test suite" OFF)
option(DUNELEGACY_ENABLE_PCH "Enable precompiled headers" ON)
option(DUNELEGACY_STATIC_RUNTIME "Use static runtime libraries" OFF)

# Check for required packages with specific versions
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2>=2.32.4)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer>=2.8.1)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf>=2.24.0)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    
    # Check for Visual Studio
    if(MSVC)
        # Set Visual Studio specific options
        set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0")
        set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE "x64")
        
        # Runtime library settings
        if(DUNELEGACY_STATIC_RUNTIME)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
        
        # Precompiled header settings
        if(DUNELEGACY_ENABLE_PCH)
            set(PCH_SOURCE "${CMAKE_SOURCE_DIR}/IDE/VC/stdafx.cpp")
            set(PCH_HEADER "${CMAKE_SOURCE_DIR}/IDE/VC/stdafx.h")
        endif()
    endif()
else()
    # Unix-like systems
    find_package(Threads REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Global compile options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Add subdirectories
add_subdirectory(src)
if(DUNELEGACY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Create a custom target for copying data files
add_custom_target(copy_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data/"
    "${CMAKE_BINARY_DIR}/bin/"
    COMMENT "Copying data files directly to bin directory"
)

# Add dependency to ensure data files are copied after main target is built
add_dependencies(copy_data dunelegacy)

# Windows-specific file operations
if(WIN32)
    # Copy all .PAK files
    file(GLOB PAK_FILES "${CMAKE_SOURCE_DIR}/data/*.PAK")
    foreach(PAK_FILE ${PAK_FILES})
        get_filename_component(PAK_FILENAME ${PAK_FILE} NAME)
        file(COPY "${PAK_FILE}" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
    endforeach()
    
    # SDL2 DLL handling
    find_program(SDL2_DLL_PATH "SDL2.dll" 
        HINTS ${SDL2_LIBRARY_DIRS} ${SDL2_LIBDIR} 
        PATH_SUFFIXES bin
        DOC "Path to SDL2.dll")
    find_program(SDL2_MIXER_DLL_PATH "SDL2_mixer.dll"
        HINTS ${SDL2_MIXER_LIBRARY_DIRS} ${SDL2_MIXER_LIBDIR}
        PATH_SUFFIXES bin
        DOC "Path to SDL2_mixer.dll")
    find_program(SDL2_TTF_DLL_PATH "SDL2_ttf.dll"
        HINTS ${SDL2_TTF_LIBRARY_DIRS} ${SDL2_TTF_LIBDIR}
        PATH_SUFFIXES bin
        DOC "Path to SDL2_ttf.dll")
        
    # Copy SDL2 DLLs (either from found locations or fallback)
    # SDL2.dll
    if(SDL2_DLL_PATH)
        file(COPY "${SDL2_DLL_PATH}" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
        message(STATUS "Copying SDL2.dll from ${SDL2_DLL_PATH}")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/bin/Release-x64/SDL2.dll")
        file(COPY "${CMAKE_SOURCE_DIR}/bin/Release-x64/SDL2.dll" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
        message(STATUS "Copying SDL2.dll from fallback location")
    else()
        message(WARNING "SDL2.dll not found. Please copy it manually to the build/bin directory.")
    endif()
    
    # SDL2_mixer.dll
    if(SDL2_MIXER_DLL_PATH)
        file(COPY "${SDL2_MIXER_DLL_PATH}" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
        message(STATUS "Copying SDL2_mixer.dll from ${SDL2_MIXER_DLL_PATH}")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/bin/Release-x64/SDL2_mixer.dll")
        file(COPY "${CMAKE_SOURCE_DIR}/bin/Release-x64/SDL2_mixer.dll" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
        message(STATUS "Copying SDL2_mixer.dll from fallback location")
    else()
        message(WARNING "SDL2_mixer.dll not found. Please copy it manually to the build/bin directory.")
    endif()
    
    # SDL2_ttf.dll
    if(SDL2_TTF_DLL_PATH)
        file(COPY "${SDL2_TTF_DLL_PATH}" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
        message(STATUS "Copying SDL2_ttf.dll from ${SDL2_TTF_DLL_PATH}")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/bin/Release-x64/SDL2_ttf.dll")
        file(COPY "${CMAKE_SOURCE_DIR}/bin/Release-x64/SDL2_ttf.dll" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
        message(STATUS "Copying SDL2_ttf.dll from fallback location")
    else()
        message(WARNING "SDL2_ttf.dll not found. Please copy it manually to the build/bin directory.")
    endif()
endif()

# Install data files
install(DIRECTORY data/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
    PATTERN "*.svn" EXCLUDE
)

# Install desktop file and icons on Linux
if(UNIX AND NOT APPLE)
    install(FILES dunelegacy.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    install(FILES dunelegacy.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps
    )
    install(FILES dunelegacy-128x128.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps
        RENAME dunelegacy.png
    )
    install(FILES dunelegacy.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    )
endif()

# Install documentation
install(FILES AUTHORS COPYING NEWS README
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Dune Legacy Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/dunelegacy.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/dunelegacy.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${PROJECT_NAME}.exe")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_ICON_FILE "dunelegacy.icns")
else()
    set(CPACK_GENERATOR "DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0, libsdl2-mixer-2.0-0, libsdl2-ttf-2.0-0")
    set(CPACK_RPM_PACKAGE_LICENSE "GPL-2.0+")
    set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
endif()

include(CPack) 