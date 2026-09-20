#pragma once

#include <optional>
#include <string>

class SystemInterface {
	public:
		std::optional<std::string> run_command(const std::string&);
		int run_interactive(const std::string&);
};
