# SPDX-License-Identifier: GPL-2.0
#
# Makefile for Windows Registry Compatibility Layer
#

obj-$(CONFIG_COMPAT_REGISTRY) += compat_registry.o

compat_registry-y := compat_registry.o registry.o config_parser.o proc_interface.o

# 如果需要支持特定的配置文件格式，可以添加相应的目标
# 例如：obj-$(CONFIG_COMPAT_REGISTRY_JSON) += json_parser.o
# 例如：obj-$(CONFIG_COMPAT_REGISTRY_YAML) += yaml_parser.o

# 清理规则
clean-files := *.o *.mod.c *.symvers *.order *.cmd *.ko *.markers

# 内核模块编译规则
ccflags-y := -I$(src)
ccflags-y += -D__KERNEL__
ccflags-y += -DMODULE

# 启用更严格的编译警告
ccflags-y += -Wall -Wextra -Wno-unused-parameter -Wno-error

# 如果启用调试，添加调试标志
ifeq ($(CONFIG_COMPAT_REGISTRY_DEBUG),y)
ccflags-y += -DCOMPAT_REGISTRY_DEBUG
endif

# 如果启用JSON支持，添加相关标志
ifeq ($(CONFIG_COMPAT_REGISTRY_JSON),y)
ccflags-y += -DCOMPAT_REGISTRY_JSON
endif

# 如果启用YAML支持，添加相关标志
ifeq ($(CONFIG_COMPAT_REGISTRY_YAML),y)
ccflags-y += -DCOMPAT_REGISTRY_YAML
endif

# 定义模块信息
compat_registry-objs := $(compat_registry-y)