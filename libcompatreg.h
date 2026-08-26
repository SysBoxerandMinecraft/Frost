#ifndef LIBCOMPATREG_H
#define LIBCOMPATREG_H

#include <stdbool.h>
#include <stdio.h>

/*
 * libcompatreg - Lightweight registry access library for Linux
 * 
 * This library provides a simple interface to read configuration values
 * from the kernel compatibility registry module through /proc/compat_registry.
 */

/**
 * @brief Get a string value from the registry
 * 
 * @param path Registry path (e.g. "Software\\Cmd\\AutoRun")
 * @param key Key name to retrieve
 * @param default_val Default value if key not found or invalid
 * @return String value (caller must free with reg_free_string())
 */
char* reg_get_string(const char* path, const char* key, const char* default_val);

/**
 * @brief Get an integer value from the registry
 * 
 * @param path Registry path (e.g. "Software\\Explorer\\MaxFiles")
 * @param key Key name to retrieve
 * @param default_val Default value if key not found or invalid
 * @return Integer value
 */
int reg_get_int(const char* path, const char* key, int default_val);

/**
 * @brief Get a boolean value from the registry
 * 
 * @param path Registry path (e.g. "Software\\Explorer\\ShowHidden")
 * @param key Key name to retrieve
 * @param default_val Default value if key not found or invalid
 * @return Boolean value
 */
bool reg_get_bool(const char* path, const char* key, bool default_val);

/**
 * @brief Free a string returned by reg_get_string
 * 
 * @param str String to free
 */
void reg_free_string(char* str);

#endif // LIBCOMPATREG_H