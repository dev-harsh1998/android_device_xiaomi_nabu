//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
#include <string>

#pragma once

struct KeyboardState {
    std::string path;
    std::string enable;
    std::string disable;
};

using KeyboardIO = struct KeyboardState;

const KeyboardIO kb_state = {
    .path = "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_conn_status",
    .enable = "enable_keyboard",
    .disable = "disable_keyboard"
};

int getKeyboardState(void);
void setKeyboardState(int state);