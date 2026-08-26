/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 配置文件解析器头文件
 * 
 * 定义了用于解析.ini、.conf、.json、.yaml格式配置文件的接口，
 * 在内核模块加载时从/etc/registry/目录读取配置，构建注册表树。
 */

#ifndef _COMPAT_CONFIG_PARSER_H
#define _COMPAT_CONFIG_PARSER_H

#include <linux/types.h>
#include "registry.h"

/* 配置文件类型枚举 */
enum config_file_type {
	CONFIG_INI,    /* INI格式 */
	CONFIG_CONF,   /* CONF格式 */
	CONFIG_JSON,   /* JSON格式 */
	CONFIG_YAML,   /* YAML格式 */
	CONFIG_UNKNOWN /* 未知格式 */
};

/* 配置解析状态结构 */
struct config_parser_state {
	enum config_file_type type;    /* 文件类型 */
	const char *current_section;  /* 当前节名称 */
	int line_number;               /* 当前行号 */
	int error_count;               /* 错误计数 */
	bool strict_mode;              /* 严格模式 */
};

/* 配置解析结果 */
struct config_parse_result {
	struct registry_tree *tree;    /* 解析后的注册表树 */
	int file_count;                /* 解析的文件数量 */
	int key_count;                 /* 创建的键数量 */
	int value_count;               /* 创建的值数量 */
	int error_count;               /* 错误数量 */
};

/* 解析配置文件 */
extern struct config_parse_result *parse_config_file(const char *filename, 
                                                   struct registry_tree *tree);

/* 解析配置目录 */
extern struct config_parse_result *parse_config_directory(const char *dirname, 
                                                         struct registry_tree *tree);

/* 释放解析结果 */
extern void free_config_parse_result(struct config_parse_result *result);

/* INI文件解析器 */
extern int parse_ini_file(const char *filename, struct registry_tree *tree, 
                         struct config_parser_state *state);

/* CONF文件解析器 */
extern int parse_conf_file(const char *filename, struct registry_tree *tree, 
                         struct config_parser_state *state);

/* JSON文件解析器 */
extern int parse_json_file(const char *filename, struct registry_tree *tree, 
                         struct config_parser_state *state);

/* YAML文件解析器 */
extern int parse_yaml_file(const char *filename, struct registry_tree *tree, 
                         struct config_parser_state *state);

/* 辅助函数：检测文件类型 */
extern enum config_file_type detect_config_file_type(const char *filename);

/* 辅助函数：清理字符串中的空白字符 */
extern char *trim_whitespace(char *str);

/* 辅助函数：移除注释 */
extern char *remove_comments(char *str, char comment_char);

/* 辅助函数：解析键值对 */
extern int parse_key_value(const char *line, char **key, char **value);

/* 辅助函数：判断布尔值 */
extern bool parse_bool_value(const char *str);

/* 辅助函数：创建注册表键路径 */
extern struct registry_key *create_registry_path(struct registry_tree *tree, 
                                               const char *path);

#endif /* _COMPAT_CONFIG_PARSER_H */