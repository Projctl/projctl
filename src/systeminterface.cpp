#include "systeminterface.hpp"

#include <cstdlib>

std::optional<std::string> SystemInterface::run(const std::string &command) {
	FILE *pipe = popen(command.c_str(), "r");
	if (!pipe) [[unlikely]] return std::nullopt;

	std::string output;
	char buffer[256];

	while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
		output += buffer;

	int status = pclose(pipe);
	if (status != 0) [[unlikely]] return std::nullopt;
	if (!output.empty() && output.back() == '\n') output.pop_back();
	return output;
}

int SystemInterface::run_interactive(const std::string &command) { return std::system(command.c_str()); }

std::filesystem::path SystemInterface::config_home() {
	const char* xdg = std::getenv("XDG_CONFIG_HOME");

	if (xdg && *xdg != '\0') return std::filesystem::path{xdg};

	const char* home = std::getenv("HOME");

	if (home && *home != '\0') return std::filesystem::path{home}/".config";

	return {};
}

std::filesystem::path SystemInterface::cache_home() {
	const char* xdg = std::getenv("XDG_CACHE_HOME");

	if (xdg && *xdg != '\0') return std::filesystem::path{xdg};

	const char* home = std::getenv("HOME");

	if (home && *home != '\0') return std::filesystem::path{home}/".cashe";

	return {};
}
