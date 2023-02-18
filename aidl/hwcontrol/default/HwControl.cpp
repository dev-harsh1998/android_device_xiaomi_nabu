//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
#define TAG "HwControlHAL"

#include "HwControl.h"

#include <android/binder_manager.h>
#include <android-base/logging.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>

#include <fcntl.h>
#include <signal.h>

namespace aidl::custom::hardware::hwcontrol {

::ndk::ScopedAStatus HwControl::getHwState(HwType hwType, int32_t* _aidl_return) {
    if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "getHwState called";
    // Switch HW type
    switch (hwType) {
        case HwType::KEYBOARD:
            *_aidl_return = getKeyboardState();
            if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "getHwState: Keyboard state is " << *_aidl_return;
            break;
        default:
            *_aidl_return = 0;
            if (!DISABLE_DEBUG) LOG(ERROR) << TAG << ": " << "getHwState: Unimplemented HW type";
            break;
    }
    return ::ndk::ScopedAStatus::ok();

}

::ndk::ScopedAStatus HwControl::setHwState(HwType hwType, int32_t state) {
    if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "setHwState called with state = " << state;

    // for HwType we only accept 0 or 1, consider anything greater than 1 as 1 and warn
    if (state > 1) {
        if (!DISABLE_DEBUG) LOG(WARNING) << TAG << ": " << "setHwState:" << "state is greater than 1, setting to 1";
        state = 1;
    }

    switch (hwType) {
        case HwType::KEYBOARD:
            setKeyboardState(state);
            if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "setHwState: Keyboard state set to: " << state;
            break;
        case HwType::STYLUS:
            setTouchFeatureState(TOUCH_FEATURE_STYLUS, state);
            if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "setHwState: Stylus state set to: " << state;
            break;
        case HwType::TAP2WAKE:
            setTouchFeatureState(TOUCH_FEATURE_TAP2WAKE, state);
            if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "setHwState: Tap2Wake state set to: " << state;
            break;
        default:
            if (!DISABLE_DEBUG) LOG(ERROR) << TAG << ": " << "setHwState: Unimplemented HW type";
            break;
    }
    return ::ndk::ScopedAStatus::ok();
}

// Base constructor
HwControl::HwControl(void) {
    if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "HwControl: Constructor";
}

}