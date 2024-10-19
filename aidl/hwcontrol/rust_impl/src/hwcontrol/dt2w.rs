//
// Copyright (C) 2024 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
use super::sysfs::SysFSManager;
use super::err_mgr::mng_err;

/// This function sets the tap to wake state.
/// input: enable: bool
/// output: Result<(), bool>
/// The function returns Ok(()) if the operation was successful, otherwise it returns Err(false).
pub fn set_tap_to_wake(enable: bool) -> Result<(), bool> {
    let sysfs_tap_node = "/sys/touchpanel/double_tap";
    match SysFSManager::write_to_sysfs(sysfs_tap_node, if enable { "1" } else { "0" }) {
        Ok(_) => Ok(()),
        Err(e) => {
            mng_err(e, sysfs_tap_node);
            Err(false)
        }
    }
}

// This function gets the tap to wake state.
// output: Result<bool, bool>
// The function returns Ok(true) if the tap to wake is enabled, otherwise it returns Ok(false).
pub fn get_tap_to_wake() -> Result<bool, bool> {
    let sysfs_tap_node = "/sys/touchpanel/double_tap";
    match SysFSManager::read_from_sysfs(sysfs_tap_node) {
        Ok(data) => {
            if data.trim() == "1" {
                Ok(true)
            } else {
                Ok(false)
            }
        }
        Err(e) => {
            mng_err(e, sysfs_tap_node);
            Err(false)
        }
    }
}