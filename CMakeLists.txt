cmake_minimum_required(VERSION 3.16)
project(CatGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Cuando se compila con emcmake / emscripten, le decimos a raylib que use el backend Web.
if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "raylib platform" FORCE)
endif()

# raylib se descarga y compila automaticamente con FetchContent
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW ON
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
if(NOT EMSCRIPTEN)
    set(SUPPORT_X11 ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(raylib)

add_executable(catgame
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Level.cpp
    src/PowerUp.cpp
    src/Bark.cpp
    src/Particle.cpp
)

target_include_directories(catgame PRIVATE src)
target_link_libraries(catgame PRIVATE raylib)

if(EMSCRIPTEN)
    # Genera index.html (+ index.js + index.wasm) directamente.
    set_target_properties(catgame PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
    )
    target_link_options(catgame PRIVATE
        "-sUSE_GLFW=3"
        "-sASYNCIFY"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=67108864"
        "-sFORCE_FILESYSTEM=1"
        "-sEXPORTED_RUNTIME_METHODS=['HEAPF32']"
        "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
    )
    # Optimizacion para reducir el tamanio del .wasm
    target_compile_options(catgame PRIVATE -Os)
endif()

if(WIN32)
    target_link_libraries(catgame PRIVATE winmm)
endif()

if(APPLE)
    target_link_libraries(catgame PRIVATE
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()
