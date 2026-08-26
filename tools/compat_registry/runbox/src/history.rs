// SPDX-License-Identifier: GPL-2.0
/*
 * history.rs - Run history management for runbox
 *
 * This module provides functionality to manage run command history,
 * reading configuration from the registry and persisting history to disk.
 *
 * Copyright (c) 2024 Linux Kernel Community
 * Based on original runbox implementation
 */

use std::fs;
use std::path::PathBuf;
use std::io::Write;

use crate::lib;

/// Maximum history entries (default value, actual limit from registry)
const MAX_DEFAULT: usize = 30;

/// Get the history file path
fn history_path() -> PathBuf {
    let base = std::env::var_os("XDG_CONFIG_HOME")
        .map(PathBuf::from)
        .unwrap_or_else(|| {
            let home = std::env::var_os("HOME").unwrap_or_else(|| std::ffi::OsString::from("/tmp"));
            PathBuf::from(home).join(".config").join("runbox")
        });
    
    base.join("history")
}

/// Load history from file and apply registry-based limits
pub fn load() -> Vec<String> {
    // Get maximum history count from registry
    let max_history = lib::get_int("Software\\Runbox", "MaxHistory", MAX_DEFAULT as i32) as usize;
    
    let content = match fs::read_to_string(history_path()) {
        Ok(s) => s,
        Err(_) => return Vec::new(),
    };
    
    // History line processing constants
    const MAX_LEN: usize = 512;

    content
        .lines()
        .map(sanitize_line)
        .filter(|l| !l.is_empty())
        .filter(|l| l.chars().count() <= MAX_LEN)
        .take(max_history) // Apply registry-based limit
        .collect()
}

/// Sanitize a single history line
fn sanitize_line(line: &str) -> String {
    let mut out = String::with_capacity(line.len());
    for ch in line.chars() {
        // Skip control characters
        if ch.is_control() {
            // Replace newlines, carriage returns, and tabs with spaces
            if ch == '\n' || ch == '\r' || ch == '\t' {
                out.push(' ');
            }
            continue;
        }
        out.push(ch);
    }
    
    // Collapse consecutive whitespace
    let collapsed: String = out
        .split_whitespace()
        .collect::<Vec<_>>()
        .join(" ");
    
    collapsed.trim().to_string()
}

/// Record a command in history
pub fn record(cmdline: &str) {
    let cmdline = cmdline.trim();
    if cmdline.is_empty() {
        return;
    }
    
    let mut items = load();
    items.retain(|x| x != cmdline);
    items.insert(0, cmdline.to_string());
    items.truncate(MAX);

    let path = history_path();
    if let Some(dir) = path.parent() {
        let _ = fs::create_dir_all(dir);
    }
    
    if let Ok(mut file) = fs::OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(&path)
    {
        let _ = file.write_all(items.join("\n").as_bytes());
    }
}

/// Clear all history records
pub fn clear() {
    let _ = fs::write(history_path(), "");
}

/// Get the maximum history count from registry
pub fn get_max_history() -> usize {
    lib::get_int("Software\\Runbox", "MaxHistory", MAX_DEFAULT as i32) as usize
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_sanitize_line() {
        let input = "test\tstring\nwith\rcontrol\0characters";
        let result = sanitize_line(input);
        assert_eq!(result, "test string with control characters");
    }
    
    #[test]
    fn test_sanitize_empty_line() {
        let input = "   \t\n\r  ";
        let result = sanitize_line(input);
        assert_eq!(result, "");
    }
    
    #[test]
    fn test_sanitize_long_line() {
        let input = "a".repeat(513); // Exceeds MAX_LEN
        let result = sanitize_line(&input);
        assert_eq!(result.len(), MAX_LEN);
    }
}