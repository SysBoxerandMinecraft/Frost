// SPDX-License-Identifier: GPL-2.0
/*
 * Windows Registry Compatibility Layer - Main Module
 * 
 * 此文件是Windows注册表兼容层的主模块，整合了注册表核心功能、
 * 配置解析器和proc接口，提供完整的注册表服务。
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include "registry.h"
#include "config_parser.h"
#include "proc_interface.h"

#define MODULE_NAME "compat_registry"

/* 全局注册表树实例 */
static struct registry_tree *registry_tree = NULL;
/* 配置目录路径 */
static char config_dir[PATH_MAX] = CONFIG_REGISTRY_DIR;

/* 模块参数 */
static bool auto_load_config = true;
module_param(auto_load_config, bool, 0444);
MODULE_PARM_DESC(auto_load_config, "Auto-load configuration from /etc/registry/ directory");

/* 从配置目录加载配置文件 */
static int load_configuration(void)
{
	struct config_parse_result *result;
	int ret = 0;
	
	if (!registry_tree) {
		printk(KERN_ERR "%s: Registry tree not initialized\n", MODULE_NAME);
		return -EINVAL;
	}
	
	if (!auto_load_config) {
		printk(KERN_INFO "%s: Configuration auto-loading disabled\n", MODULE_NAME);
		return 0;
	}
	
	printk(KERN_INFO "%s: Loading configuration from %s\n", MODULE_NAME, config_dir);
	
	/* 解析配置目录 */
	result = parse_config_directory(config_dir, registry_tree);
	if (!result) {
		printk(KERN_ERR "%s: Failed to parse configuration directory\n", MODULE_NAME);
		return -EIO;
	}
	
	if (result->error_count > 0) {
		printk(KERN_WARNING "%s: Configuration parsed with %d errors\n", 
		       MODULE_NAME, result->error_count);
	}
	
	printk(KERN_INFO "%s: Configuration loaded successfully - %d files, %d keys, %d values\n",
	       MODULE_NAME, result->file_count, result->key_count, result->value_count);
	
	free_config_parse_result(result);
	return ret;
}

/* 初始化注册表树 */
static int __init registry_tree_init(void)
{
	registry_tree = registry_tree_create();
	if (!registry_tree) {
		printk(KERN_ERR "%s: Failed to create registry tree\n", MODULE_NAME);
		return -ENOMEM;
	}
	
	printk(KERN_INFO "%s: Registry tree initialized\n", MODULE_NAME);
	return 0;
}

/* 清理注册表树 */
static void __exit registry_tree_exit(void)
{
	if (registry_tree) {
		registry_tree_destroy(registry_tree);
		registry_tree = NULL;
		printk(KERN_INFO "%s: Registry tree cleaned up\n", MODULE_NAME);
	}
}

/* 模块初始化函数 */
static int __init compat_registry_init(void)
{
	int ret;
	
	printk(KERN_INFO "%s: Initializing Windows Registry Compatibility Layer\n", MODULE_NAME);
	
	/* 初始化注册表树 */
	ret = registry_tree_init();
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to initialize registry tree\n", MODULE_NAME);
		return ret;
	}
	
	/* 初始化proc接口 */
	ret = registry_proc_init(registry_tree);
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to initialize proc interface\n", MODULE_NAME);
		registry_tree_exit();
		return ret;
	}
	
	/* 加载配置文件 */
	ret = load_configuration();
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to load configuration\n", MODULE_NAME);
		registry_proc_cleanup();
		registry_tree_exit();
		return ret;
	}
	
	printk(KERN_INFO "%s: Windows Registry Compatibility Layer initialized successfully\n", 
	       MODULE_NAME);
	printk(KERN_INFO "%s: Available at /proc/compat_registry\n", MODULE_NAME);
	
	return 0;
}

/* 模块清理函数 */
static void __exit compat_registry_exit(void)
{
	printk(KERN_INFO "%s: Shutting down Windows Registry Compatibility Layer\n", MODULE_NAME);
	
	/* 清理proc接口 */
	registry_proc_cleanup();
	
	/* 清理注册表树 */
	registry_tree_exit();
	
	printk(KERN_INFO "%s: Windows Registry Compatibility Layer shut down completed\n", 
	       MODULE_NAME);
}

module_init(compat_registry_init);
module_exit(compat_registry_exit);

/* 模块信息 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Kernel Compatibility");
MODULE_DESCRIPTION("Windows Registry Compatibility Layer for Linux Kernel");
MODULE_VERSION("1.0");
MODULE_ALIAS("windows-registry");
MODULE_ALIAS("wine-compat");

/* 导出符号供其他模块使用 */
EXPORT_SYMBOL_GPL(registry_tree_create);
EXPORT_SYMBOL_GPL(registry_tree_destroy);
EXPORT_SYMBOL_GPL(registry_key_create);
EXPORT_SYMBOL_GPL(registry_key_destroy);
EXPORT_SYMBOL_GPL(registry_key_open);
EXPORT_SYMBOL_GPL(registry_key_close);
EXPORT_SYMBOL_GPL(registry_value_create);
EXPORT_SYMBOL_GPL(registry_value_destroy);
EXPORT_SYMBOL_GPL(registry_value_set);
EXPORT_SYMBOL_GPL(registry_value_get);
EXPORT_SYMBOL_GPL(registry_value_remove);
EXPORT_SYMBOL_GPL(registry_value_get_string);
EXPORT_SYMBOL_GPL(registry_value_get_int);
EXPORT_SYMBOL_GPL(registry_value_get_bool);
EXPORT_SYMBOL_GPL(registry_value_get_binary);
EXPORT_SYMBOL_GPL(registry_key_iterate_values);
EXPORT_SYMBOL_GPL(registry_key_iterate_subkeys);