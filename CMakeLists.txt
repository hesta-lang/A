cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(DiscordRichPresence VERSION 4.1.2)

file(GLOB_RECURSE SOURCES 
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src/)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# The original discord-presence dependency is a desktop IPC client and the
# upstream repository is currently unavailable. Android builds use a safe
# compile-only backend until the Discord Social SDK Android transport is integrated.
if (ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DRPC_ANDROID_NO_PRESENCE=1)
else()
    CPMAddPackage("gh:EclipseMenu/discord-presence#3b50b08")
    target_link_libraries(${PROJECT_NAME} discord-rpc)
endif()

setup_geode_mod(${PROJECT_NAME} EXTERNALS ninxout.champions:1.0.0-alpha.1)
