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
# Keep this disabled by default while diagnosing the native SDK crash.
# Enable explicitly with -DDRPC_ENABLE_ANDROID_SOCIAL_SDK=ON only after
# the launcher packaging and SDK initialization have been verified.
if (ANDROID)
    option(DRPC_ENABLE_ANDROID_SOCIAL_SDK "Enable Discord Social SDK for Android" OFF)

    option(DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY "Only test Discord SDK client initialization; skip callbacks and presence" OFF)

    if (DRPC_ENABLE_ANDROID_SOCIAL_SDK)
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
            set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_LIBRARIES discord_partner_sdk)
            target_compile_definitions(${PROJECT_NAME} PRIVATE DRPC_ANDROID_SOCIAL_SDK=1)
            if (DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY)
                target_compile_definitions(${PROJECT_NAME} PRIVATE DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY=1)
                message(WARNING "Diagnostic mode: Discord client init only; callbacks and presence are disabled.")
            endif()
            message(STATUS "Discord Social SDK enabled for Android ABI: ${ANDROID_ABI}")
        else()
            target_compile_definitions(${PROJECT_NAME} PRIVATE DRPC_ANDROID_NO_PRESENCE=1)
            message(WARNING "Discord Social SDK files not found; Android Rich Presence remains disabled. Expected: ${DISCORD_SOCIAL_SDK_ROOT}")
        endif()
    else()
        target_compile_definitions(${PROJECT_NAME} PRIVATE DRPC_ANDROID_NO_PRESENCE=1)
        message(WARNING "Android Discord Social SDK temporarily disabled for crash isolation; Rich Presence is off.")
    endif()
else()
    CPMAddPackage("gh:EclipseMenu/discord-presence#3b50b08")
    target_link_libraries(${PROJECT_NAME} discord-rpc)
endif()

setup_geode_mod(${PROJECT_NAME} EXTERNALS ninxout.champions:1.0.0-alpha.1)
