// SPDX-License-Identifier: GPL-2.0
//
// build.rs - Build script for runbox
//
// This build script configures the Rust compilation for runbox,
// particularly for linking with the compat_registry library.
//
// Copyright (c) 2024 Linux Kernel Community

fn main() {
    // For Linux targets, link with the compat_registry library
    #[cfg(target_os = "linux")]
    {
        println!("cargo:rustc-link-search=../../../lib");
        println!("cargo:rustc-link-lib=static=compat_registry");
        println!("cargo:warning=Using compat_registry library for registry access");
    }
    
    // Add any platform-specific configurations here
    #[cfg(not(target_os = "linux"))]
    {
        println!("cargo:warning=runbox is designed for Linux systems");
    }
}