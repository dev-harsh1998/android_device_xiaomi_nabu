//
// Copyright (C) 2024 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
use super::sysfs::SysFSManager;
use super::err_mgr::mng_err;

/// This function sets the pen state.
/// input: enable: bool
/// output: Result<(), bool>
/// The function returns Ok(()) if the operation was successful, otherwise it returns Err(false).
pub fn set_pen(enable: bool) -> Result<(), bool> {
    let sysfs_pen_node = "/sys/touchpanel/pen";
    match SysFSManager::write_to_sysfs(sysfs_pen_node, if enable { "1" } else { "0" }) {
        Ok(_) => Ok(()),
        Err(e) => {
            mng_err(e, sysfs_pen_node);
            Err(false)
        }
    }
}

// This function gets the pen state.
// output: Result<bool, bool>
// The function returns Ok(true) if the pen is enabled, otherwise it returns Ok(false).
pub fn get_pen() -> Result<bool, bool> {
    let sysfs_pen_node = "/sys/touchpanel/pen";
    match SysFSManager::read_from_sysfs(sysfs_pen_node) {
        Ok(data) => {
            if data.trim() == "1" {
                Ok(true)
            } else {
                Ok(false)
            }
        }
        Err(e) => {
            mng_err(e, sysfs_pen_node);
            Err(false)
        }
    }
}