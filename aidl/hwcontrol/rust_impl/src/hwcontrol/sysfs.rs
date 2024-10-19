//
// Copyright (C) 2024 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
use std::fs::{File, OpenOptions};
use std::io::{self, Read, Write};
use std::path::Path;
use std::fmt;
use std::error::Error;
use log::{info, warn};

/// Custom error type for SysFS operations
#[derive(Debug)]
pub enum SysFSError {
    Io(io::Error),
    InvalidInput(String),
    InvalidPath(String),
}

impl fmt::Display for SysFSError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            SysFSError::Io(err) => write!(f, "IO error: {}", err),
            SysFSError::InvalidInput(msg) => write!(f, "Invalid input: {}", msg),
            SysFSError::InvalidPath(path) => write!(f, "Invalid SysFS path: {}", path),
        }
    }
}

impl Error for SysFSError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            SysFSError::Io(err) => Some(err),
            _ => None,
        }
    }
}

impl From<io::Error> for SysFSError {
    fn from(err: io::Error) -> Self {
        SysFSError::Io(err)
    }
}

/// Manages operations on SysFS entries
pub struct SysFSManager;

impl SysFSManager {
    /// Validates if the given path is a valid SysFS entry
    ///
    /// # Arguments
    ///
    /// * `path` - The path to validate
    ///
    /// # Returns
    ///
    /// * `Ok(())` if the path is valid
    /// * `Err(SysFSError)` if the path is invalid
    fn validate_path<P: AsRef<Path>>(path: P) -> Result<(), SysFSError> {
        let path = path.as_ref();
        if !path.starts_with("/sys/") || !path.exists() {
            return Err(SysFSError::InvalidPath(path.to_string_lossy().into_owned()));
        }
        Ok(())
    }

    /// Writes data to a SysFS entry
    ///
    /// # Arguments
    ///
    /// * `path` - The path to the SysFS entry
    /// * `data` - The data to write
    ///
    /// # Returns
    ///
    /// * `Ok(())` if the write was successful
    /// * `Err(SysFSError)` if an error occurred
    pub fn write_to_sysfs<P: AsRef<Path>>(path: P, data: &str) -> Result<(), SysFSError> {
        let path = path.as_ref();
        
        // Validate path and check permissions before attempting to write
        Self::validate_path(path)?;

        info!("Attempting to write '{}' to {}", data, path.display());

        let trimmed_data = data.trim();
        if trimmed_data.is_empty() {
            warn!("Attempting to write empty string to {}", path.display());
            return Err(SysFSError::InvalidInput("Empty input string".to_string()));
        }

        let mut file = OpenOptions::new().write(true).open(path)?;

        file.write_all(trimmed_data.as_bytes())?;
        file.flush()?;

        info!("Successfully wrote to {}", path.display());
        Ok(())
    }

    /// Reads data from a SysFS entry
    ///
    /// # Arguments
    ///
    /// * `path` - The path to the SysFS entry
    ///
    /// # Returns
    ///
    /// * `Ok(String)` containing the data read from the SysFS entry
    /// * `Err(SysFSError)` if an error occurred
    pub fn read_from_sysfs<P: AsRef<Path>>(path: P) -> Result<String, SysFSError> {
        let path = path.as_ref();

        // Validate path and check permissions before attempting to read
        Self::validate_path(path)?;

        info!("Attempting to read from {}", path.display());

        let mut file = File::open(path)?;
        let mut contents = String::new();
        file.read_to_string(&mut contents)?;

        let trimmed_contents = contents.trim().to_string();
        info!("Successfully read '{}' from {}", trimmed_contents, path.display());
        Ok(trimmed_contents)
    }
}