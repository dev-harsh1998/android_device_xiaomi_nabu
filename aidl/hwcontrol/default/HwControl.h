#pragma once

#include <aidl/custom/hardware/hwcontrol/BnHwControl.h>

#include <mutex>
#include <thread>

#include "KeyboardState.h"

namespace aidl::custom::hardware::hwcontrol {

class HwControl : public BnHwControl {
    public:
    HwControl(void);
    ::ndk::ScopedAStatus getHwState(HwType hwType, int *_aidl_return) override;
    ::ndk::ScopedAStatus setHwState(HwType hwType, int state) override;
};

}