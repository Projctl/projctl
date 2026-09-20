#include "systeminterface.hpp"

#include <cstdlib>

std::optional<std::string> SystemInterface::run_command(const std::string& command)
{
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) return std::nullopt;

	std::string output;
	char buffer[256];

	while (fgets(buffer, sizeof(buffer), pipe) != nullptr) output += buffer;

	int status = pclose(pipe);
	if (status != 0) return std::nullopt;
	if (!output.empty() && output.back() == '\n') output.pop_back();
	return output;
}

int SystemInterface::run_interactive(const std::string& command)
{
	return std::system(command.c_str());
}
