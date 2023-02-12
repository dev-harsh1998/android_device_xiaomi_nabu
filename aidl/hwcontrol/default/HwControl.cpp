#define LOG_TAG "HwControlHAL"

#include "HwControl.h"

#include <android/binder_manager.h>
#include <log/log.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>

#include <fcntl.h>
#include <signal.h>

namespace aidl::custom::hardware::hwcontrol {

::ndk::ScopedAStatus HwControl::getHwState(HwType hwType, int32_t* _aidl_return) {
    ALOGD("getHwState");
    // Switch HW type
    switch (hwType) {
        case HwType::KEYBOARD:
            *_aidl_return = getKeyboardState();
            ALOGI("getHwState: Keyboard state is %d", *_aidl_return);
            break;
        default:
            *_aidl_return = 0;
            ALOGE("getHwState: Unimplemented HW type");
            break;
    }
    return ::ndk::ScopedAStatus::ok();

}

::ndk::ScopedAStatus HwControl::setHwState(HwType hwType, int32_t state) {
    ALOGD("setHwState");
    switch (hwType) {
        case HwType::KEYBOARD:
            // for keyboard we only accept 0 or 1, consider anything greater than 1 as 1
            (state > 1) ? 1 : state;
            setKeyboardState(state);
            ALOGI("setHwState: Keyboard state set to %d", state);
            break;
        default:
            ALOGE("setHwState: Unimplemented HW type");
            break;
    }
    return ::ndk::ScopedAStatus::ok();
}

// Base constructor
HwControl::HwControl(void) {
    ALOGD("HwControl::HwControl constructor initialized");
}

}