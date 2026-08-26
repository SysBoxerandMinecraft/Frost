// SPDX-License-Identifier: GPL-2.0
/*
 * main.rs - Runbox main application
 *
 * A Windows-style run dialog implemented in Rust.
 * This is a kernel tool that reads configuration from /proc/compat_registry.
 *
 * Copyright (c) 2024 Linux Kernel Community
 * Based on original runbox implementation
 */

mod history;
mod lib;

use std::env;

/// Configuration structure for runbox
struct RunConfig {
    max_history: usize,
    prompt_text: String,
    show_desktop_shortcuts: bool,
    confirm_exit: bool,
}

impl Default for RunConfig {
    fn default() -> Self {
        RunConfig {
            max_history: 30,
            prompt_text: "Open:".to_string(),
            show_desktop_shortcuts: true,
            confirm_exit: true,
        }
    }
}

/// Load configuration from registry
fn load_config() -> RunConfig {
    RunConfig {
        max_history: lib::get_int("Software\\Runbox", "MaxHistory", 30),
        prompt_text: lib::get_string("Software\\Runbox", "PromptText", "Open:"),
        show_desktop_shortcuts: lib::get_bool("Software\\Runbox", "ShowDesktopShortcuts", true),
        confirm_exit: lib::get_bool("Software\\Runbox", "ConfirmExit", true),
    }
}

/// Show help information
fn show_help() {
    println!("Runbox - Run Dialog for Linux");
    println!("Usage: runbox [options]");
    println!("Options:");
    println!("  -h, --help      Show this help");
    println!("  -v, --version   Show version information");
    println!("");
    println!("Configuration is loaded from registry: HKLM\\Software\\Runbox");
}

/// Show version information
fn show_version() {
    println!("Runbox version 1.0");
    println!("A Windows-style run dialog implemented in Rust");
    println!("Configuration from Linux kernel registry module");
}

/// Simulate run dialog functionality
fn run_dialog() {
    let config = load_config();
    
    println!("Runbox - Run Dialog Simulation");
    println!("===============================");
    println!("Configuration loaded from registry:");
    println!("  Max history: {}", config.max_history);
    println!("  Prompt text: {}", config.prompt_text);
    println!("  Desktop shortcuts: {}", if config.show_desktop_shortcuts { "enabled" } else { "disabled" });
    println!("  Confirm exit: {}", if config.confirm_exit { "enabled" } else { "disabled" });
    
    // Load and display history
    let history = history::load();
    println!("\nRecent commands:");
    for (i, cmd) in history.iter().take(5).enumerate() {
        println!("  {}. {}", i + 1, cmd);
    }
    
    // Simulate command execution
    println!("\nSimulating command execution...");
    let test_cmd = "ls -la";
    println!("Executing: {}", test_cmd);
    
    // Record to history
    history::record(test_cmd);
    println!("Command added to history");
    
    // Show new history
    let updated_history = history::load();
    println!("\nUpdated history (first 5 entries):");
    for (i, cmd) in updated_history.iter().take(5).enumerate() {
        println!("  {}. {}", i + 1, cmd);
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    
    // Parse command line arguments
    for arg in &args[1..] {
        match arg.as_str() {
            "-h" | "--help" => {
                show_help();
                return;
            }
            "-v" | "--version" => {
                show_version();
                return;
            }
            _ => {
                println!("Unknown option: {}", arg);
                show_help();
                return;
            }
        }
    }
    
    // Run the dialog simulation
    run_dialog();
}