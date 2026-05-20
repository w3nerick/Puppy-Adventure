cmake_minimum_required(VERSION 3.16)
project(CatGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# raylib se descarga y compila automáticamente con FetchContent
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
set(SUPPORT_X11    ON  CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(catgame
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Level.cpp
)

target_include_directories(catgame PRIVATE src)
target_link_libraries(catgame PRIVATE raylib)

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
