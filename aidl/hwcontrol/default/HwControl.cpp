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
    LOG(INFO) << LOG_TAG << ": " << "getHwState: " << hwType;
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
    LOG(INFO) << LOG_TAG << ": " << "setHwState: " << hwType << " to " << state;
    switch (hwType) {
        case HwType::KEYBOARD:
            // for keyboard we only accept 0 or 1, consider anything greater than 1 as 1
            (state > 1) ? 1 : state;
            setKeyboardState(state);
            LOG(INFO) << LOG_TAG << ": " << "setHwState: Keyboard state set to " << state;
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