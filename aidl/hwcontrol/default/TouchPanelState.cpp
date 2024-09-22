//
// Copyright (C) 2024 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
#include <android-base/logging.h>
#include <fstream>
#include "TouchPanelState.h"

#define TAG "TouchPanelState"

static inline void setPathState(const std::string &path, int state) {
    std::ofstream path_stream(path);
    if (!path_stream.is_open() && !path_stream.good()){
        LOG(ERROR) << "setPathState: " << "Failed to open " << path;
        return;
    }
    path_stream << (state == 1) ? "1" : "0";
    path_stream.close(); 
}

void setTouchPanelState(int feature, int state) {
    std::string path = SYSFS_TOUCHPANEL_PARENT_PATH;
    switch (feature) {
        case TOUCH_FEATURE_TAP2WAKE:
            path.append("double_tap");
            break;
        case TOUCH_FEATURE_STYLUS:
            path.append("pen");
            break;
        default:
            LOG(ERROR) << "setTouchPanelState: " << "Unknown feature " << feature;
            return;
    }

    LOG(INFO) << "setTouchPanelState: " << "Setting " << path << " to " << state;
    // Set the state
    setPathState(path, state);
}