<!--
  2026年8月26日 · 距 Linux 0.01 诞生已过 35 年
  Frost 在这个日子正式归档。
  愿每一行代码都像 0.01 那样，成为后来者的起点。
-->

# Frost Linux Kernel

Frost 是一个基于主线 Linux 6.12.104 定制构建的内核源码树。  
它在保留原有内核功能的基础上，额外集成了一个实验性的注册表兼容层模块（位于 `drivers/compat_registry/`），用于探索内核态与用户态的配置交互机制。  

该模块处于早期开发阶段，仅供研究学习使用。

---

## 主要特性

- 基于 Linux 6.12.104 LTS 主线版本
- 默认 x86_64 配置，支持绝大多数现代硬件
- 内置实验性注册表兼容模块（`drivers/compat/`）
- 保留完整内核调试支持（可配置）

---

## 构建说明

### 环境准备

```bash
sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev dwarves
```

### 配置与编译

```bash
make olddefconfig
make -j$(nproc) bzImage
make -j$(nproc) modules
```

### 安装内核

```bash
sudo make modules_install
sudo make install
sudo update-grub
```

---

## 目录结构

```text
arch/        架构相关代码
block/       块设备层
crypto/      内核加密 API
drivers/     设备驱动（包含 compat_registry/ 模块）
fs/          文件系统
include/     公共头文件
init/        初始化代码
ipc/         进程间通信
kernel/      内核核心
lib/         内核库函数
mm/          内存管理
net/         网络协议栈
scripts/     构建工具脚本
tools/       配套用户态工具（独立构建）
usr/         initramfs 生成
virt/        虚拟化支持
```

---

## 许可证

本项目基于 GPL-2.0 许可证发布，与 Linux 内核保持一致。  
详见根目录下的 `COPYING` 文件。

---

## 致谢

- Linux 内核社区
- 所有参与内核开发和测试的贡献者
