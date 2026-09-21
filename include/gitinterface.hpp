#pragma once

#include "projectcontent.hpp"
#include "systeminterface.hpp"

#include <optional>
#include <expected>
#include <string>

class GitInterface {
	SystemInterface& system_interface;
	bool is_repo(const ProjectContent&);
	std::optional<std::string> add(const ProjectContent&);
	public:
	GitInterface(SystemInterface&);
	std::expected<std::string, std::string> commit(const ProjectContent&, std::string_view);
};
