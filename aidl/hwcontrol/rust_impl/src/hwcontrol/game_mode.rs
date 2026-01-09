//
// Copyright (C) 2024 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
use super::sysfs::SysFSManager;
use super::err_mgr::mng_err;

/// This function sets the game mode state.
/// When enabled (1), sets touchscreen firmware to MIUI 12.5 mode.
/// When disabled (0), returns to original firmware behavior.
/// Note: Requires a screen power cycle (off/on) to apply.
/// input: enable: bool
/// output: Result<(), bool>
/// The function returns Ok(()) if the operation was successful, otherwise it returns Err(false).
pub fn set_game_mode(enable: bool) -> Result<(), bool> {
    let sysfs_game_mode_node = "/sys/touchpanel/game_mode";
    match SysFSManager::write_to_sysfs(sysfs_game_mode_node, if enable { "1" } else { "0" }) {
        Ok(_) => Ok(()),
        Err(e) => {
            mng_err(e, sysfs_game_mode_node);
            Err(false)
        }
    }
}

// This function gets the game mode state.
// output: Result<bool, bool>
// The function returns Ok(true) if game mode is enabled, otherwise it returns Ok(false).
pub fn get_game_mode() -> Result<bool, bool> {
    let sysfs_game_mode_node = "/sys/touchpanel/game_mode";
    match SysFSManager::read_from_sysfs(sysfs_game_mode_node) {
        Ok(data) => {
            if data.trim() == "1" {
                Ok(true)
            } else {
                Ok(false)
            }
        }
        Err(e) => {
            mng_err(e, sysfs_game_mode_node);
            Err(false)
        }
    }
}

