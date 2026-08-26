/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Windows Registry Compatibility Layer for Linux Kernel
 * 
 * 此文件定义了模拟Windows注册表服务的核心数据结构和函数接口。
 * 为其他内核模块提供注册表查询和设置功能。
 */

#ifndef _COMPAT_REGISTRY_H
#define _COMPAT_REGISTRY_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>

/* 注册表值类型 */
enum registry_value_type {
	REG_NONE = 0,        /* 无类型 */
	REG_SZ = 1,         /* 字符串 */
	REG_EXPAND_SZ = 2,  /* 可扩展字符串 */
	REG_BINARY = 3,     /* 二进制数据 */
	REG_DWORD = 4,     /* 32位整数 */
	REG_DWORD_LITTLE_ENDIAN = 4,  /* 与REG_DWORD相同 */
	REG_DWORD_BIG_ENDIAN = 5,
	REG_LINK = 6,      /* 符号链接 */
	REG_MULTI_SZ = 7,   /* 多字符串 */
	REG_RESOURCE_LIST = 8,
	REG_FULL_RESOURCE_DESCRIPTOR = 9,
	REG_RESOURCE_REQUIREMENTS_LIST = 10,
	REG_QWORD = 11,    /* 64位整数 */
	REG_QWORD_LITTLE_ENDIAN = 11,  /* 与REG_QWORD相同 */
	REG_MAX = 11
};

/* 注册表值结构 */
struct registry_value {
	char *name;                    /* 值名称 */
	enum registry_value_type type; /* 值类型 */
	union {
		char *sz;                 /* 字符串 */
		u32 dword;                /* 32位整数 */
		u64 qword;                /* 64位整数 */
		void *binary;             /* 二进制数据 */
		char **multi_sz;          /* 多字符串数组 */
	} data;
	struct list_head list;         /* 链表节点 */
};

/* 注册表键结构 */
struct registry_key {
	char *name;                    /* 键名称 */
	struct list_head values;       /* 值链表 */
	struct list_head subkeys;      /* 子键链表 */
	struct registry_key *parent;   /* 父键指针 */
	struct list_head list;         /* 链表节点 */
};

/* 注册表树结构 */
struct registry_tree {
	struct registry_key *root;     /* 根键 */
	struct list_head all_keys;      /* 所有键的链表（用于快速查找） */
};

/* 注册表操作API */
extern struct registry_tree *registry_tree_create(void);
extern void registry_tree_destroy(struct registry_tree *tree);
extern struct registry_key *registry_key_create(const char *name, struct registry_key *parent);
extern void registry_key_destroy(struct registry_key *key);
extern struct registry_key *registry_key_open(struct registry_tree *tree, const char *path);
extern int registry_key_close(struct registry_key *key);
extern struct registry_value *registry_value_create(struct registry_key *key, 
                                                   const char *name, 
                                                   enum registry_value_type type);
extern void registry_value_destroy(struct registry_value *value);
extern int registry_value_set(struct registry_key *key, 
                             const char *name, 
                             enum registry_value_type type, 
                             const void *data);
extern struct registry_value *registry_value_get(struct registry_key *key, const char *name);
extern int registry_value_remove(struct registry_key *key, const char *name);

/* 注册表查询API */
extern char *registry_value_get_string(struct registry_key *key, const char *name, const char *default_value);
extern int registry_value_get_int(struct registry_key *key, const char *name, int default_value);
extern bool registry_value_get_bool(struct registry_key *key, const char *name, bool default_value);
extern int registry_value_get_binary(struct registry_key *key, const char *name, void *data, size_t max_size);

/* 注册表遍历API */
extern void registry_key_iterate_values(struct registry_key *key, 
                                      void (*callback)(struct registry_value *, void *), 
                                      void *data);
extern void registry_key_iterate_subkeys(struct registry_key *key, 
                                       void (*callback)(struct registry_key *, void *), 
                                       void *data);

/* 内核模块初始化和清理 */
extern int registry_init(void);
extern void registry_exit(void);

/* 内核模块参数 */
extern bool registry_debug;
extern int registry_max_keys;

#endif /* _COMPAT_REGISTRY_H */