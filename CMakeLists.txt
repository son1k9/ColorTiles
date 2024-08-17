cmake_minimum_required (VERSION 3.8)
project ("ColorTiles")

# Dependencies
include(FetchContent)

set(RAYLIB_VERSION 5.0)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS ${RAYLIB_VERSION} EXACT
)
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(raylib)

add_executable (ColorTiles  "src/main.cpp")
target_sources(ColorTiles PRIVATE 
    src/gameplay_screen.cpp
    src/main.cpp
    src/main_menu_screen.cpp
    src/score.cpp
    src/scores_screen.cpp
    src/screen.cpp
    src/settings.cpp
    src/settings_screen.cpp
)
target_include_directories(ColorTiles PRIVATE src)
target_link_libraries(${PROJECT_NAME} raylib)

#Removes console
if (WIN32)
    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
    set(SUBSYSTEM_LINKER_OPTIONS /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
else()
    set(SUBSYSTEM_LINKER_OPTIONS -mwindows)
endif()

target_link_options(ColorTiles PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

set_property(TARGET ColorTiles PROPERTY CXX_STANDARD 20)

file(COPY assets/Skins DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
