#include "projctl.hpp"

#include <print>
#include <fstream>
#include <string_view>

namespace
{
	constexpr std::string_view MARGIN = "    ";
	constexpr int NAME_WIDTH = 20;
}

ProjCtl::ProjCtl()
{
	projects_path = "/home/evrandil/.config/projctl/projects.txt";
	load_projects();
}

ProjCtl::~ProjCtl()
{
	save_projects();
}

void ProjCtl::load_projects()
{
	std::ifstream projects(projects_path, std::ios::in);

	std::string path_str;
	std::string name;
	while (std::getline(projects, name, '\t'))
	{
		std::getline(projects, path_str);
		this->projects.emplace(name, ProjectContent { std::filesystem::path(path_str) } );
	}

	projects.close();
}

void ProjCtl::save_projects()
{
	std::ofstream projects(projects_path);

	for (const auto& [name, content]: this->projects) projects << name << '\t' << content.path.string() << std::endl;

	projects.close();
}

void ProjCtl::list_projects()
{
	if	(projects.size() == 0)	std::println("There are no projects saved");
	else std::println("{}{}{:<{}}{}\n", MARGIN, MARGIN, "Name", NAME_WIDTH, "Path");

	for	(const auto& [name, content] : projects)	std::println("{}{}{:<{}}{}", MARGIN, MARGIN, name, NAME_WIDTH, content.path.string());
}

void ProjCtl::project_status(const std::string& name)
{
	std::println("Ayo");
}

void ProjCtl::project_add(const std::pair<std::string, ProjectContent&> new_project)
{
	if (projects.contains(new_project.first))
	{
		std::print("{}Project with name {} already exists!", MARGIN, new_project.first);
		return;
	}
	projects.emplace(new_project);
	std::println("{}Added project:\t{}", MARGIN, new_project.first);
	std::println("{}With path:\t\t{}", MARGIN, new_project.second.path.string());
}

void ProjCtl::project_remove(const std::string& name)
{
	if (projects.erase(name) == 0)
	{
		std::print("{}Project with name {} doesn't exist!", MARGIN, name);
		return;
	}
	std::println("{}Removed project:\t{}", MARGIN, name);
}

void ProjCtl::path_show(const std::string& name)
{
	std::println("{}{:<20}{}\n\n{}{}{}", MARGIN, "Path for:", name, MARGIN, MARGIN, projects[name].path.string());
}
