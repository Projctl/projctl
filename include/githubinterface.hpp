#pragma once

#include "issue.hpp"

#include <vector>
#include <expected>
#include <string>

class GitHubInterface {
	public:
	bool installed() const;
	bool authenticated() const;
	std::expected<std::string, std::string>repo_list() const;
	std::expected<std::vector<Issue>, std::string>issue_list(std::string_view) const;
};
