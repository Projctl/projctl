#pragma once

#include "projectcontent.hpp"
#include "systeminterface.hpp"

#include <expected>
#include <string>

class GitInterface {
	SystemInterface& system_interface;
	std::expected<std::string, std::string> add(const ProjectContent&);
	public:
	GitInterface(SystemInterface&);
	bool is_repo(const ProjectContent&);
	std::expected<std::string, std::string> commit(const ProjectContent&, std::string_view);
	std::expected<std::string, std::string> push(const ProjectContent&);
	std::expected<std::string, std::string> pull(const ProjectContent&);
};
