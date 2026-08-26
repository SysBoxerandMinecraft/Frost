// SPDX-License-Identifier: GPL-2.0
/*
 * explorer - Explorer for Linux (Windows Explorer clone)
 * 
 * A simple file manager that mimics Windows Explorer functionality.
 * This is a kernel tool that reads configuration from /proc/compat_registry.
 *
 * Copyright (c) 2024 Linux Kernel Community
 * Based on original Explorer-for-Linux implementation
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "../lib/compat_registry.h"

// Qt-based file manager window (simplified version)
class ExplorerWindow {
private:
	std::string config_file;
	bool show_hidden;
	std::string sort_column;
	bool sort_ascending;
	
public:
	ExplorerWindow(const std::string& config_path) 
		: config_file(config_path), show_hidden(false), 
		  sort_column("name"), sort_ascending(true) {
		loadConfiguration();
	}
	
private:
	void loadConfiguration() {
		/* Load configuration from registry */
		show_hidden = reg_get_bool("Software\\Explorer", "ShowHidden", false);
		sort_column = reg_get_string("Software\\Explorer", "SortColumn", "name");
		sort_ascending = reg_get_bool("Software\\Explorer", "SortAscending", true);
		
		std::cout << "Configuration loaded from registry:" << std::endl;
		std::cout << "  Show hidden: " << (show_hidden ? "true" : "false") << std::endl;
		std::cout << "  Sort by: " << sort_column << std::endl;
		std::cout << "  Sort order: " << (sort_ascending ? "ascending" : "descending") << std::endl;
	}
	
	void updateFileList() {
		/* Simulate updating file list */
		std::cout << "Updating file list with show_hidden=" 
			  << (show_hidden ? "true" : "false") << std::endl;
	}
	
public:
	void run() {
		std::cout << "Starting Explorer for Linux..." << std::endl;
		std::cout << "Configuration now loaded from registry: HKLM\\Software\\Explorer" << std::endl;
		
		/* Simple UI simulation */
		std::cout << "\n=== Explorer Interface ===" << std::endl;
		std::cout << "Hidden files: " << (show_hidden ? "Shown" : "Hidden") << std::endl;
		std::cout << "Sort by: " << sort_column << " (" << (sort_ascending ? "ASC" : "DESC") << ")" << std::endl;
		updateFileList();
	}
};

int main(int argc, char *argv[]) {
	/* Configuration file path - deprecated but kept for compatibility */
	std::string config_file;
	const char *home = getenv("HOME");
	if (home) {
		config_file = std::string(home) + "/.config/explorer.conf";
	} else {
		config_file = "/etc/explorer.conf";
	}
	
	/* Parse command line arguments */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
			config_file = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
			std::cout << "Options:" << std::endl;
			std::cout << "  --config <file>  Configuration file path (deprecated)" << std::endl;
			std::cout << "  -h, --help      Show this help" << std::endl;
			return 0;
		}
	}
	
	std::cout << "Configuration file (deprecated): " << config_file << std::endl;
	
	ExplorerWindow window(config_file);
	window.run();
	
	return 0;
}