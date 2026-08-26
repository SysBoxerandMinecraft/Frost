// SPDX-License-Identifier: GPL-2.0
/*
 * Windows Registry Compatibility Layer for Linux Kernel
 * 
 * 实现模拟Windows注册表服务的核心功能，包括注册表树的管理、
 * 值的存储和查询等操作。
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "registry.h"

/* 内核模块参数 */
bool registry_debug = false;
int registry_max_keys = 1024;
module_param(registry_debug, bool, 0644);
module_param(registry_max_keys, int, 0644);

/* 注册表树实现 */
struct registry_tree *registry_tree_create(void)
{
	struct registry_tree *tree;
	
	tree = kzalloc(sizeof(*tree), GFP_KERNEL);
	if (!tree)
		return NULL;
	
	INIT_LIST_HEAD(&tree->all_keys);
	
	/* 创建根键 */
	tree->root = registry_key_create("", NULL);
	if (!tree->root) {
		kfree(tree);
		return NULL;
	}
	
	/* 将根键添加到所有键链表 */
	list_add(&tree->root->list, &tree->all_keys);
	
	if (registry_debug)
		printk(KERN_INFO "Registry tree created\n");
	
	return tree;
}

void registry_tree_destroy(struct registry_tree *tree)
{
	if (!tree)
		return;
	
	/* 递归销毁所有键 */
	if (tree->root) {
		registry_key_destroy(tree->root);
	}
	
	kfree(tree);
	
	if (registry_debug)
		printk(KERN_INFO "Registry tree destroyed\n");
}

struct registry_key *registry_key_create(const char *name, struct registry_key *parent)
{
	struct registry_key *key;
	
	key = kzalloc(sizeof(*key), GFP_KERNEL);
	if (!key)
		return NULL;
	
	key->name = kstrdup(name, GFP_KERNEL);
	if (!key->name) {
		kfree(key);
		return NULL;
	}
	
	INIT_LIST_HEAD(&key->values);
	INIT_LIST_HEAD(&key->subkeys);
	key->parent = parent;
	
	/* 如果有父键，将此键添加到父键的子键链表 */
	if (parent) {
		list_add(&key->list, &parent->subkeys);
	}
	
	if (registry_debug)
		printk(KERN_INFO "Registry key created: %s\n", name);
	
	return key;
}

void registry_key_destroy(struct registry_key *key)
{
	if (!key)
		return;
	
	/* 先销毁所有子键 */
	while (!list_empty(&key->subkeys)) {
		struct registry_key *subkey = list_first_entry(&key->subkeys,
		                                                struct registry_key, list);
		registry_key_destroy(subkey);
	}
	
	/* 销毁所有值 */
	while (!list_empty(&key->values)) {
		struct registry_value *value = list_first_entry(&key->values,
		                                                struct registry_value, list);
		registry_value_destroy(value);
	}
	
	kfree(key->name);
	kfree(key);
	
	if (registry_debug)
		printk(KERN_INFO "Registry key destroyed\n");
}

struct registry_key *registry_key_open(struct registry_tree *tree, const char *path)
{
	char *path_copy, *token;
	char *saveptr = NULL;
	struct registry_key *current = tree->root;
	
	if (!path || !*path)
		return current;
	
	path_copy = kstrdup(path, GFP_KERNEL);
	if (!path_copy)
		return NULL;
	
	/* 分割路径并遍历 */
	token = strtok_r(path_copy, "/", &saveptr);
	while (token != NULL) {
		struct registry_key *next = NULL;
		
		/* 在子键中查找 */
		list_for_each_entry(next, &current->subkeys, list) {
			if (strcmp(next->name, token) == 0) {
				current = next;
				break;
			}
		}
		
		/* 如果未找到子键 */
		if (next == NULL) {
			kfree(path_copy);
			return NULL;
		}
		
		token = strtok_r(NULL, "/", &saveptr);
	}
	
	kfree(path_copy);
	return current;
}

int registry_key_close(struct registry_key *key)
{
	/* 当前实现中不需要特殊操作，键在不再使用时会被销毁 */
	return 0;
}

struct registry_value *registry_value_create(struct registry_key *key, 
                                           const char *name, 
                                           enum registry_value_type type)
{
	struct registry_value *value;
	
	if (!key || !name)
		return NULL;
	
	value = kzalloc(sizeof(*value), GFP_KERNEL);
	if (!value)
		return NULL;
	
	value->name = kstrdup(name, GFP_KERNEL);
	if (!value->name) {
		kfree(value);
		return NULL;
	}
	
	value->type = type;
	INIT_LIST_HEAD(&value->list);
	
	/* 根据类型初始化数据 */
	switch (type) {
	case REG_SZ:
	case REG_EXPAND_SZ:
		value->data.sz = kstrdup("", GFP_KERNEL);
		if (!value->data.sz) {
			kfree(value->name);
			kfree(value);
			return NULL;
		}
		break;
		
	case REG_DWORD:
	case REG_DWORD_LITTLE_ENDIAN:
		value->data.dword = 0;
		break;
		
	case REG_QWORD:
	case REG_QWORD_LITTLE_ENDIAN:
		value->data.qword = 0;
		break;
		
	case REG_BINARY:
		value->data.binary = kzalloc(1, GFP_KERNEL);
		if (!value->data.binary) {
			kfree(value->name);
			kfree(value);
			return NULL;
		}
		break;
		
	case REG_MULTI_SZ:
		value->data.multi_sz = kzalloc(sizeof(char *), GFP_KERNEL);
		if (!value->data.multi_sz) {
			kfree(value->name);
			kfree(value);
			return NULL;
		}
		break;
		
	default:
		kfree(value->name);
		kfree(value);
		return NULL;
	}
	
	/* 添加到键的值链表 */
	list_add(&value->list, &key->values);
	
	if (registry_debug)
		printk(KERN_INFO "Registry value created: %s (type: %d)\n", name, type);
	
	return value;
}

void registry_value_destroy(struct registry_value *value)
{
	if (!value)
		return;
	
	kfree(value->name);
	
	/* 根据类型释放数据 */
	switch (value->type) {
	case REG_SZ:
	case REG_EXPAND_SZ:
		kfree(value->data.sz);
		break;
		
	case REG_BINARY:
		kfree(value->data.binary);
		break;
		
	case REG_MULTI_SZ:
		if (value->data.multi_sz) {
			char **multi = value->data.multi_sz;
			while (*multi) {
				kfree(*multi);
				multi++;
			}
			kfree(value->data.multi_sz);
		}
		break;
	}
	
	kfree(value);
	
	if (registry_debug)
		printk(KERN_INFO "Registry value destroyed\n");
}

int registry_value_set(struct registry_key *key, 
                      const char *name, 
                      enum registry_value_type type, 
                      const void *data)
{
	struct registry_value *value;
	
	if (!key || !name)
		return -EINVAL;
	
	/* 查找是否已存在同名值 */
	value = registry_value_get(key, name);
	if (value) {
		/* 先删除现有值 */
		registry_value_destroy(value);
	}
	
	/* 创建新值 */
	value = registry_value_create(key, name, type);
	if (!value)
		return -ENOMEM;
	
	/* 设置数据 */
	switch (type) {
	case REG_SZ:
	case REG_EXPAND_SZ:
		kfree(value->data.sz);
		value->data.sz = kstrdup(data, GFP_KERNEL);
		if (!value->data.sz) {
			registry_value_destroy(value);
			return -ENOMEM;
		}
		break;
		
	case REG_DWORD:
	case REG_DWORD_LITTLE_ENDIAN:
		if (data && sizeof(u32) <= *(size_t *)data)
			value->data.dword = *(u32 *)data;
		else
			value->data.dword = 0;
		break;
		
	case REG_QWORD:
	case REG_QWORD_LITTLE_ENDIAN:
		if (data && sizeof(u64) <= *(size_t *)data)
			value->data.qword = *(u64 *)data;
		else
			value->data.qword = 0;
		break;
		
	case REG_BINARY:
		kfree(value->data.binary);
		if (data) {
			size_t size = *(size_t *)data;
			value->data.binary = kzalloc(size, GFP_KERNEL);
			if (value->data.binary) {
				memcpy(value->data.binary, data, size);
			}
		}
		break;
		
	default:
		registry_value_destroy(value);
		return -EINVAL;
	}
	
	return 0;
}

struct registry_value *registry_value_get(struct registry_key *key, const char *name)
{
	if (!key || !name)
		return NULL;
	
	struct registry_value *value;
	list_for_each_entry(value, &key->values, list) {
		if (strcmp(value->name, name) == 0) {
			return value;
		}
	}
	
	return NULL;
}

int registry_value_remove(struct registry_key *key, const char *name)
{
	struct registry_value *value;
	
	if (!key || !name)
		return -EINVAL;
	
	value = registry_value_get(key, name);
	if (!value)
		return -ENOENT;
	
	list_del(&value->list);
	registry_value_destroy(value);
	
	return 0;
}

/* 注册表查询API实现 */
char *registry_value_get_string(struct registry_key *key, const char *name, const char *default_value)
{
	struct registry_value *value;
	
	value = registry_value_get(key, name);
	if (!value)
		return default_value ? kstrdup(default_value, GFP_KERNEL) : NULL;
	
	if (value->type == REG_SZ || value->type == REG_EXPAND_SZ)
		return kstrdup(value->data.sz, GFP_KERNEL);
	
	return default_value ? kstrdup(default_value, GFP_KERNEL) : NULL;
}

int registry_value_get_int(struct registry_key *key, const char *name, int default_value)
{
	struct registry_value *value;
	
	value = registry_value_get(key, name);
	if (!value)
		return default_value;
	
	if (value->type == REG_DWORD || value->type == REG_DWORD_LITTLE_ENDIAN)
		return (int)value->data.dword;
	
	return default_value;
}

bool registry_value_get_bool(struct registry_key *key, const char *name, bool default_value)
{
	struct registry_value *value;
	
	value = registry_value_get(key, name);
	if (!value)
		return default_value;
	
	if (value->type == REG_DWORD || value->type == REG_DWORD_LITTLE_ENDIAN)
		return value->data.dword != 0;
	
	return default_value;
}

int registry_value_get_binary(struct registry_key *key, const char *name, void *data, size_t max_size)
{
	struct registry_value *value;
	
	if (!data || max_size == 0)
		return -EINVAL;
	
	value = registry_value_get(key, name);
	if (!value)
		return -ENOENT;
	
	if (value->type != REG_BINARY)
		return -EINVAL;
	
	if (!value->data.binary)
		return 0;
	
	size_t size = max_size;
	memcpy(data, value->data.binary, size);
	return size;
}

/* 注册表遍历API实现 */
void registry_key_iterate_values(struct registry_key *key, 
                                void (*callback)(struct registry_value *, void *), 
                                void *data)
{
	if (!key || !callback)
		return;
	
	struct registry_value *value;
	list_for_each_entry(value, &key->values, list) {
		callback(value, data);
	}
}

void registry_key_iterate_subkeys(struct registry_key *key, 
                                 void (*callback)(struct registry_key *, void *), 
                                 void *data)
{
	if (!key || !callback)
		return;
	
	struct registry_key *subkey;
	list_for_each_entry(subkey, &key->subkeys, list) {
		callback(subkey, data);
	}
}

/* 注册表树全局实例 */
static struct registry_tree *registry_tree;

/* 内核模块初始化 */
int __init registry_init(void)
{
	registry_tree = registry_tree_create();
	if (!registry_tree) {
		printk(KERN_ERR "Failed to create registry tree\n");
		return -ENOMEM;
	}
	
	printk(KERN_INFO "Windows Registry Compatibility Layer initialized\n");
	return 0;
}

/* 内核模块清理 */
void __exit registry_exit(void)
{
	if (registry_tree) {
		registry_tree_destroy(registry_tree);
		registry_tree = NULL;
	}
	
	printk(KERN_INFO "Windows Registry Compatibility Layer exited\n");
}

module_init(registry_init);
module_exit(registry_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Kernel Compatibility");
MODULE_DESCRIPTION("Windows Registry Compatibility Layer for Linux Kernel");