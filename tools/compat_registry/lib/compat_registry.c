/*
 * tools/compat_registry/lib/compat_registry.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>

#include "compat_registry.h"

#define PROC_PATH "/proc/compat_registry"
#define MAX_LINE 4096
#define MAX_DEPTH 64

static char *cached_content = NULL;

static char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

static int get_indent_level(const char *line) {
    int level = 0;
    while (*line == ' ') {
        level++;
        line++;
    }
    return level / 4;
}

static void parse_key_value(const char *line, char *key, char *value) {
    char *equal = strchr(line, '=');
    if (!equal) {
        if (line[0] == '[') {
            const char *end = strchr(line, ']');
            if (end) {
                size_t len = end - line - 1;
                strncpy(key, line + 1, len);
                key[len] = '\0';
                value[0] = '\0';
            }
        }
        return;
    }

    *equal = '\0';
    strcpy(key, trim(line));
    strcpy(value, trim(equal + 1));

    size_t vlen = strlen(value);
    if (vlen >= 2 && ((value[0] == '"' && value[vlen-1] == '"') ||
                      (value[0] == '\'' && value[vlen-1] == '\''))) {
        memmove(value, value + 1, vlen - 2);
        value[vlen - 2] = '\0';
    }
}

static void load_cache(void) {
    FILE *fp = fopen(PROC_PATH, "r");
    if (!fp) {
        cached_content = strdup("");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    cached_content = malloc(size + 1);
    if (!cached_content) {
        fclose(fp);
        return;
    }
    size_t nread = fread(cached_content, 1, size, fp);
    cached_content[nread] = '\0';
    fclose(fp);
}
	
	registry_loaded = true;
	return 0;
}

/**
 * normalize_path() - Convert backslash path to forward slash for internal processing
 * @path: Path with backslashes
 * 
 * Returns: Forward-slash path (caller must free)
 */
static char *normalize_path(const char *path)
{
	if (!path) {
		return NULL;
	}
	
	size_t len = strlen(path);
	char *normalized = malloc(len + 1);
	if (!normalized) {
		return NULL;
	}
	
	for (size_t i = 0; i < len; i++) {
		normalized[i] = (path[i] == '\\') ? '/' : path[i];
	}
	normalized[len] = '\0';
	
	return normalized;
}

/**
 * find_registry_value() - Find a value in the registry content
 * @normalized_path: Normalized path
 * @key: Key name
 * @value: Buffer to store found value
 * @value_size: Size of value buffer
 * 
 * Returns: 1 if found, 0 if not found
 */
static int find_registry_value(const char *normalized_path, const char *key, 
			      char *value, size_t value_size)
{
	if (!normalized_path || !key || !value || value_size == 0) {
		return 0;
	}
	
	char path_copy[512];
	strncpy(path_copy, normalized_path, sizeof(path_copy) - 1);
	path_copy[sizeof(path_copy) - 1] = '\0';
	
	/* Find the path in registry content */
	char *path_pos = strstr(registry_content, path_copy);
	if (!path_pos) {
		return 0;
	}
	
	/* Look for key-value pair after this path */
	char *line_start = path_pos;
	char *line_end;
	
	/* Find the line containing our key */
	while ((line_end = strchr(line_start, '\n')) != NULL) {
		/* Check if this line contains our key */
		char *key_pos = strstr(line_start, key);
		if (key_pos && key_pos < line_end) {
			/* Verify it's not part of a larger word */
			char *key_start = key_pos;
			while (key_start > line_start && !isspace(key_start[-1])) {
				key_start--;
			}
			
			if (strncmp(key_start, key, strlen(key)) == 0) {
				/* Find the '=' character */
				char *equal_pos = strchr(key_pos, '=');
				if (equal_pos && equal_pos < line_end) {
					/* Skip whitespace after '=' */
					char *value_start = equal_pos + 1;
					while (value_start < line_end && isspace(*value_start)) {
						value_start++;
					}
					
					/* Copy value until end of line */
					size_t value_len = 0;
					char *value_end = value_start;
					while (value_end < line_end && !isspace(*value_end)) {
						value_end++;
						value_len++;
					}
					
					/* Extract value, handling quotes */
					if (value_len > 0 && *value_start == '"') {
						value_start++;
						value_len--;
					}
					if (value_len > 0 && value_start[value_len - 1] == '"') {
						value_len--;
					}
					
					/* Copy to output buffer */
					if (value_len < value_size) {
						memcpy(value, value_start, value_len);
						value[value_len] = '\0';
						return 1;
					}
				}
			}
		}
		line_start = line_end + 1;
		if (line_start - registry_content > MAX_REGISTRY_CONTENT_SIZE) {
			break;
		}
	}
	
	return 0;
}

/**
 * parse_bool_value() - Parse boolean value from string
 * @value: String value to parse
 * 
 * Returns: Parsed boolean value
 */
static bool parse_bool_value(const char *value)
{
	if (!value) {
		return false;
	}
	
	/* Check for various true representations */
	if (strcasecmp(value, "true") == 0 || 
	    strcasecmp(value, "yes") == 0 || 
	    strcasecmp(value, "1") == 0 ||
	    strcmp(value, "1") == 0) {
		return true;
	}
	
	/* Check for various false representations */
	if (strcasecmp(value, "false") == 0 || 
	    strcasecmp(value, "no") == 0 || 
	    strcasecmp(value, "0") == 0 ||
	    strcmp(value, "0") == 0) {
		return false;
	}
	
	/* Default to false */
	return false;
}

/**
 * parse_int_value() - Parse hex or decimal integer value
 * @value: String value to parse
 * 
 * Returns: Parsed integer value
 */
static int parse_int_value(const char *value)
{
	if (!value) {
		return 0;
	}
	
	/* Check for hex format (0x prefix) */
	if (strlen(value) > 2 && value[0] == '0' && value[1] == 'x') {
		return (int)strtol(value, NULL, 16);
	}
	
	/* Parse as decimal */
	return atoi(value);
}

char *reg_get_string(const char *path, const char *key, const char *default_val)
{
	if (!key) {
		return default_val ? strdup(default_val) : NULL;
	}
	
	/* Load registry content if not already loaded */
	if (!registry_loaded && load_registry_content() != 0) {
		return default_val ? strdup(default_val) : NULL;
	}
	
	/* Normalize path */
	char *normalized_path = normalize_path(path);
	if (!normalized_path) {
		return default_val ? strdup(default_val) : NULL;
	}
	
	/* Find value in registry */
	char value_buffer[1024] = {0};
	if (find_registry_value(normalized_path, key, value_buffer, sizeof(value_buffer))) {
		free(normalized_path);
		return strdup(value_buffer);
	}
	
	/* Free and return default */
	free(normalized_path);
	return default_val ? strdup(default_val) : NULL;
}

int reg_get_int(const char *path, const char *key, int default_val)
{
	if (!key) {
		return default_val;
	}
	
	/* Load registry content if not already loaded */
	if (!registry_loaded && load_registry_content() != 0) {
		return default_val;
	}
	
	/* Normalize path */
	char *normalized_path = normalize_path(path);
	if (!normalized_path) {
		return default_val;
	}
	
	/* Find value in registry */
	char value_buffer[1024] = {0};
	if (find_registry_value(normalized_path, key, value_buffer, sizeof(value_buffer))) {
		int result = parse_int_value(value_buffer);
		free(normalized_path);
		return result;
	}
	
	/* Free and return default */
	free(normalized_path);
	return default_val;
}

bool reg_get_bool(const char *path, const char *key, bool default_val)
{
	if (!key) {
		return default_val;
	}
	
	/* Load registry content if not already loaded */
	if (!registry_loaded && load_registry_content() != 0) {
		return default_val;
	}
	
	/* Normalize path */
	char *normalized_path = normalize_path(path);
	if (!normalized_path) {
		return default_val;
	}
	
	/* Find value in registry */
	char value_buffer[1024] = {0};
	if (find_registry_value(normalized_path, key, value_buffer, sizeof(value_buffer))) {
		bool result = parse_bool_value(value_buffer);
		free(normalized_path);
		return result;
	}
	
	/* Free and return default */
	free(normalized_path);
	return default_val;
}

void reg_free_string(char *str)
{
	free(str);
}