# Android ARM64 port status

## What this package does

- Keeps DiscordRPC2's Geode hooks and Geometry Dash activity collection code.
- Removes the unavailable `EclipseMenu/discord-presence` dependency from Android builds, so Android builds no longer fail just trying to fetch that desktop-oriented dependency.
- Uses a compile-only Android backend. It stores the most recent `RPCOptions` and logs a clear warning; it does **not** publish Rich Presence yet.
- Leaves the original desktop RPC backend intact for non-Android builds.

## Why the mobile status is not implemented yet

The original `discord-presence` / legacy `discord-rpc` transport targets a local Discord desktop client. It cannot simply connect to the Android Discord app. Discord's current documentation now describes Rich Presence over an unauthenticated RPC transport on Android via Discord Social SDK 1.10+, requiring the SDK, a registered Discord application ID, and a compatible Android integration. This source package does not include that SDK or a verified Geode-to-SDK bridge, so pretending the old IPC client works would be incorrect.

## Next implementation step

Integrate the official Discord Social SDK Android RPC transport behind a separate Android-only adapter, confirm the SDK's distribution/licensing and ABI requirements, then map `RPCOptions` fields to its `Activity` API. Keep all state collection in `Observer.cpp`; only replace the transport in `RPCManager.cpp`.

## Build

The GitHub Actions workflow builds for Geode `Android64`. The resulting `.geode` is a test package, not a functional mobile Rich Presence release until the SDK adapter is implemented and tested on-device.
