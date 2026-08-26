// SPDX-License-Identifier: GPL-2.0
/*
 * 配置文件解析器实现
 * 
 * 实现了.ini、.conf、.json、.yaml格式配置文件的解析功能，
 * 在内核模块加载时从/etc/registry/目录读取配置，构建注册表树。
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/dirent.h>
#include <linux/ctype.h>
#include "config_parser.h"

#define MAX_LINE_LENGTH 4096
#define MAX_PATH_LENGTH 512
#define DEFAULT_REGISTRY_DIR "/etc/registry/"

/* 通用配置解析函数 */
struct config_parse_result *parse_config_file(const char *filename, 
                                           struct registry_tree *tree)
{
	struct config_parse_result *result;
	struct config_parser_state state;
	enum config_file_type file_type;
	int ret;
	
	if (!filename || !tree)
		return NULL;
	
	result = kzalloc(sizeof(*result), GFP_KERNEL);
	if (!result)
		return NULL;
	
	result->tree = tree;
	
	/* 检测文件类型 */
	file_type = detect_config_file_type(filename);
	if (file_type == CONFIG_UNKNOWN) {
		printk(KERN_ERR "Unknown config file type: %s\n", filename);
		result->error_count++;
		return result;
	}
	
	/* 初始化解析状态 */
	memset(&state, 0, sizeof(state));
	state.type = file_type;
	state.strict_mode = false;
	
	/* 根据文件类型调用相应的解析器 */
	switch (file_type) {
	case CONFIG_INI:
		ret = parse_ini_file(filename, tree, &state);
		break;
		
	case CONFIG_CONF:
		ret = parse_conf_file(filename, tree, &state);
		break;
		
	case CONFIG_JSON:
		ret = parse_json_file(filename, tree, &state);
		break;
		
	case CONFIG_YAML:
		ret = parse_yaml_file(filename, tree, &state);
		break;
		
	default:
		ret = -EINVAL;
		break;
	}
	
	if (ret < 0) {
		printk(KERN_ERR "Failed to parse config file %s: %d\n", filename, ret);
		result->error_count++;
	} else {
		result->file_count = 1;
		result->error_count = state.error_count;
	}
	
	return result;
}

/* 解析配置目录 */
struct config_parse_result *parse_config_directory(const char *dirname, 
                                                 struct registry_tree *tree)
{
	struct config_parse_result *result;
	struct file *dir;
	struct dirent64 *dent;
	char path[MAX_PATH_LENGTH];
	int ret;
	
	if (!dirname || !tree)
		return NULL;
	
	result = kzalloc(sizeof(*result), GFP_KERNEL);
	if (!result)
		return NULL;
	
	result->tree = tree;
	
	/* 简化：暂时不支持目录遍历，只处理单个配置文件 */
	printk(KERN_WARNING "Directory traversal not fully implemented, using fixed config files only\n");
	return parse_config_file(DEFAULT_REGISTRY_DIR "registry.ini", tree);
	
	filp_close(dir, NULL);
	return result;
}

/* 释放解析结果 */
void free_config_parse_result(struct config_parse_result *result)
{
	if (result) {
		kfree(result);
	}
}

/* 检测文件类型 */
enum config_file_type detect_config_file_type(const char *filename)
{
	const char *ext;
	
	if (!filename)
		return CONFIG_UNKNOWN;
	
	/* 查找文件扩展名 */
	ext = strrchr(filename, '.');
	if (!ext)
		return CONFIG_UNKNOWN;
	
	ext++; /* 跳过. */
	
	if (strcasecmp(ext, "ini") == 0)
		return CONFIG_INI;
	else if (strcasecmp(ext, "conf") == 0 || strcasecmp(ext, "cfg") == 0)
		return CONFIG_CONF;
	else if (strcasecmp(ext, "json") == 0)
		return CONFIG_JSON;
	else if (strcasecmp(ext, "yaml") == 0 || strcasecmp(ext, "yml") == 0)
		return CONFIG_YAML;
	
	return CONFIG_UNKNOWN;
}

/* 清理字符串中的空白字符 */
char *trim_whitespace(char *str)
{
	char *start;
	char *end;
	
	if (!str)
		return NULL;
	
	/* 查找第一个非空白字符 */
	start = str;
	while (*start && isspace(*start))
		start++;
	
	/* 如果全是空白字符，返回空字符串 */
	if (!*start)
		return start;
	
	/* 查找最后一个非空白字符 */
	end = start + strlen(start) - 1;
	while (end > start && isspace(*end))
		end--;
	
	/* 终止字符串 */
	*(end + 1) = '\0';
	
	return start;
}

/* 移除注释 */
char *remove_comments(char *str, char comment_char)
{
	char *comment_pos;
	
	if (!str)
		return NULL;
	
	comment_pos = strchr(str, comment_char);
	if (comment_pos)
		*comment_pos = '\0';
	
	return str;
}

/* 解析键值对 */
int parse_key_value(const char *line, char **key, char **value)
{
	const char *eq_pos;
	char *temp_key, *temp_value;
	
	if (!line || !key || !value)
		return -EINVAL;
	
	/* 分割键值对 */
	eq_pos = strchr(line, '=');
	if (!eq_pos)
		return -EINVAL;
	
	/* 分配内存并复制键 */
	temp_key = kzalloc(eq_pos - line + 1, GFP_KERNEL);
	if (!temp_key)
		return -ENOMEM;
	strncpy(temp_key, line, eq_pos - line);
	
	/* 分配内存并复制值 */
	temp_value = kstrdup(eq_pos + 1, GFP_KERNEL);
	if (!temp_value) {
		kfree(temp_key);
		return -ENOMEM;
	}
	
	/* 移除注释并修剪空白 */
	temp_value = remove_comments(temp_value, ';');
	temp_value = trim_whitespace(temp_value);
	
	*key = temp_key;
	*value = temp_value;
	return 0;
}

/* 判断布尔值 */
bool parse_bool_value(const char *str)
{
	if (!str)
		return false;
	
	/* 空字符串或注释视为false */
	if (*str == '\0' || *str == '#' || *str == ';')
		return false;
	
	/* 检查是否为true值 */
	if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0 || 
	    strcasecmp(str, "1") == 0 || strcasecmp(str, "on") == 0)
		return true;
	
	/* 检查是否为false值 */
	if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0 || 
	    strcasecmp(str, "0") == 0 || strcasecmp(str, "off") == 0)
		return false;
	
	/* 默认行为：未被注释的行视为true */
	return true;
}

/* 创建注册表键路径 */
struct registry_key *create_registry_path(struct registry_tree *tree, 
                                       const char *path)
{
	char *path_copy, *token;
	char *saveptr = NULL;
	struct registry_key *current = tree->root;
	char key_path[MAX_PATH_LENGTH];
	
	if (!path || !*path)
		return current;
	
	path_copy = kstrdup(path, GFP_KERNEL);
	if (!path_copy)
		return NULL;
	
	/* 清理路径：去除首尾/并规范化 */
	token = trim_whitespace(path_copy);
	if (token[0] == '/')
		token++;
	
	/* 构建键路径 */
	snprintf(key_path, sizeof(key_path), "%s", token);
	
	/* 分割路径并创建键 */
	char *saveptr = NULL;
	token = strsep(&token, "/");
	while (token != NULL) {
		struct registry_key *next = NULL;
		
		/* 在子键中查找 */
		list_for_each_entry(next, &current->subkeys, list) {
			if (strcmp(next->name, token) == 0) {
				current = next;
				break;
			}
		}
		
		/* 如果未找到子键，创建新的 */
		if (next == NULL) {
			next = registry_key_create(token, current);
			if (!next) {
				kfree(path_copy);
				return NULL;
			}
		}
		
		token = strsep(&token, "/");
	}
	
	kfree(path_copy);
	return current;
}

/* INI文件解析器实现 */
int parse_ini_file(const char *filename, struct registry_tree *tree, 
                  struct config_parser_state *state)
{
	struct file *file;
	char *buf, *line, *section = NULL;
	loff_t pos = 0;
	ssize_t bytes_read;
	int ret = 0;
	
	if (!filename || !tree || !state)
		return -EINVAL;
	
	/* 打开文件 */
	file = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(file)) {
		printk(KERN_ERR "Failed to open INI file %s: %ld\n", filename, PTR_ERR(file));
		return PTR_ERR(file);
	}
	
	/* 映射文件到内存 */
	buf = kmalloc(MAX_LINE_LENGTH, GFP_KERNEL);
	if (!buf) {
		filp_close(file, NULL);
		return -ENOMEM;
	}
	
	/* 重置状态 */
	state->current_section = NULL;
	state->line_number = 0;
	state->error_count = 0;
	
	/* 读取文件 */
	while (1) {
		loff_t prev_pos = pos;
		bytes_read = kernel_read(file, buf, MAX_LINE_LENGTH - 1, &pos);
		if (bytes_read <= 0)
			break;
		
		buf[bytes_read] = '\0';
		
		/* 处理每一行 */
		line = buf;
		while (line && *line) {
			char *trimmed, *original_line;
			
			original_line = line;
			line = strchr(line, '\n');
			if (line)
				*line++ = '\0';
			
			trimmed = trim_whitespace(original_line);
			if (!trimmed || *trimmed == '\0')
				continue;
			
			state->line_number++;
			
			/* 移除注释 */
			trimmed = remove_comments(trimmed, ';');
			if (!trimmed || *trimmed == '\0')
				continue;
			
			/* 处理节 */
			if (trimmed[0] == '[') {
				char *end = strchr(trimmed, ']');
				if (end) {
					*end = '\0';
					kfree(section);
					section = kstrdup(trimmed + 1, GFP_KERNEL);
					if (!section) {
						ret = -ENOMEM;
						goto out;
					}
					state->current_section = section;
					continue;
				} else {
					printk(KERN_ERR "Invalid section format at line %d\n", 
					       state->line_number);
					state->error_count++;
					continue;
				}
			}
			
			/* 处理键值对 */
			char *key, *value;
			if (parse_key_value(trimmed, &key, &value) == 0) {
				struct registry_key *target_key;
				char *final_key;
				
				/* 根据节确定目标键 */
				if (state->current_section) {
					char path[MAX_PATH_LENGTH];
					snprintf(path, sizeof(path), "%s/%s", 
					        state->current_section, key);
					target_key = create_registry_path(tree, path);
				} else {
					target_key = create_registry_path(tree, key);
				}
				
				if (!target_key) {
					kfree(key);
					kfree(value);
					ret = -ENOMEM;
					goto out;
				}
				
				/* 判断值的类型并设置 */
				if (parse_bool_value(value)) {
					/* 布尔值，设置REG_DWORD类型 */
					u32 bool_value = parse_bool_value(value) ? 1 : 0;
					registry_value_set(target_key, value, REG_DWORD, &bool_value);
				} else if (strchr(value, '.') != NULL) {
					/* 可能是数字 */
					char *end;
					s64 num = simple_strtoll(value, &end, 0);
					if (*end == '\0') {
						/* 数字类型 */
						if (num > 0xFFFFFFFFULL) {
							u64 qword_value = (u64)num;
							registry_value_set(target_key, value, REG_QWORD, &qword_value);
						} else {
							u32 dword_value = (u32)num;
							registry_value_set(target_key, value, REG_DWORD, &dword_value);
						}
					} else {
						/* 字符串类型 */
						registry_value_set(target_key, value, REG_SZ, value);
					}
				} else {
					/* 字符串类型 */
					registry_value_set(target_key, value, REG_SZ, value);
				}
				
				kfree(key);
				kfree(value);
			} else {
				printk(KERN_ERR "Invalid key-value pair at line %d: %s\n", 
				       state->line_number, trimmed);
				state->error_count++;
			}
		}
	}
	
out:
	kfree(buf);
	kfree(section);
	filp_close(file, NULL);
	return ret;
}

/* CONF文件解析器实现（简化版） */
int parse_conf_file(const char *filename, struct registry_tree *tree, 
                  struct config_parser_state *state)
{
	/* CONF文件格式与INI类似，这里简化实现 */
	return parse_ini_file(filename, tree, state);
}

/* JSON文件解析器实现（简化版） */
int parse_json_file(const char *filename, struct registry_tree *tree, 
                  struct config_parser_state *state)
{
	/* JSON文件解析需要更复杂的实现，这里简化处理 */
	printk(KERN_INFO "JSON file parsing not fully implemented, using basic key-value parsing\n");
	return parse_ini_file(filename, tree, state);
}

/* YAML文件解析器实现（简化版） */
int parse_yaml_file(const char *filename, struct registry_tree *tree, 
                  struct config_parser_state *state)
{
	/* YAML文件解析需要更复杂的实现，这里简化处理 */
	printk(KERN_INFO "YAML file parsing not fully implemented, using basic key-value parsing\n");
	return parse_ini_file(filename, tree, state);
}