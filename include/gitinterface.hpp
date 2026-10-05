#pragma once

#include "projectcontent.hpp"

#include <expected>
#include <string>


enum class HostOption {
	GitHub,
	GitLab,
	Unknown
};

namespace GitInterface {
	[[nodiscard]] std::expected<std::string, std::string> add(const ProjectContent&);
	[[nodiscard]] bool is_repo(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> changes(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> branch(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> branch_list(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> branch_switch(const ProjectContent&, std::string_view branch);
	[[nodiscard]] std::expected<std::string, std::string> remote(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> remote_short(const ProjectContent&);
	[[nodiscard]] std::expected<HostOption, std::string> host(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> commit(const ProjectContent&, std::string_view);
	[[nodiscard]] std::expected<std::string, std::string> push(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> push_branch(const ProjectContent&, std::string_view);
	[[nodiscard]] std::expected<std::string, std::string> pull(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> fetch(const ProjectContent&);
	[[nodiscard]] std::expected<std::string, std::string> default_merge(const ProjectContent&);
};
