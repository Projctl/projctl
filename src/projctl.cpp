#include "projctl.hpp"

#include <print>
#include <fstream>
#include <string_view>
#include <format>

using ProjectIteratorResult = std::optional<std::unordered_map<std::string, ProjectContent>::const_iterator>;

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
}

void ProjCtl::save_projects()
{
	std::ofstream projects(projects_path);

	for (const decltype(this->projects)::value_type& project: this->projects) projects << project.first << '\t' << project.second.path.string() << '\n';
}

void ProjCtl::list_projects()
{
	if	(projects.empty())	std::println("There are no projects saved");
	else std::println("{}{}{:<{}}{}\n", MARGIN, MARGIN, "Name", NAME_WIDTH, "Path");

	for	(const decltype(this->projects)::value_type& project : projects)	std::println("{}{}{:<{}}{}", MARGIN, MARGIN, project.first, NAME_WIDTH, project.second.path.string());
}

ProjectIteratorResult ProjCtl::project_find(const std::string& name)
{
	std::unordered_map<std::string, ProjectContent>::const_iterator project = projects.find(name);
	if (project == projects.end())
	{
		std::println("{}Project with name {} doesn't exist!", MARGIN, name);
		return std::nullopt;
	}
	return project;
}

void ProjCtl::project_status(const std::string& name)
{
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::println("{}{:<10}{}", MARGIN, "Name:", (*project)->first);
	std::println("{}{:<10}{}", MARGIN, "Path:", content.path.string());
	std::println("{}{:<10}{}", MARGIN, "Exists:", std::filesystem::exists(content.path)?"Yes":"No");
	bool is_git = std::filesystem::exists(content.path/".git");
	std::println("{}{:<10}{}", MARGIN, "Git:", is_git?"Yes":"No");
	if (!is_git) return;
	std::optional<std::string> branch_output = system_interface.run_command(std::format("git -C \"{}\" branch --show-current", content.path.string()));
	std::println("{}{:<10}{}", MARGIN, "Branch:", branch_output ? *branch_output : "----");
	std::optional<std::string> remote_output = system_interface.run_command(std::format("git -C \"{}\" remote get-url origin", content.path.string()));
	std::println("{}{:<10}{}", MARGIN, "Remote:", remote_output ? *remote_output : "----");
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
	ProjectIteratorResult project = projects.find(name);
	if (!project) return;

	std::println("{}{:<20}{}\n\n{}{}{}", MARGIN, "Path for:", name, MARGIN, MARGIN, (*project)->second.path.string());
}

void ProjCtl::project_open(const std::string& name)
{
	ProjectIteratorResult project = projects.find(name);
	if (!project) return;

	int _output = system_interface.run_interactive(std::format("nvim \"{}\"", (*project)->second.path.string()));
}
