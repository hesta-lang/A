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

# Android Social SDK integration.
# Place the extracted SDK files in external/discord-social-sdk:
#   include/discordpp.h
#   include/cdiscord.h
#   libs/<ANDROID_ABI>/libdiscord_partner_sdk.so
if (ANDROID)
    set(DISCORD_SOCIAL_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/discord-social-sdk"
        CACHE PATH "Path to extracted Discord Social SDK for Android")
    set(DISCORD_SOCIAL_SDK_LIBRARY
        "${DISCORD_SOCIAL_SDK_ROOT}/libs/${ANDROID_ABI}/libdiscord_partner_sdk.so")

    if (EXISTS "${DISCORD_SOCIAL_SDK_ROOT}/include/discordpp.h"
       AND EXISTS "${DISCORD_SOCIAL_SDK_ROOT}/include/cdiscord.h"
       AND EXISTS "${DISCORD_SOCIAL_SDK_LIBRARY}")
        add_library(discord_partner_sdk SHARED IMPORTED)
        set_target_properties(discord_partner_sdk PROPERTIES
            IMPORTED_LOCATION "${DISCORD_SOCIAL_SDK_LIBRARY}")
        target_include_directories(${PROJECT_NAME} PRIVATE
            "${DISCORD_SOCIAL_SDK_ROOT}/include")
        target_link_libraries(${PROJECT_NAME} discord_partner_sdk)
        target_compile_definitions(${PROJECT_NAME} PRIVATE DRPC_ANDROID_SOCIAL_SDK=1)
        message(STATUS "Discord Social SDK enabled for Android ABI: ${ANDROID_ABI}")
    else()
        # Keep builds working until the SDK headers and matching ABI library are
        # actually present in this mod repository.
        target_compile_definitions(${PROJECT_NAME} PRIVATE DRPC_ANDROID_NO_PRESENCE=1)
        message(WARNING "Discord Social SDK files not found; Android Rich Presence remains disabled. Expected: ${DISCORD_SOCIAL_SDK_ROOT}")
    endif()
else()
    CPMAddPackage("gh:EclipseMenu/discord-presence#3b50b08")
    target_link_libraries(${PROJECT_NAME} discord-rpc)
endif()

setup_geode_mod(${PROJECT_NAME} EXTERNALS ninxout.champions:1.0.0-alpha.1)
