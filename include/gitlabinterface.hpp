#pragma once

#include "issue.hpp"

#include <expected>
#include <string>


class GitLabInterface {
	public:
	bool installed() const;
	bool authenticated() const;
	std::expected<std::string, std::string> repo_list();
	std::expected<std::vector<Issue>, std::string> issue_list(std::string_view remote_short);
};
