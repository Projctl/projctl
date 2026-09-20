#pragma once

#include <optional>
#include <string>
#include <filesystem>

class SystemInterface {
	public:
		std::optional<std::string> run_command(const std::string&);
		int run_interactive(const std::string&);
		std::filesystem::path get_config_home();
};
