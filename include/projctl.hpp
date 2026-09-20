#pragma once

#include "projectcontent.hpp"
#include "systeminterface.hpp"

#include <filesystem>
#include <unordered_map>

class ProjCtl
{
	std::unordered_map<std::string, ProjectContent> projects;
	std::filesystem::path projects_path;
	SystemInterface system_interface;
	void load_projects();
	void save_projects();
	std::optional<std::unordered_map<std::string, ProjectContent>::const_iterator> project_find(const std::string&);

	public:
	ProjCtl();
	~ProjCtl();
	void project_add(const std::pair<std::string, ProjectContent&>);
	void project_remove(const std::string&);
	void list_projects();
	void path_show(const std::string&);
	void project_status(const std::string&);
	void project_open(const std::string&);
};
