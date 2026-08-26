// SPDX-License-Identifier: GPL-2.0
/*
 * lib.rs - Registry access library for runbox
 *
 * This library provides interfaces to read configuration values from the
 * kernel compatibility registry module through /proc/compat_registry.
 *
 * Copyright (c) 2024 Linux Kernel Community
 * Based on original runbox implementation
 */

//! Registry access library for runbox
//! 
//! This module provides FFI bindings to the C-based compat_registry library
//! and Rust-friendly wrappers for registry access.

use std::ffi::{CString, CStr};
use std::os::raw::{c_char, c_int};

// FFI bindings to the C library
#[link(name = "compat_registry")]
extern "C" {
    fn reg_get_string(path: *const c_char, key: *const c_char, default_val: *const c_char) -> *mut c_char;
    fn reg_get_int(path: *const c_char, key: *const c_char, default_val: c_int) -> c_int;
    fn reg_get_bool(path: *const c_char, key: *const c_char, default_val: bool) -> bool;
    fn reg_free_string(str: *mut c_char);
}

/// Get a string value from the registry
/// 
/// # Arguments
/// * `path` - Registry path (e.g. "Software\\Runbox\\MaxHistory")
/// * `key` - Key name to retrieve
/// * `default_val` - Default value if key not found or invalid
/// 
/// # Returns
/// String value from registry or default value
pub fn get_string(path: &str, key: &str, default_val: &str) -> String {
    let path_c = CString::new(path).unwrap();
    let key_c = CString::new(key).unwrap();
    let default_val_c = CString::new(default_val).unwrap();
    
    let result = unsafe {
        reg_get_string(path_c.as_ptr(), key_c.as_ptr(), default_val_c.as_ptr())
    };
    
    if result.is_null() {
        return default_val.to_string();
    }
    
    let result_str = unsafe { CStr::from_ptr(result) }.to_string_lossy().into_owned();
    unsafe { reg_free_string(result) };
    result_str
}

/// Get an integer value from the registry
/// 
/// # Arguments
/// * `path` - Registry path (e.g. "Software\\Runbox\\MaxHistory")
/// * `key` - Key name to retrieve
/// * `default_val` - Default value if key not found or invalid
/// 
/// # Returns
/// Integer value from registry or default value
pub fn get_int(path: &str, key: &str, default_val: i32) -> i32 {
    let path_c = CString::new(path).unwrap();
    let key_c = CString::new(key).unwrap();
    
    unsafe { reg_get_int(path_c.as_ptr(), key_c.as_ptr(), default_val) }
}

/// Get a boolean value from the registry
/// 
/// # Arguments
/// * `path` - Registry path (e.g. "Software\\Runbox\\ShowDesktopShortcuts")
/// * `key` - Key name to retrieve
/// * `default_val` - Default value if key not found or invalid
/// 
/// # Returns
/// Boolean value from registry or default value
pub fn get_bool(path: &str, key: &str, default_val: bool) -> bool {
    let path_c = CString::new(path).unwrap();
    let key_c = CString::new(key).unwrap();
    
    unsafe { reg_get_bool(path_c.as_ptr(), key_c.as_ptr(), default_val) }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_get_string() {
        let result = get_string("Software\\Test", "Key", "default");
        assert_eq!(result, "default");
    }
    
    #[test]
    fn test_get_int() {
        let result = get_int("Software\\Test", "Number", 42);
        assert_eq!(result, 42);
    }
    
    #[test]
    fn test_get_bool() {
        let result = get_bool("Software\\Test", "Flag", true);
        assert_eq!(result, true);
    }
}