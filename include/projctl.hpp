#pragma once

#include "projectcontent.hpp"

#include <filesystem>
#include <unordered_map>

class ProjCtl
{
	std::unordered_map<std::string, ProjectContent> projects;
	std::filesystem::path projects_path;
	void load_projects();
	void save_projects();

	public:
	ProjCtl();
	~ProjCtl();
	void project_add(const std::pair<std::string, ProjectContent&>);
	void project_remove(const std::string&);
	void list_projects();
	void path_show(const std::string&);
	void project_status(const std::string&);
};
