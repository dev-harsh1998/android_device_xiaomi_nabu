//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
#include <android-base/logging.h>
#include "TouchFeatureState.h"

#define TAG "TouchFeatureState"

void setTouchFeatureState(int feature, int state) {
    // open the touch feature device in read/write mode.
    file_fd fd(open(TOUCH_FEATURE_DEVICE, O_RDWR));

    // Make sure feature is valid it has to be either wake or stylus. We do not support any other feature for now.
    if (feature != TOUCH_FEATURE_TAP2WAKE && feature != TOUCH_FEATURE_STYLUS) {
        if (!DISABLE_DEBUG) LOG(ERROR) << TAG << ": " << "Invalid feature: " << feature;
        return;
    }

    // create a buffer to hold the ioctl data, consider state greater than 0 as 1 always.
    int buf[2] = {feature, state};
    // check if the file descriptor is valid
    if (fd.get() == -1) {
        if (!DISABLE_DEBUG) LOG(ERROR) << TAG << ": " << "Failed to open: " << TOUCH_FEATURE_DEVICE;
        return;
    } else {
        // write the ioctl data to the device
        if (ioctl(fd.get(), TOUCH_FEATURE_IOCTL_SETMODE, &buf) == -1) {
            if (!DISABLE_DEBUG) LOG(ERROR) << TAG << ": " << "Failed to write to: " << TOUCH_FEATURE_DEVICE;
            return;
        } else {
            if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "Wrote touch feature state as: " << ((state > 0) ? "true" : "false")
                << " for feature: " << ((feature == TOUCH_FEATURE_TAP2WAKE) ? "TAP2WAKE" : "STYLUS");
        }
    }
}