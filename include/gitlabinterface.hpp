#pragma once

#include "issue.hpp"
#include "request.hpp"

#include <expected>
#include <string>


class GitLabInterface {
	public:
	bool installed() const;
	bool authenticated() const;
	std::expected<std::string, std::string> repo_list() const;
	std::expected<std::vector<Issue>, std::string> issue_list(std::string_view) const;
	std::expected<void, std::string> issue_create(std::string_view) const;
	std::expected<std::vector<Request>, std::string> request_list(std::string_view) const;
	std::expected<void, std::string> request_create(std::string_view) const;
	std::expected<std::string, std::string> request_merge(std::string_view) const;
};
