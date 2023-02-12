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
    // destroy using void
    (void ) hwType;
    *_aidl_return = 0;
    return ::ndk::ScopedAStatus::ok();

}

::ndk::ScopedAStatus HwControl::setHwState(HwType hwType, int32_t state) {
    ALOGD("setHwState");
    // destroy using void
    (void ) hwType;
    (void ) state;
    return ::ndk::ScopedAStatus::ok();
}

// Base constructor
HwControl::HwControl(void) {
    ALOGD("HwControl::HwControl");
}

}