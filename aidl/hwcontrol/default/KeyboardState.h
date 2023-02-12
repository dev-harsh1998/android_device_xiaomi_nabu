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
const std::string TAG="KeyboardState";

int getKeyboardState(void);
void setKeyboardState(int state);