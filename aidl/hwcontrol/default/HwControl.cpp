#define LOG_TAG "HwControlHAL"

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
    LOG(INFO) << LOG_TAG << ": " << "getHwState called";
    // Switch HW type
    switch (hwType) {
        case HwType::KEYBOARD:
            *_aidl_return = getKeyboardState();
            LOG(INFO) << LOG_TAG << ": " << "getHwState: Keyboard state is " << *_aidl_return;
            break;
        default:
            *_aidl_return = 0;
            LOG(ERROR) << LOG_TAG << ": " << "getHwState: Unimplemented HW type";
            break;
    }
    return ::ndk::ScopedAStatus::ok();

}

::ndk::ScopedAStatus HwControl::setHwState(HwType hwType, int32_t state) {
    LOG(INFO) << LOG_TAG << ": " << "setHwState called with state = " << state;

    // for HwType we only accept 0 or 1, consider anything greater than 1 as 1 and warn
    if (state > 1) {
        LOG(WARNING) << LOG_TAG << ": " << "setHwState:" << "state is greater than 1, setting to 1";
        state = 1;
    }

    switch (hwType) {
        case HwType::KEYBOARD:
            setKeyboardState(state);
            LOG(INFO) << LOG_TAG << ": " << "setHwState: Keyboard state set to: " << state;
            break;
        case HwType::STYLUS:
            setTouchFeatureState(TOUCH_FEATURE_STYLUS, state);
            LOG(INFO) << LOG_TAG << ": " << "setHwState: Stylus state set to: " << state;
            break;
        case HwType::TAP2WAKE:
            setTouchFeatureState(TOUCH_FEATURE_TAP2WAKE, state);
            LOG(INFO) << LOG_TAG << ": " << "setHwState: Tap2Wake state set to: " << state;
            break;
        default:
            LOG(ERROR) << LOG_TAG << ": " << "setHwState: Unimplemented HW type";
            break;
    }
    return ::ndk::ScopedAStatus::ok();
}

// Base constructor
HwControl::HwControl(void) {
    LOG(INFO) << LOG_TAG << ": " << "HwControl: Constructor";
}

}