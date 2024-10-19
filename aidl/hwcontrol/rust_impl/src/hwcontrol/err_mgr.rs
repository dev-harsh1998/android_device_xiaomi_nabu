//
// Copyright (C) 2024 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
use super::sysfs::SysFSError;
use log::error;

pub fn mng_err(err: SysFSError, sysfs_node: &str) {
    match err {
        SysFSError::Io(_) => {
            error!("Error reading/writing to SysFS node: {}", sysfs_node);
        }
        SysFSError::InvalidPath(sysfs_node) => {
            error!("Invalid SysFS node: {}", sysfs_node);
        }
        SysFSError::InvalidInput(_sysfs_node) => {
            error!("Invalid input");
        }
    }
}