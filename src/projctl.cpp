#include "projctl.hpp"

#include <format>
#include <fstream>
#include <print>
#include <string_view>
#include <iostream>

using ProjectIteratorResult = std::optional<std::map<std::string, ProjectContent>::const_iterator>;

namespace {
constexpr std::string_view MARGIN = "    ";
constexpr int NAME_WIDTH = 20;

ProjectType detect_project_type(const std::filesystem::path& path) {
	if (std::filesystem::exists(path / "Cargo.toml")) return ProjectType::Rust;
	if (std::filesystem::exists(path / "CMakeLists.txt")) return ProjectType::CMake;
	if (std::filesystem::exists(path / "package.json")) return ProjectType::Node;
	if (std::filesystem::exists(path / "pyproject.toml")) return ProjectType::Python;
	return ProjectType::Unknown;
}

} // namespace

ProjCtl::ProjCtl() {
	config_dir = system_interface.get_config_home()/"projctl";
	load_projects();
}

ProjCtl::~ProjCtl() { save_projects(); }


void ProjCtl::load_projects() {
	std::ifstream projects(config_dir/"projects.txt", std::ios::in);

	std::string path_str;
	std::string name;
	while (std::getline(projects, name, '\t')) {
		std::getline(projects, path_str);
		this->projects.emplace(name, ProjectContent { std::filesystem::path(path_str), detect_project_type(std::filesystem::path(path_str)) } );
	}
}

void ProjCtl::save_projects() {
	std::ofstream projects(config_dir/"projects.txt");

	for (const decltype(this->projects)::value_type &project : this->projects)
		projects << project.first << '\t' << project.second.path.string() << '\n';
}

void ProjCtl::list_projects() {
	if (projects.empty())
		std::println("There are no projects saved");
	else
		std::println("{}{}{:<{}}{}\n", MARGIN, MARGIN, "Name", NAME_WIDTH, "Path");

	for (const decltype(this->projects)::value_type &project : projects)
		std::println("{}{}{:<{}}{}", MARGIN, MARGIN, project.first, NAME_WIDTH, project.second.path.string());
}

ProjectIteratorResult ProjCtl::project_find(const std::string &name) {
	std::map<std::string, ProjectContent>::const_iterator project = projects.find(name);
	if (project == projects.end()) {
		std::println("{}Project with name {} doesn't exist!", MARGIN, name);
		return std::nullopt;
	}
	return project;
}

void ProjCtl::project_status(const std::string &name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::println("{}{:<{}}{}", MARGIN, "Name:", NAME_WIDTH, (*project)->first);
	std::println("{}{:<{}}{}", MARGIN, "Path:", NAME_WIDTH, content.path.string());
	std::println("{}{:<{}}{}", MARGIN, "Exists:", NAME_WIDTH, std::filesystem::exists(content.path) ? "Yes" : "No");
	std::println("{}{:<{}}{}", MARGIN, "Type:", NAME_WIDTH, project_type_to_string(content.type));
	bool is_git = std::filesystem::exists(content.path / ".git");
	std::println("{}{:<{}}{}", MARGIN, "Git:", NAME_WIDTH, is_git ? "Yes" : "No");
	if (!is_git) return;
	std::optional<std::string> branch_output = system_interface.run_command(std::format("git -C \"{}\" branch --show-current", content.path.string()));
	std::println("{}{:<{}}{}", MARGIN, "Branch:", NAME_WIDTH, branch_output ? *branch_output : "----");
	std::optional<std::string> remote_output = system_interface.run_command(std::format("git -C \"{}\" remote get-url origin", content.path.string()));
	std::println("{}{:<{}}{}", MARGIN, "Remote:", NAME_WIDTH, remote_output ? *remote_output : "----");
}

void ProjCtl::project_add(const std::string name, std::filesystem::path path) {
	if (projects.contains(name)) {
		std::print("{}Project with name {} already exists!", MARGIN, name);
		return;
	}
	if (path.string() == ".") path = std::filesystem::current_path();
	std::pair<std::string, ProjectContent>  new_project = { name, ProjectContent { path, detect_project_type(path) } };
	projects.emplace(new_project);
	std::println("{}Added project:\t{}", MARGIN, new_project.first);
	std::println("{}With path:\t\t{}", MARGIN, new_project.second.path.string());
}
void ProjCtl::project_add_current() {
	std::filesystem::path path = std::filesystem::current_path();
	std::string name = std::filesystem::current_path().filename().string();
	if (projects.contains(name)) {
		std::print("{}Project with name {} already exists!", MARGIN, name);
		return;
	}
	std::pair<std::string, ProjectContent> new_project = { name, { path, detect_project_type(path) } };
	projects.emplace(new_project);
	std::println("{}Added project:\t{}", MARGIN, new_project.first);
	std::println("{}With path:\t\t{}", MARGIN, new_project.second.path.string());
}

void ProjCtl::project_remove(const std::string& name) {
	if (projects.erase(name) == 0) {
		std::print("{}Project with name {} doesn't exist!", MARGIN, name);
		return;
	}
	std::println("{}Removed project:\t{}", MARGIN, name);
}

void ProjCtl::path_show(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;

	std::println("{}{:<{}}{}\n\n{}{}{}", MARGIN, "Path for:", NAME_WIDTH, name, MARGIN, MARGIN, (*project)->second.path.string());
}

void ProjCtl::project_open(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;

	system_interface.run_interactive(std::format("nvim \"{}\"", (*project)->second.path.string()));
}

void ProjCtl::project_build(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::optional<std::string_view> command_option;

	if (content.build_command) {
		command_option = content.build_command;
		goto build;
	}

	command_option = build_command_for(content.type);
build:
	if (!command_option) {
		std::println("{}There's no default command for {} as type of project {}", MARGIN, project_type_to_string(content.type), name);
		return;
	}

	std::string command = std::string("cd ").append(content.path.string()).append(" && ").append(*command_option);

	std::cout << *system_interface.run_command(command);
}

void ProjCtl::project_run(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::optional<std::string> command_option;

	if (content.run_command) {
		command_option = content.run_command;
		goto run;
	}

	command_option = run_command_for(content.type);
run:
	if (!command_option) {
		std::println("{}There's no default command for {} as type of project {}", MARGIN, project_type_to_string(content.type), name);
		return;
	}

	std::string command = std::string("cd ").append(content.path.string()).append(" && ").append(*command_option);

	std::cout << *system_interface.run_command(command);
}
