#include "RPCManager.hpp"

#if defined(DRPC_ANDROID_SOCIAL_SDK)
#define DISCORDPP_IMPLEMENTATION
#include <discordpp.h>
#include <memory>
#include <optional>
#include <string>
#elif !defined(DRPC_ANDROID_NO_PRESENCE)
#include <discord-rpc.hpp>
#endif

#include <Geode/loader/Log.hpp>

using namespace geode::log;

#define APPLICATION_ID "1178492879627366472"

#if defined(DRPC_ANDROID_SOCIAL_SDK)
namespace {
    std::shared_ptr<discordpp::Client> s_discordClient;

    // Pump Discord's native event queue from Geode's game-thread scheduler.
    // The scheduler retains the node after scheduleUpdate(), so it stays alive
    // while the mod is loaded.
    class DiscordCallbackPump final : public cocos2d::CCNode {
    public:
        static DiscordCallbackPump* create() {
            auto* node = new DiscordCallbackPump();
            if (node && node->init()) {
                node->autorelease();
                node->scheduleUpdate();
                return node;
            }
            delete node;
            return nullptr;
        }

        void update(float) override {
            discordpp::RunCallbacks();
        }
    };

    DiscordCallbackPump* s_callbackPump = nullptr;

    void startDiscordCallbackPump() {
        if (!s_callbackPump) {
            s_callbackPump = DiscordCallbackPump::create();
            if (!s_callbackPump) {
                geode::log::error("Discord Social SDK: failed to create callback pump.");
            }
        }
    }

    // The Social SDK can dispatch work from native worker threads. Geode/Cocos
    // objects must only be touched from the game thread, so route SDK tasks
    // through Geode's main-thread queue before constructing the Client.
    void configureDiscordSdkThreading() {
        discordpp::SetSynchronizationContext([](std::function<void()> task) {
            geode::queueInMainThread([task = std::move(task)]() mutable {
                task();
            });
        });
    }

    void initializeAndroidDiscordClient() {
        if (s_discordClient) return;
        configureDiscordSdkThreading();
        s_discordClient = std::make_shared<discordpp::Client>();
        s_discordClient->SetApplicationId(std::stoull(APPLICATION_ID));
        geode::log::info("Discord Social SDK: native client constructed.");
#if !defined(DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY)
        startDiscordCallbackPump();
#endif
    }

    void updateAndroidPresence(const RPCOptions& options) {
        initializeAndroidDiscordClient();
#if defined(DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY)
        geode::log::info("Discord Social SDK diagnostic mode: skipping UpdateRichPresence.");
        return;
#endif

        discordpp::Activity activity;
        activity.SetType(discordpp::ActivityTypes::Playing);

        const auto state = options.state.empty() ? RPCManager::get().defaultState : options.state;
        if (!state.empty()) activity.SetState(state);
        if (!options.details.empty()) activity.SetDetails(options.details);

        discordpp::ActivityAssets assets;
        bool hasAssets = false;
        const auto largeImage = options.largeImage.empty()
            ? RPCManager::get().defaultLargeImage : options.largeImage;
        const auto largeText = options.largeImageText.empty()
            ? RPCManager::get().defaultLargeImageText : options.largeImageText;
        if (!largeImage.empty()) {
            assets.SetLargeImage(largeImage);
            hasAssets = true;
        }
        if (!largeText.empty()) {
            assets.SetLargeText(largeText);
            hasAssets = true;
        }
        if (hasAssets) activity.SetAssets(assets);

        if (options.startTimestamp != 0 || options.endTimestamp != 0) {
            discordpp::ActivityTimestamps timestamps;
            if (options.startTimestamp != 0) timestamps.SetStart(options.startTimestamp);
            if (options.endTimestamp != 0) timestamps.SetEnd(options.endTimestamp);
            activity.SetTimestamps(timestamps);
        }

        s_discordClient->UpdateRichPresence(std::move(activity),
            [](discordpp::ClientResult result) {
                if (result.Successful()) {
                    geode::log::info("Discord Social SDK: Rich Presence updated.");
                } else {
                    geode::log::warn("Discord Social SDK: Rich Presence update failed: {}",
                        result.Error());
                }
            });
        // DiscordCallbackPump processes asynchronous SDK callbacks each frame
        // on the game thread; do not call RunCallbacks only once here.
    }
}
#endif

void RPCManager::initRPC() {
    if (hasInitializedRPC) return;
#if defined(DRPC_ANDROID_SOCIAL_SDK)
#if defined(DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY)
    // Diagnostic stage 4: test Client construction and Application ID only.
    // Synchronization-context setup has passed independently. Do not start
    // callbacks or update presence in this stage.
    try {
        configureDiscordSdkThreading();
        s_discordClient = std::make_shared<discordpp::Client>();
        s_discordClient->SetApplicationId(std::stoull(APPLICATION_ID));
        geode::log::info("Discord Social SDK diagnostic: client constructed and application ID set; callbacks/presence disabled.");
    } catch (const std::exception& e) {
        geode::log::error("Discord Social SDK diagnostic: client initialization threw: {}", e.what());
    }
#else
    // Delay native client construction until the first presence update.
    geode::log::info("Discord Social SDK: Android RPC deferred until first presence update.");
#endif
    hasInitializedRPC = true;
#elif defined(DRPC_ANDROID_NO_PRESENCE)
    geode::log::warn(
        "Discord Rich Presence: SDK files are missing from external/discord-social-sdk; "
        "Android presence is disabled."
    );
    hasInitializedRPC = true;
#else
    discord::RPCManager::get()
        .setClientID(APPLICATION_ID)
        .onReady([](discord::User const& user) {
            geode::log::info("connected to user {}#{} (user id: {})", user.username, user.discriminator, user.id);
        })
        .onDisconnected([](int errcode, std::string_view message) {
            geode::log::error("disconnected with error code {} - message: {}", errcode, message);
        })
        .initialize();
    hasInitializedRPC = true;
#endif
}

void RPCManager::clearRPC() {
#if defined(DRPC_ANDROID_SOCIAL_SDK)
#if defined(DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY)
    currentOptions = RPCOptions{};
    return;
#else
    if (s_discordClient) {
        s_discordClient->ClearRichPresence();
    }
#endif
#elif defined(DRPC_ANDROID_NO_PRESENCE)
    currentOptions = RPCOptions{};
#else
    discord::RPCManager::get().clearPresence();
#endif
}

void RPCManager::setRPC(RPCOptions options) {
    if (options == currentOptions) return;
    currentOptions = options;
#if defined(DRPC_ANDROID_SOCIAL_SDK)
#if defined(DRPC_ANDROID_DIAGNOSTIC_INIT_ONLY)
    geode::log::info("Discord Social SDK diagnostic: presence update skipped.");
    return;
#else
    updateAndroidPresence(currentOptions);
#endif
#elif defined(DRPC_ANDROID_NO_PRESENCE)
    // Presence stays disabled until the extracted SDK is added to this repository.
    return;
#else
    auto& presence = discord::Presence::get();
    presence.clear();

    presence
        .setState(options.state.empty() ? defaultState : options.state)
        .setDetails(options.details)
        .setLargeImageKey(options.largeImage.empty() ? defaultLargeImage : options.largeImage)
        .setLargeImageText(options.largeImage.empty() ? defaultLargeImageText : options.largeImageText)
        .setSmallImageKey(options.smallImage)
        .setSmallImageText(options.smallImageText)
        .setStartTimestamp(options.startTimestamp != 0 ? options.startTimestamp : startTime)
        .setEndTimestamp(options.endTimestamp)
        .setPartyID(options.partyID)
        .setPartySize(options.partySize)
        .setPartyMax(options.maxPartySize)
        .setInstance(false);

    if (options.buttons.size() >= 1 && !options.buttons[0].title.empty()) {
        presence.setButton1(options.buttons[0].title, options.buttons[0].url);
    }
    if (options.buttons.size() >= 2 && !options.buttons[1].title.empty()) {
        presence.setButton2(options.buttons[1].title, options.buttons[1].url);
    }

    presence.refresh();
#endif
}

LevelDifficulty RPCManager::getDifficultyFromLevel(GJGameLevel* level) {
    if (level->m_autoLevel) return Auto;
    if (level->m_demon == 1) {
        switch (level->m_demonDifficulty) {
            case 3: return EasyDemon;
            case 4: return MediumDemon;
            case 0: return HardDemon;
            case 5: return InsaneDemon;
            case 6: return ExtremeDemon;
        }
    }
#define DIFF_INT(diff) static_cast<int>(GJDifficulty::diff)
    auto diff = level->getAverageDifficulty();
    if (level->m_levelType == GJLevelType::Main) diff = static_cast<int>(level->m_difficulty);
    switch (diff) {
        case 0: return NA;
        case DIFF_INT(Easy): return Easy;
        case DIFF_INT(Normal): return Normal;
        case DIFF_INT(Hard): return Hard;
        case DIFF_INT(Harder): return Harder;
        case DIFF_INT(Insane): return Insane;
    }
#undef DIFF_INT
    return NA;
}

std::string RPCManager::getAssetKey(LevelDifficulty difficulty) {
    switch (difficulty) {
        case NA: return "na";
        case Auto: return "auto";
        case Easy: return "easy";
        case Normal: return "normal";
        case Hard: return "hard";
        case Harder: return "harder";
        case Insane: return "insane";
        case EasyDemon: return "easy_demon";
        case MediumDemon: return "medium_demon";
        case HardDemon: return "hard_demon";
        case InsaneDemon: return "insane_demon";
        case ExtremeDemon: return "extreme_demon";
    }
}
