# Windows Registry Compatibility Layer

This is a Linux kernel module that simulates Windows registry services, providing Windows registry functionality to the Linux kernel, primarily used to support applications like Wine and other compatibility layers.

## Features

- Maintains registry tree structure in kernel memory
- Supports accessing registry data through `/proc/compat_registry` interface
- Supports loading configuration files from `/etc/registry/` directory
- Supports `.ini`, `.conf`, `.json`, and `.yaml` configuration file formats
- Provides comprehensive kernel API for other modules to query/set registry items
- Implements boolean value rules: commented lines evaluate to `false`, non-commented lines evaluate to `true`

## Build and Enable

### 1. Enable the Module

When configuring the kernel with `make menuconfig`:

```
Device Drivers  --->
    Windows Registry Compatibility Layer  --->
        [*] Windows Registry Compatibility Layer
        [ ] Enable registry debugging
        (1024) Maximum number of registry keys
        (/etc/registry/) Registry configuration directory
```

### 2. Build the Kernel

```bash
make -j$(nproc)
```

### 3. Load the Module

```bash
sudo insmod drivers/compat/compat_registry.ko
```

## Configuration File Formats

### INI Format Example

```ini
[Software]
EnableFeature
DebugMode=true
LogLevel=3
#AutoUpdate
```

### Boolean Value Rules

- `EnableFeature` (no value) → `true`
- `DebugMode=true` (explicit true) → `true`
- `LogLevel=3` (numeric value) → stores 3
- `#AutoUpdate` (commented line) → `false`

## API Interface

### Create Registry Tree
```c
struct registry_tree *tree = registry_tree_create();
```

### Open Registry Key
```c
struct registry_key *key = registry_key_open(tree, "Software/MyApp");
```

### Set Registry Values
```c
// Set boolean value
u32 bool_val = 1;
registry_value_set(key, "Enabled", REG_DWORD, &bool_val);

// Set string
registry_value_set(key, "Name", REG_SZ, "MyApp");
```

### Get Registry Values
```c
// Get boolean value
bool enabled = registry_value_get_bool(key, "Enabled", false);

// Get string
char *name = registry_value_get_string(key, "Name", "Unknown");
```

### Iterate Through Registry
```c
// Iterate values
registry_key_iterate_values(key, value_callback, data);

// Iterate subkeys
registry_key_iterate_subkeys(key, key_callback, data);
```

## /proc Interface

After loading the module, you can view current registry contents through `/proc/compat_registry`:

```bash
cat /proc/compat_registry
```

Example output:
```
[]
  [Software]
    EnableFeature = true
    DebugMode = "true"
    LogLevel = 0x00000003
  [System]
    TimeZone = "UTC"
    Language = "en_US"
```

## Configuration File Directory

The default configuration file directory is `/etc/registry/`. The module loads all supported configuration files from this directory during startup.

Create configuration directory:
```bash
sudo mkdir -p /etc/registry/
```

Place configuration files in the directory:
```bash
sudo cp example.ini /etc/registry/
```

## Usage Examples

### 1. Create Application Configuration

Create `/etc/registry/myapp.ini`:
```ini
[MyApp]
Version=1.0
Debug=false
MaxConnections=100
```

### 2. Use Registry in Kernel Module

```c
#include <linux/module.h>
#include "registry.h"

static int __init my_module_init(void)
{
    struct registry_tree *tree;
    struct registry_key *app_key;
    
    // Create registry tree
    tree = registry_tree_create();
    if (!tree)
        return -ENOMEM;
    
    // Open application key
    app_key = registry_key_open(tree, "MyApp");
    if (!app_key) {
        registry_tree_destroy(tree);
        return -ENOENT;
    }
    
    // Get configuration
    int version = registry_value_get_int(app_key, "Version", 0);
    bool debug = registry_value_get_bool(app_key, "Debug", false);
    int max_conn = registry_value_get_int(app_key, "MaxConnections", 10);
    
    printk(KERN_INFO "App config: version=%d, debug=%s, max_conn=%d\n",
           version, debug ? "true" : "false", max_conn);
    
    registry_key_close(app_key);
    registry_tree_destroy(tree);
    
    return 0;
}

static void __exit my_module_exit(void)
{
}

module_init(my_module_init);
module_exit(my_module_exit);
```

## Debugging

Enable debug output:
```
Device Drivers  --->
    Windows Registry Compatibility Layer  --->
        [*] Enable registry debugging
```

Debug information will appear in kernel logs:
```bash
dmesg | grep compat_registry
```

## Notes

1. This module runs only in kernel space and does not involve any user interface
2. All data is stored in kernel memory and will be lost after reboot
3. Default configuration directory is `/etc/registry/`, which can be modified via compile options
4. Maximum number of supported keys can be adjusted via compile options
5. For large amounts of registry data, memory usage adjustments may be needed

## Compatibility

- Linux kernel version: 6.12+
- Compiler: GCC 7+
- Requires PROC_FS support

## License

GPL v2

## Author

Linux Kernel Compatibility Team