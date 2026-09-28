#pragma once

#include "systeminterface.hpp"
#include "issue.hpp"

#include <vector>
#include <expected>
#include <string>

class GitHubInterface {
	SystemInterface& system_interface;

	public:
	GitHubInterface(SystemInterface& system_interface);
	bool installed() const;
	bool authenticated() const;
	std::expected<std::string, std::string>repo_list();
	std::expected<std::vector<Issue>, std::string>issue_list(std::string_view);
};
