#include <Geode/Geode.hpp>
#include <managers/RPCManager.hpp>
#include <managers/Observer.hpp>

using namespace geode::prelude;

std::string getSysName() {
#if defined(GEODE_IS_WINDOWS)
    return "Windows";
#elif defined(GEODE_IS_ANDROID)
    return "Android";
#elif defined(GEODE_IS_IOS)
    return "iOS";
#else
    return "macOS";
#endif
}

$on_mod(Loaded) {
    auto& rpcManager = RPCManager::get();
    rpcManager.initRPC();
    rpcManager.defaultState = "Browsing menus";
    rpcManager.defaultLargeImage = "gd-large";
    rpcManager.defaultLargeImageText = fmt::format("Playing Geometry Dash on {}", getSysName());
    rpcManager.startTime = time(0);

    // start doing things
    Observer::get();
}
