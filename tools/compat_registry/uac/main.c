// SPDX-License-Identifier: GPL-2.0
/*
 * uac - Linux UAC (User Account Control)
 * 
 * A simple implementation of User Account Control for Linux.
 * This is a kernel tool that reads configuration from /proc/compat_registry.
 *
 * Copyright (c) 2024 Linux Kernel Community
 * Based on original Linux_uac implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "../lib/compat_registry.h"

/* Default configuration values */
#define DEFAULT_PROMPT_TIMEOUT 30
#define DEFAULT_PROMPT "User Account Control"
#define DEFAULT_MAX_PASSWORD_ATTEMPTS 3
#define DEFAULT_MIN_PASSWORD_LENGTH 8
#define DEFAULT_LOG_FILE "/var/log/uac.log"

/* Configuration structure */
typedef struct {
	int prompt_timeout;
	char *prompt_text;
	int max_password_attempts;
	int min_password_length;
	bool require_admin_approval;
	char *allowed_programs;
	char *blocked_programs;
	bool logging_enabled;
	char *log_file;
} uac_config_t;

/* Global configuration */
static uac_config_t config = {
	.prompt_timeout = DEFAULT_PROMPT_TIMEOUT,
	.prompt_text = NULL,
	.max_password_attempts = DEFAULT_MAX_PASSWORD_ATTEMPTS,
	.min_password_length = DEFAULT_MIN_PASSWORD_LENGTH,
	.require_admin_approval = true,
	.allowed_programs = NULL,
	.blocked_programs = NULL,
	.logging_enabled = true,
	.log_file = NULL,
};

/* Load configuration from registry */
static void load_config(void)
{
	/* Load configuration from registry */
	config.prompt_timeout = reg_get_int("Software\\UAC", "PromptTimeout", DEFAULT_PROMPT_TIMEOUT);
	
	char *prompt_temp = reg_get_string("Software\\UAC", "PromptText", DEFAULT_PROMPT);
	if (config.prompt_text) free(config.prompt_text);
	config.prompt_text = prompt_temp;
	
	config.max_password_attempts = reg_get_int("Software\\UAC", "MaxPasswordAttempts", DEFAULT_MAX_PASSWORD_ATTEMPTS);
	config.min_password_length = reg_get_int("Software\\UAC", "MinPasswordLength", DEFAULT_MIN_PASSWORD_LENGTH);
	config.require_admin_approval = reg_get_bool("Software\\UAC", "RequireAdminApproval", true);
	
	char *allowed_temp = reg_get_string("Software\\UAC", "AllowedPrograms", "");
	if (config.allowed_programs) free(config.allowed_programs);
	config.allowed_programs = allowed_temp;
	
	char *blocked_temp = reg_get_string("Software\\UAC", "BlockedPrograms", "");
	if (config.blocked_programs) free(config.blocked_programs);
	config.blocked_programs = blocked_temp;
	
	config.logging_enabled = reg_get_bool("Software\\UAC", "LoggingEnabled", true);
	
	char *log_file_temp = reg_get_string("Software\\UAC", "LogFile", DEFAULT_LOG_FILE);
	if (config.log_file) free(config.log_file);
	config.log_file = log_file_temp;
	
	/* Set defaults if not configured */
	if (!config.prompt_text) {
		config.prompt_text = strdup(DEFAULT_PROMPT);
	}
	if (!config.allowed_programs) {
		config.allowed_programs = strdup("");
	}
	if (!config.blocked_programs) {
		config.blocked_programs = strdup("");
	}
	if (!config.log_file) {
		config.log_file = strdup(DEFAULT_LOG_FILE);
	}
	
	printf("Configuration loaded from registry:\n");
	printf("  Prompt timeout: %d\n", config.prompt_timeout);
	printf("  Prompt text: %s\n", config.prompt_text);
	printf("  Max password attempts: %d\n", config.max_password_attempts);
	printf("  Min password length: %d\n", config.min_password_length);
	printf("  Require admin approval: %s\n", config.require_admin_approval ? "true" : "false");
	printf("  Logging enabled: %s\n", config.logging_enabled ? "true" : "false");
}

/* Log event */
static void log_event(const char *message)
{
	if (!config.logging_enabled) {
		return;
	}
	
	FILE *fp = fopen(config.log_file, "a");
	if (!fp) {
		return;
	}
	
	time_t now;
	time(&now);
	char *time_str = ctime(&now);
	time_str[strcspn(time_str, "\n")] = '\0';
	
	fprintf(fp, "[%s] %s\n", time_str, message);
	fclose(fp);
}

/* Check if program is allowed */
static bool is_program_allowed(const char *program)
{
	if (strlen(config.allowed_programs) == 0) {
		return true;
	}
	return (strstr(config.allowed_programs, program) != NULL);
}

/* Check if program is blocked */
static bool is_program_blocked(const char *program)
{
	return (strstr(config.blocked_programs, program) != NULL);
}

/* Show UAC prompt simulation */
static bool show_uac_prompt(const char *program_name)
{
	printf("\n=== %s ===\n", config.prompt_text);
	printf("An administrator has requested permission to run:\n");
	printf("  Program: %s\n", program_name);
	printf("  Timeout: %d seconds\n", config.prompt_timeout);
	
	if (config.require_admin_approval) {
		printf("Administrator password required:\n");
		
		char password[256];
		int attempts = 0;
		
		while (attempts < config.max_password_attempts) {
			printf("Enter password (attempt %d/%d): ", attempts + 1, config.max_password_attempts);
			fflush(stdout);
			
			if (fgets(password, sizeof(password), stdin)) {
				password[strcspn(password, "\n")] = '\0';
				
				if (strlen(password) >= config.min_password_length) {
					printf("Access granted!\n");
					log_event("UAC approval for program");
					return true;
				} else {
					printf("Invalid password (minimum %d characters)\n", config.min_password_length);
					attempts++;
				}
			}
		}
		
		printf("Maximum password attempts reached. Access denied.\n");
		log_event("UAC access denied - too many attempts");
		return false;
	} else {
		printf("Do you want to allow this program to run? (y/N): ");
		char response[10];
		if (fgets(response, sizeof(response), stdin)) {
			if (response[0] == 'y' || response[0] == 'Y') {
				printf("Access granted!\n");
				log_event("UAC approval for program");
				return true;
			}
		}
		printf("Access denied.\n");
		log_event("UAC access denied - user denied");
		return false;
	}
}

/* Show help */
static void show_help(void)
{
	printf("Linux User Account Control\n");
	printf("Usage: uac [options] <program>\n");
	printf("Options:\n");
	printf("  -h, --help              Show this help\n");
	printf("  -t, --timeout <sec>     Set prompt timeout\n");
	printf("  -a, --admin             Require admin approval\n");
	printf("  -n, --no-admin          Don't require admin approval\n");
	printf("\nConfiguration now loaded from registry: HKLM\\Software\\UAC\n");
}

int main(int argc, char *argv[])
{
	char *program_to_run = NULL;
	
	/* Load configuration */
	load_config();
	
	/* Parse command line arguments */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			show_help();
			return 0;
		} else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) {
			if (i + 1 < argc) {
				config.prompt_timeout = atoi(argv[i + 1]);
				printf("Set timeout to %d seconds (will not persist to registry)\n", config.prompt_timeout);
				i++;
			}
		} else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--admin") == 0) {
			config.require_admin_approval = true;
			printf("Admin approval required (will not persist to registry)\n");
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-admin") == 0) {
			config.require_admin_approval = false;
			printf("Admin approval not required (will not persist to registry)\n");
		} else if (argv[i][0] != '-') {
			/* Assume it's the program to run */
			program_to_run = argv[i];
		}
	}
	
	if (!program_to_run) {
		printf("Error: No program specified\n");
		show_help();
		return 1;
	}
	
	printf("Linux UAC started\n");
	printf("Configuration now loaded from registry: HKLM\\Software\\UAC\n");
	printf("Prompt timeout: %d seconds\n", config.prompt_timeout);
	printf("Require admin approval: %s\n", config.require_admin_approval ? "yes" : "no");
	
	/* Check if program is blocked */
	if (is_program_blocked(program_to_run)) {
		printf("ERROR: Program '%s' is blocked by UAC policy\n", program_to_run);
		log_event("UAC blocked program");
		return 1;
	}
	
	/* Check if program is allowed */
	if (!is_program_allowed(program_to_run)) {
		printf("WARNING: Program '%s' is not in allowed list\n", program_to_run);
	}
	
	/* Show UAC prompt */
	bool approved = show_uac_prompt(program_to_run);
	
	if (approved) {
		printf("Executing: %s\n", program_to_run);
		log_event("Program executed with UAC approval");
	} else {
		printf("Program execution denied\n");
		log_event("Program execution denied by UAC");
	}
	
	/* Cleanup */
	if (config.prompt_text) free(config.prompt_text);
	if (config.allowed_programs) free(config.allowed_programs);
	if (config.blocked_programs) free(config.blocked_programs);
	if (config.log_file) free(config.log_file);
	
	return approved ? 0 : 1;
}