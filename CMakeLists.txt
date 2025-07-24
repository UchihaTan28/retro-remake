cmake_minimum_required(VERSION 3.10)
project(KungFu-Arcade)

set(CMAKE_CXX_STANDARD 17)

# Enable static linking
set(raylib_STATIC ON)
add_definitions(-DRAYLIB_STATIC)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

# Suppress warning
add_compile_options(-Wno-stringop-overflow)

# Raylib paths
set(RAYLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/raylib/src")
set(RAYLIB_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/raylib/build/raylib")

include_directories(${RAYLIB_INCLUDE_DIR})
link_directories(${RAYLIB_LIBRARY_DIR})

file(GLOB SOURCES "src/*.cpp")

file(COPY "${CMAKE_SOURCE_DIR}/assets"
     DESTINATION "${CMAKE_BINARY_DIR}")

add_executable(kungfu ${SOURCES})
target_link_libraries(kungfu raylib winmm gdi32 opengl32)
