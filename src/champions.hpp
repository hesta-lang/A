// INCREDIBLY HACKY WORKAROUND
// to get Champions support working
// until Champions releases

#pragma once

#include <Geode/loader/Dispatch.hpp>

// required for GEODE_EVENT_EXPORT
// #define MY_MOD_ID "ninxout.champions"

namespace champions {
    inline geode::Result<bool> inChampionsGame() {
        return geode::Ok(false);
    }
    inline geode::Result<bool> isQueueing() {
        return geode::Ok(false);
    }
    inline geode::Result<int> getLastQueue() {
        return geode::Err("no champ!");
    }
    inline geode::Result<GJGameLevel*> getChampionsLevel() {
        return geode::Err("no champ!");
    }
}

#undef MY_MOD_ID
