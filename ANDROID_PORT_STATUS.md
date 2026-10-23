# Discord Rich Presence — preparación de port Android arm64

Este árbol parte del repositorio fuente que proporcionó el usuario y añade un workflow de compilación Geode `android64` y detección de plataforma Android en `main.cpp`.

## Estado real

**Esto todavía no es un port funcional de Rich Presence para Android.** No se ha podido verificar una compilación en este entorno y la implementación de RPC actual usa `EclipseMenu/discord-presence` (`discord-rpc`), una implementación heredada diseñada alrededor del transporte RPC de cliente de escritorio. Que el mod compile para Android no demuestra que pueda publicar presencia en la app móvil de Discord.

## Bloqueador de Rich Presence

Para lograr presencia real en Android hay que reemplazar el backend de `RPCManager` por un transporte soportado en Android. La documentación actual de Discord indica que Social SDK 1.10+ admite Rich Presence en Android mediante RPC no autenticado, pero ese SDK es una dependencia nativa separada y debe descargarse desde el Developer Portal, verificarse y enlazarse para `arm64-v8a`. No se incluye aquí ningún binario de Discord ni credencial.

El código de `Observer`/`RPCOptions` conserva la lógica que calcula el estado de Geometry Dash; la capa que publica esos datos en Discord necesita adaptación al modelo `discordpp::Client` / `Activity` del Social SDK. No debe darse por hecho que `discord-rpc` antiguo funcione en Android.

## Compilación tentativa

El workflow `.github/workflows/android64.yml` intenta compilar el árbol con `target: android64`. Si la dependencia `discord-presence` no compila para ese target, el workflow fallará hasta que se reemplace o condicione esa dependencia. Si compila, eso solo valida el binario Geode, no la entrega de presencia a Discord Android.

## Referencias

- Geode SDK: https://github.com/geode-sdk
- Geode docs: https://docs.geode-sdk.org/
- Discord Social SDK docs: https://discord.com/developers/docs/social-sdk/index.html
- Discord Social SDK installation: https://discord.com/developers/docs/social-sdk/installation.html
- Discord Social SDK release notes (Android RPC support in 1.10): https://discord.com/developers/docs/social-sdk/release_notes.html
