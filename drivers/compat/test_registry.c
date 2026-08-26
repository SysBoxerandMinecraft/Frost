// SPDX-License-Identifier: GPL-2.0
/*
 * Windows Registry Compatibility Layer - Test Module
 * 
 * 此模块用于测试注册表兼容层的API功能，验证基本操作是否正常工作。
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "registry.h"

#define TEST_MODULE_NAME "test_registry"

/* 测试回调函数 */
static void test_value_callback(struct registry_value *value, void *data)
{
	printk(KERN_INFO "Test: Found value - %s = ", value->name);
	
	switch (value->type) {
	case REG_SZ:
		printk("\"%s\"\n", value->data.sz);
		break;
		
	case REG_DWORD:
	case REG_DWORD_LITTLE_ENDIAN:
		printk("0x%08X\n", value->data.dword);
		break;
		
	case REG_QWORD:
	case REG_QWORD_LITTLE_ENDIAN:
		printk("0x%016llX\n", value->data.qword);
		break;
		
	default:
		printk("unknown type(%d)\n", value->type);
		break;
	}
}

static void test_key_callback(struct registry_key *key, void *data)
{
	printk(KERN_INFO "Test: Found key - [%s]\n", key->name);
}

/* 测试注册表基本操作 */
static int test_registry_operations(void)
{
	struct registry_tree *tree;
	struct registry_key *root_key, *software_key, *app_key;
	struct registry_value *value;
	u32 test_value;
	int ret = 0;
	
	printk(KERN_INFO "%s: Testing registry operations...\n", TEST_MODULE_NAME);
	
	/* 创建注册表树 */
	tree = registry_tree_create();
	if (!tree) {
		printk(KERN_ERR "%s: Failed to create registry tree\n", TEST_MODULE_NAME);
		return -ENOMEM;
	}
	
	/* 测试根键 */
	root_key = tree->root;
	printk(KERN_INFO "%s: Root key: [%s]\n", TEST_MODULE_NAME, root_key->name);
	
	/* 创建子键 */
	software_key = registry_key_create("Software", root_key);
	if (!software_key) {
		printk(KERN_ERR "%s: Failed to create Software key\n", TEST_MODULE_NAME);
		ret = -ENOMEM;
		goto cleanup;
	}
	
	app_key = registry_key_create("MyApp", software_key);
	if (!app_key) {
		printk(KERN_ERR "%s: Failed to create MyApp key\n", TEST_MODULE_NAME);
		ret = -ENOMEM;
		goto cleanup;
	}
	
	printk(KERN_INFO "%s: Created keys successfully\n", TEST_MODULE_NAME);
	
	/* 测试设置值 */
	test_value = 100;
	ret = registry_value_set(software_key, "Version", REG_DWORD, &test_value);
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to set Version value\n", TEST_MODULE_NAME);
		goto cleanup;
	}
	
	ret = registry_value_set(software_key, "Name", REG_SZ, "Test Application");
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to set Name value\n", TEST_MODULE_NAME);
		goto cleanup;
	}
	
	ret = registry_value_set(app_key, "Enabled", REG_DWORD, &(u32){1});
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to set Enabled value\n", TEST_MODULE_NAME);
		goto cleanup;
	}
	
	printk(KERN_INFO "%s: Set values successfully\n", TEST_MODULE_NAME);
	
	/* 测试获取值 */
	value = registry_value_get(software_key, "Version");
	if (value) {
		printk(KERN_INFO "%s: Got Version = 0x%08X\n", TEST_MODULE_NAME, value->data.dword);
	} else {
		printk(KERN_ERR "%s: Failed to get Version value\n", TEST_MODULE_NAME);
	}
	
	/* 测试便利函数 */
	int version = registry_value_get_int(software_key, "Version", 0);
	printk(KERN_INFO "%s: Version (int) = %d\n", TEST_MODULE_NAME, version);
	
	char *name = registry_value_get_string(software_key, "Name", "Unknown");
	if (name) {
		printk(KERN_INFO "%s: Name = \"%s\"\n", TEST_MODULE_NAME, name);
		kfree(name);
	}
	
	bool enabled = registry_value_get_bool(app_key, "Enabled", false);
	printk(KERN_INFO "%s: Enabled = %s\n", TEST_MODULE_NAME, enabled ? "true" : "false");
	
	/* 测试遍历 */
	printk(KERN_INFO "%s: Traversing Software key values:\n", TEST_MODULE_NAME);
	registry_key_iterate_values(software_key, test_value_callback, NULL);
	
	printk(KERN_INFO "%s: Traversing root key subkeys:\n", TEST_MODULE_NAME);
	registry_key_iterate_subkeys(root_key, test_key_callback, NULL);
	
	printk(KERN_INFO "%s: All tests completed successfully\n", TEST_MODULE_NAME);

cleanup:
	if (tree) {
		registry_tree_destroy(tree);
	}
	
	return ret;
}

/* 测试模块初始化 */
static int __init test_registry_init(void)
{
	int ret;
	
	printk(KERN_INFO "%s: Starting registry compatibility tests\n", TEST_MODULE_NAME);
	
	/* 运行基本操作测试 */
	ret = test_registry_operations();
	if (ret < 0) {
		printk(KERN_ERR "%s: Registry tests failed\n", TEST_MODULE_NAME);
		return ret;
	}
	
	printk(KERN_INFO "%s: Registry compatibility tests completed successfully\n", TEST_MODULE_NAME);
	return 0;
}

/* 测试模块清理 */
static void __exit test_registry_exit(void)
{
	printk(KERN_INFO "%s: Registry compatibility tests unloaded\n", TEST_MODULE_NAME);
}

module_init(test_registry_init);
module_exit(test_registry_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Kernel Compatibility");
MODULE_DESCRIPTION("Test Module for Windows Registry Compatibility Layer");
MODULE_VERSION("1.0");