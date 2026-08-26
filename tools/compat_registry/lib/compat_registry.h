/*
 * tools/compat_registry/lib/compat_registry.h
 * 用户态注册表访问库 - 头文件
 */
#ifndef _COMPAT_REGISTRY_H_
#define _COMPAT_REGISTRY_H_

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

char *reg_get_string(const char *path, const char *key, const char *default_val);
int reg_get_int(const char *path, const char *key, int default_val);
bool reg_get_bool(const char *path, const char *key, bool default_val);
void reg_reload_cache(void);

#ifdef __cplusplus
}
#endif

#endif /* _COMPAT_REGISTRY_H_ */