# Find CppUnit package
find_package(CppUnit REQUIRED)

# Test executable
add_executable(dunelegacy_tests)

# Source files
target_sources(dunelegacy_tests
    PRIVATE
        testmain.cpp
        FileSystemTestCase/FileSystemTestCase.cpp
        INIFileTestCase/INIFileTestCase1.cpp
        INIFileTestCase/INIFileTestCase2.cpp
        INIFileTestCase/INIFileTestCase3.cpp
)

# Include directories
target_include_directories(dunelegacy_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(dunelegacy_tests
    PRIVATE
        CppUnit::CppUnit
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
)

if(WIN32)
    target_link_libraries(dunelegacy_tests
        PRIVATE
            ws2_32
            winmm
            imm32
            version
    )
endif()

# Compiler options
target_compile_options(dunelegacy_tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# Add test
add_test(NAME dunelegacy_tests COMMAND dunelegacy_tests)

# Set test properties
set_tests_properties(dunelegacy_tests PROPERTIES
    ENVIRONMENT "DUNELEGACY_DATADIR=${CMAKE_SOURCE_DIR}/data"
)

# Copy test data to build directory
add_custom_command(
    TARGET dunelegacy_tests
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "${CMAKE_BINARY_DIR}/tests/data"
    COMMENT "Copying test data to build directory"
) 