#pragma once

#include <optional>
#include <string>
#include <filesystem>

class SystemInterface {
	public:
		std::optional<std::string> run(const std::string&);
		int run_interactive(const std::string&);
		std::filesystem::path config_home();
		std::filesystem::path cache_home();
};
