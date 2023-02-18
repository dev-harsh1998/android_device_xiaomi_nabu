//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
#include "KeyboardState.h"
#include <android-base/logging.h>
#include <fstream>

const std::string TAG="KeyboardState";

int getKeyboardState(void) {
    std::ifstream kb_data(kb_state.path);

    if (!kb_data.is_open() || !kb_data.good()) {
        if (!DISABLE_DEBUG) LOG(ERROR) << TAG << ": "<< "Failed to open: " << kb_state.path;
        return 0; //asume false.
    }

    std::string sysfs_data((std::istreambuf_iterator<char>(kb_data)),
                           std::istreambuf_iterator<char>());

    if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "Read keyboard state as: " << ((sysfs_data == kb_state.enable) ? "true" : "false");
    kb_data.close();

    if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "Returning keyboard state as: " << ((sysfs_data == kb_state.enable) ? "true" : "false");
    return (sysfs_data == kb_state.enable) ? 1 : 0;
}

void setKeyboardState(int state) {
    std::ofstream kb_data(kb_state.path);

    if (!kb_data.is_open() || !kb_data.good()) {
        if (!DISABLE_DEBUG) LOG(ERROR) << TAG << ": "<< "Failed to open: " << kb_state.path;
        return;
    }

    if (state == 1) {
        kb_data << kb_state.enable;
    } else {
        kb_data << kb_state.disable;
    }

    if (!DISABLE_DEBUG) LOG(INFO) << TAG << ": " << "Wrote keyboard state as: " << ((state == 1) ? "true" : "false");
    kb_data.close();
}