// SPDX-License-Identifier: GPL-2.0
/*
 * cmd - Windows Command Prompt Clone for Linux
 * 
 * A simple command prompt emulator that mimics basic Windows cmd functionality.
 * This is a kernel tool that reads configuration from /proc/compat_registry.
 *
 * Copyright (c) 2024 Linux Kernel Community
 * Based on original Windows cmd implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../lib/compat_registry.h"

/* Default configuration values */
#define DEFAULT_MAX_HISTORY 100
#define DEFAULT_PROMPT "CMD>"
#define DEFAULT_AUTO_RUN "/etc/profile"

/* Configuration variables - populated from registry */
static int max_history = DEFAULT_MAX_HISTORY;
static char *prompt = NULL;
static char *auto_run = NULL;

/* Load configuration from registry */
static void load_config(void)
{
	/* Load configuration from registry */
	max_history = reg_get_int("Software\\CMD", "MaxHistory", DEFAULT_MAX_HISTORY);
	
	char *prompt_temp = reg_get_string("Software\\CMD", "Prompt", DEFAULT_PROMPT);
	if (prompt) free(prompt);
	prompt = prompt_temp;
	
	char *auto_run_temp = reg_get_string("Software\\CMD", "AutoRun", DEFAULT_AUTO_RUN);
	if (auto_run) free(auto_run);
	auto_run = auto_run_temp;
	
	printf("Loaded configuration from registry:\n");
	printf("  Max history: %d\n", max_history);
	printf("  Prompt: %s\n", prompt);
	printf("  Auto run: %s\n", auto_run);
}

/* Show help */
static void show_help(void)
{
	printf("Windows Command Prompt Clone\n");
	printf("Usage: cmd [options]\n");
	printf("Options:\n");
	printf("  -h, --help      Show this help\n");
	printf("  -H, --history   Set maximum history size\n");
	printf("  -p, --prompt    Set command prompt\n");
	printf("\nConfiguration is loaded from registry: HKLM\\Software\\CMD\n");
}

int main(int argc, char *argv[])
{
	int i;
	
	/* Parse command line arguments */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			show_help();
			return 0;
		} else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--history") == 0) {
			if (i + 1 < argc) {
				max_history = atoi(argv[i + 1]);
				printf("Set max_history to %d (will not persist to registry)\n", max_history);
				i++;
			}
		} else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--prompt") == 0) {
			if (i + 1 < argc) {
				if (prompt) free(prompt);
				prompt = strdup(argv[i + 1]);
				printf("Set prompt to: %s (will not persist to registry)\n", prompt);
				i++;
			}
		}
	}
	
	/* Load configuration from registry */
	load_config();
	
	/* Set defaults if not configured */
	if (!prompt) {
		prompt = strdup(DEFAULT_PROMPT);
	}
	if (!auto_run) {
		auto_run = strdup(DEFAULT_AUTO_RUN);
	}
	
	printf("Command Prompt Clone initialized\n");
	printf("Max history: %d\n", max_history);
	printf("Prompt: %s\n", prompt);
	printf("Auto run: %s\n", auto_run);
	
	/* Simple command loop simulation */
	printf("\n%s ", prompt);
	printf("Running command loop (simulation)...\n");
	
	/* Execute auto-run script if it exists */
	if (auto_run) {
		printf("Executing auto-run: %s\n", auto_run);
	}
	
	/* Cleanup */
	if (prompt) free(prompt);
	if (auto_run) free(auto_run);
	
	return 0;
}