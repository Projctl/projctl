#pragma once

#include "projectcontent.hpp"
#include "systeminterface.hpp"

#include <filesystem>
#include <map>

class ProjCtl
{
	std::map<std::string, ProjectContent> projects;
	std::filesystem::path config_dir;
	SystemInterface system_interface;
	void load_projects();
	void save_projects();
	std::optional<std::map<std::string, ProjectContent>::const_iterator> project_find(const std::string&);

	public:
	ProjCtl();
	~ProjCtl();
	void project_add(const std::string, std::filesystem::path);
	void project_add_current();
	void project_remove(const std::string&);
	void list_projects();
	void path_show(const std::string&);
	void project_status(const std::string&);
	void project_open(const std::string&);
	void project_build(const std::string&);
};
