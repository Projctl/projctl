#pragma once

#include <optional>
#include <string>
#include <filesystem>

namespace SystemInterface {
	[[nodiscard]] std::optional<std::string> run(const std::string&);
	int run_interactive(const std::string&);
	[[nodiscard]] std::filesystem::path config_home();
	[[nodiscard]] std::filesystem::path data_home();
	[[nodiscard]] std::filesystem::path cache_home();
};
