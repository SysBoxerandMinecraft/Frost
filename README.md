# Windows Registry Compatibility Layer

这是Linux内核中的一个模拟Windows注册表服务的模块，为Linux内核提供模拟Windows注册表服务的功能，主要用于支持Wine等兼容层应用。

**English Version:** [README_US.md](README_US.md)

## 功能特性

- 在内核内存中维护注册表树结构
- 支持通过 `/proc/compat_registry` 接口访问注册表数据
- 支持从 `/etc/registry/` 目录加载配置文件
- 支持 `.ini`、`.conf`、`.json`、`.yaml` 格式的配置文件
- 提供完整的内核API供其他模块查询/设置注册表项
- 实现布尔值判定规则：注释行值为 `false`，非注释行值为 `true`

## 编译和启用

### 1. 启用模块

在使用 `make menuconfig` 配置内核时：

```
Device Drivers  --->
    Windows Registry Compatibility Layer  --->
        [*] Windows Registry Compatibility Layer
        [ ] Enable registry debugging
        (1024) Maximum number of registry keys
        (/etc/registry/) Registry configuration directory
```

### 2. 编译内核

```bash
make -j$(nproc)
```

### 3. 加载模块

```bash
sudo insmod drivers/compat/compat_registry.ko
```

## 配置文件格式

### INI 格式示例

```ini
[Software]
EnableFeature
DebugMode=true
LogLevel=3
#AutoUpdate
```

### 布尔值判定规则

- `EnableFeature`（无值）→ `true`
- `DebugMode=true`（显式true）→ `true`
- `LogLevel=3`（数值）→ 存储3
- `#AutoUpdate`（注释行）→ `false`

## API 接口

### 创建注册表树
```c
struct registry_tree *tree = registry_tree_create();
```

### 打开注册表键
```c
struct registry_key *key = registry_key_open(tree, "Software/MyApp");
```

### 设置注册表值
```c
// 设置布尔值
u32 bool_val = 1;
registry_value_set(key, "Enabled", REG_DWORD, &bool_val);

// 设置字符串
registry_value_set(key, "Name", REG_SZ, "MyApp");
```

### 获取注册表值
```c
// 获取布尔值
bool enabled = registry_value_get_bool(key, "Enabled", false);

// 获取字符串
char *name = registry_value_get_string(key, "Name", "Unknown");
```

### 遍历注册表
```c
// 遍历值
registry_key_iterate_values(key, value_callback, data);

// 遍历子键
registry_key_iterate_subkeys(key, key_callback, data);
```

## /proc 接口

模块加载后，可以通过 `/proc/compat_registry` 查看当前注册表内容：

```bash
cat /proc/compat_registry
```

输出示例：
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

## 配置文件目录

默认配置文件目录为 `/etc/registry/`。模块启动时会从此目录加载所有支持的配置文件。

创建配置目录：
```bash
sudo mkdir -p /etc/registry/
```

将配置文件放入该目录：
```bash
sudo cp example.ini /etc/registry/
```

## 使用示例

### 1. 创建应用程序配置

创建 `/etc/registry/myapp.ini`：
```ini
[MyApp]
Version=1.0
Debug=false
MaxConnections=100
```

### 2. 在内核模块中使用注册表

```c
#include <linux/module.h>
#include "registry.h"

static int __init my_module_init(void)
{
    struct registry_tree *tree;
    struct registry_key *app_key;
    
    // 创建注册表树
    tree = registry_tree_create();
    if (!tree)
        return -ENOMEM;
    
    // 打开应用程序键
    app_key = registry_key_open(tree, "MyApp");
    if (!app_key) {
        registry_tree_destroy(tree);
        return -ENOENT;
    }
    
    // 获取配置
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

## 调试

启用调试输出：
```
Device Drivers  --->
    Windows Registry Compatibility Layer  --->
        [*] Enable registry debugging
```

调试信息会显示在内核日志中：
```bash
dmesg | grep compat_registry
```

## 注意事项

1. 此模块仅在内核态运行，不涉及任何用户界面
2. 所有数据存储在内核内存中，重启后数据会丢失
3. 默认配置目录为 `/etc/registry/`，可通过编译选项修改
4. 支持的最大键数量可以通过编译选项调整
5. 对于大量注册表数据，可能需要调整内存使用

## 兼容性

- Linux内核版本：6.12+
- 编译器：GCC 7+
- 需要PROC_FS支持

## 许可证

GPL v2

## 作者

Linux Kernel Compatibility Team