#include "projctl.hpp"

#include <format>
#include <fstream>
#include <print>
#include <string_view>

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

std::string trim(std::string_view text) {
	std::size_t first = text.find_first_not_of(" \t\r\n");

	if (first == std::string_view::npos) return "";

	std::size_t last = text.find_last_not_of(" \t\r\n");

	return std::string(text.substr(first, last - first + 1));
}

} // namespace

ProjCtl::ProjCtl() : git_interface(system_interface) {
	config_dir = system_interface.config_home()/"projctl";
	data_dir = system_interface.cache_home()/"projctl";
	load();
}

ProjCtl::~ProjCtl() { save(); }


void ProjCtl::load() {
	std::ifstream projects(data_dir/"projects.ini", std::ios::in);

	std::string line;
	std::string name;
	std::string path_str;
	std::string build_command;
	std::string run_command;
	std::size_t separator;
	while (std::getline(projects, line)) {
		line = trim(line);
		if (line.empty()) continue;
		switch (line.front()) {
			case '[':
				if (!name.empty()) {
					this->projects.emplace(
							name,
							ProjectContent {
							std::filesystem::path { path_str },
							detect_project_type(path_str),
							build_command.empty() ? std::nullopt : std::optional<std::string> { build_command },
							run_command.empty() ? std::nullopt : std::optional<std::string> { run_command },
							}
							);
					path_str.clear();
					build_command.clear();
					run_command.clear();
				}
				name = trim(line.substr(1, line.size() - 2));
				continue;
			case 'p':
				separator = line.find('=');
				path_str = trim(line.substr(separator + 1));
				continue;
			case 'r':
				separator = line.find('=');
				run_command = trim(line.substr(separator + 1));
				continue;
			case 'b':
				separator = line.find('=');
				build_command = trim(line.substr(separator + 1));
				continue;
		}
	}
	if (!name.empty()) {
		this->projects.emplace(
				name,
				ProjectContent {
				std::filesystem::path { path_str },
				detect_project_type(path_str),
				build_command.empty()?std::nullopt:std::optional<std::string> { build_command },
				run_command.empty()?std::nullopt:std::optional<std::string> { run_command },
				}
				);
	}
}

void ProjCtl::save() {
	std::ofstream projects(data_dir/"projects.ini");

	for (const decltype(this->projects)::value_type &project : this->projects) {
		projects << '[' << project.first << "]\n";
		projects << "path=" << project.second.path.string() << '\n';

		if (project.second.build_command) projects << "build=" << *project.second.build_command << '\n' ;

		if (project.second.run_command) projects << "run=" << *project.second.run_command << '\n';

		projects << '\n';
	}
}

void ProjCtl::list() {
	auto list_one = [&](const std::string& name, const ProjectContent& content) { std::println("{}{}{:<{}}{}", MARGIN, MARGIN, name, NAME_WIDTH, content.path.string()); };
	if (projects.empty()) std::println("There are no projects saved");
	else {
		std::println("{}{}{:<{}}{}\n", MARGIN, MARGIN, "Name", NAME_WIDTH, "Path");
		for (const decltype(projects)::value_type& project : projects) list_one(project.first, project.second);
	}

}

void ProjCtl::list_gits() {
	auto list_one_git = [this](const std::string& name, const ProjectContent& content){
		if (git_interface.is_repo(content)) std::println("{}{}{:<{}}{:<50}{:<{}}{:<{}}", MARGIN, MARGIN, name, NAME_WIDTH, content.path.string(), *git_interface.branch(content), NAME_WIDTH, *git_interface.remote_short(content), NAME_WIDTH);
	};
	std::println("{}{}{:<{}}{:<50}{:<{}}{:<{}}\n", MARGIN, MARGIN, "Name", NAME_WIDTH, "Path", "Branch", NAME_WIDTH, "Repo", NAME_WIDTH);
	for (const decltype(projects)::value_type& project : projects) list_one_git(project.first, project.second);
}

ProjectIteratorResult ProjCtl::project_find(const std::string &name) {
	std::map<std::string, ProjectContent>::const_iterator project = projects.find(name);
	if (project == projects.end()) {
		std::println("{}Project with name {} doesn't exist!", MARGIN, name);
		return std::nullopt;
	}
	return project;
}

void ProjCtl::status(const std::string &name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::println("{}{:<{}}{}", MARGIN, "Name:", NAME_WIDTH, (*project)->first);
	std::println("{}{:<{}}{}", MARGIN, "Path:", NAME_WIDTH, content.path.string());
	std::println("{}{:<{}}{}", MARGIN, "Exists:", NAME_WIDTH, std::filesystem::exists(content.path) ? "Yes" : "No");
	std::println("{}{:<{}}{}", MARGIN, "Type:", NAME_WIDTH, project_type_to_string(content.type));
	std::expected<std::string, std::string> git = git_interface.remote_short(content);
	std::println("{}{:<{}}{}", MARGIN, "Git:", NAME_WIDTH, git ? *git: git.error());
	std::expected<std::string, std::string> branch = git_interface.branch(content);
	std::println("{}{:<{}}{}", MARGIN, "Branch:", NAME_WIDTH, branch ? *branch : branch.error());
	std::expected<std::string, std::string> remote = git_interface.remote(content);
	std::println("{}{:<{}}{}", MARGIN, "Full remote:", NAME_WIDTH, remote ? *remote: remote.error());
}

void ProjCtl::add(const std::string name, std::filesystem::path path) {
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
void ProjCtl::add_current() {
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

void ProjCtl::remove(const std::string& name) {
	if (projects.erase(name) == 0) {
		std::print("{}Project with name {} doesn't exist!", MARGIN, name);
		return;
	}
	std::println("{}Removed project:\t{}", MARGIN, name);
}

void ProjCtl::path_show(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;

	std::println("{}", (*project)->second.path.string());
}

void ProjCtl::open(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;

	system_interface.run_interactive(std::format("cd \"{}\" && nvim .", (*project)->second.path.string()));
}

void ProjCtl::build(const std::string& name) {
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

	std::println("{}", *system_interface.run(command));
}

void ProjCtl::run(const std::string& name) {
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

	std::println("{}", *system_interface.run(command));
}

void ProjCtl::git_commit(const std::string& name, std::string_view message) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.commit(content, message);

	if (!result) {
		std::println("{}", result.error());
		return;
	}

	std::println("{}", *result);
}

void ProjCtl::git_push(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.push(content);

	if (!result) {
		std::println("{}", result.error());
		return;
	}

	std::println("{}", *result);
}

void ProjCtl::git_push_branch(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.branch(content);
	if (!result) {
		std::println("{}", result.error());
	}

	std::string branch = *result;

	result = git_interface.push_branch(content, branch);

	if (!result) {
		std::println("{}", result.error());
		return;
	}

	std::println("{}", *result);
}

void ProjCtl::git_pull(const std::string& name) {
	auto pull_one = [&](const std::string& project_name, const ProjectContent& content) {
		std::println("Pulling: {}", project_name);
		std::expected<std::string, std::string> result = git_interface.pull(content);

		if (!result) {
			std::println("{}\n", result.error());
			return;
		}
		std::println("{}\n", *result);
	};

	if (name == "--all") {
		for (const decltype(projects)::value_type& project: projects) if (git_interface.is_repo(project.second)) pull_one(project.first, project.second);
		return;
	}
	ProjectIteratorResult project = project_find(name);
	if (!project) return;

	pull_one((*project)->first, (*project)->second);
}

void ProjCtl::git_branch_list(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.branch_list(content);
	if (!result) {
		std::println("{}", result.error());
		return;
	}

	std::println("{}", *result);
}

void ProjCtl::git_branch(const std::string& name, std::string_view branch) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.branch_switch(content, branch);
	if (!result) {
		std::println("{}", result.error());
		return;
	}

	std::println("{}", *result);
}
