/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 注册表/proc接口头文件
 * 
 * 提供/proc/compat_registry接口的声明，用于访问注册表数据。
 */

#ifndef _COMPAT_PROC_INTERFACE_H
#define _COMPAT_PROC_INTERFACE_H

#include <linux/fs.h>
#include "registry.h"

/* 初始化proc接口 */
extern int registry_proc_init(struct registry_tree *tree);

/* 清理proc接口 */
extern void registry_proc_cleanup(void);

#endif /* _COMPAT_PROC_INTERFACE_H */