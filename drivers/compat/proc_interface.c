// SPDX-License-Identifier: GPL-2.0
/*
 * 注册表/proc接口实现
 * 
 * 提供通过/proc/compat_registry接口访问注册表数据的接口，
 * 允许用户态程序查询注册表项。
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "registry.h"
#include "proc_interface.h"

#define PROC_ENTRY_NAME "compat_registry"
#define PROC_PATH "/proc/" PROC_ENTRY_NAME

/* 全局变量 */
static struct proc_dir_entry *proc_entry;
static struct registry_tree *proc_registry_tree;

/* 注册表树存储 */
static struct registry_tree *global_registry_tree;

/* 序列化输出注册表结构 */
static void registry_key_show(struct registry_key *key, struct seq_file *seq, int indent)
{
	const char *indent_str = "  ";
	struct registry_value *value;
	struct registry_key *subkey;
	
	if (!key || !seq)
		return;
	
	/* 显示键名 */
	for (int i = 0; i < indent; i++)
		seq_puts(seq, indent_str);
	seq_printf(seq, "[%s]\n", key->name);
	
	/* 显示值 */
	registry_key_iterate_values(key, 
	    (void (*)(struct registry_value *, void *))registry_value_show_proc, seq);
	
	/* 显示子键 */
	registry_key_iterate_subkeys(key, registry_key_show, seq);
}

/* 序列化输出注册表值 */
static void registry_value_show_proc(struct registry_value *value, struct seq_file *seq)
{
	if (!value || !seq)
		return;
	
	seq_printf(seq, "  %s = ", value->name);
	
	switch (value->type) {
	case REG_SZ:
	case REG_EXPAND_SZ:
		seq_printf(seq, "\"%s\"\n", value->data.sz);
		break;
		
	case REG_DWORD:
	case REG_DWORD_LITTLE_ENDIAN:
		seq_printf(seq, "0x%08X\n", value->data.dword);
		break;
		
	case REG_QWORD:
	case REG_QWORD_LITTLE_ENDIAN:
		seq_printf(seq, "0x%016llX\n", value->data.qword);
		break;
		
	case REG_BINARY:
		if (value->data.binary) {
			seq_printf(seq, "binary[");
			/* 这里简化显示，实际应该显示十六进制 */
			seq_puts(seq, "data]\n");
		} else {
			seq_puts(seq, "null\n");
		}
		break;
		
	case REG_MULTI_SZ:
		if (value->data.multi_sz) {
			seq_puts(seq, "multi_string[");
			char **multi = value->data.multi_sz;
			while (*multi) {
				seq_printf(seq, "\"%s\"", *multi);
				multi++;
				if (*multi)
					seq_puts(seq, ", ");
			}
			seq_puts(seq, "]\n");
		} else {
			seq_puts(seq, "null\n");
		}
		break;
		
	default:
		seq_printf(seq, "unknown_type(%d)\n", value->type);
		break;
	}
}

/* /proc文件显示函数 */
static int registry_proc_show(struct seq_file *m, void *v)
{
	if (!global_registry_tree)
		return 0;
	
	/* 显示根键 */
	registry_key_show(global_registry_tree->root, m, 0);
	
	return 0;
}

/* /proc文件打开函数 */
static int registry_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, registry_proc_show, NULL);
}

/* /proc文件操作结构 */
static const struct proc_ops registry_proc_ops = {
	.proc_open = registry_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

/* 初始化/proc接口 */
static int __init proc_interface_init(void)
{
	proc_entry = proc_create(PROC_ENTRY_NAME, 0444, NULL, &registry_proc_ops);
	if (!proc_entry) {
		printk(KERN_ERR "Failed to create proc entry %s\n", PROC_ENTRY_NAME);
		return -ENOMEM;
	}
	
	printk(KERN_INFO "Registry proc interface initialized at %s\n", PROC_PATH);
	return 0;
}

/* 清理/proc接口 */
static void __exit proc_interface_exit(void)
{
	if (proc_entry) {
		proc_remove(proc_entry);
		proc_entry = NULL;
	}
	
	printk(KERN_INFO "Registry proc interface removed\n");
}

/* 注册接口初始化函数 */
int __init registry_proc_init(struct registry_tree *tree)
{
	global_registry_tree = tree;
	return proc_interface_init();
}

/* 注册接口清理函数 */
void __exit registry_proc_cleanup(void)
{
	proc_interface_exit();
	global_registry_tree = NULL;
}

/* 导出到其他模块的函数 */
EXPORT_SYMBOL(registry_proc_init);
EXPORT_SYMBOL(registry_proc_cleanup);