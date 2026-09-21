#pragma once

#include "projectcontent.hpp"
#include "systeminterface.hpp"

#include <optional>
#include <string>

class GitInterface {
	SystemInterface& system_interface;
	public:
	GitInterface(SystemInterface&);
	std::optional<std::string> add(const ProjectContent&);
	std::optional<std::string> commit(const ProjectContent&, const std::string_view&);
};
