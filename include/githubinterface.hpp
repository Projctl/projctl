#pragma once

#include "issue.hpp"
#include "request.hpp"

#include <vector>
#include <expected>
#include <string>

class GitHubInterface {
	public:
	bool installed() const;
	bool authenticated() const;
	std::expected<std::string, std::string>repo_list() const;
	std::expected<std::vector<Issue>, std::string>issue_list(std::string_view) const;
	std::expected<void, std::string>issue_create(std::string_view) const;
	std::expected<std::vector<Request>, std::string> request_list(std::string_view) const;
};
