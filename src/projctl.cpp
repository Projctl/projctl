#include "projctl.hpp"

#include <format>
#include <fstream>
#include <print>
#include <string_view>
#include <future>
#include <vector>
#include <functional>
#include <iterator>
#include "config.hpp"

using ProjectIteratorResult = std::optional<std::map<std::string, ProjectContent>::const_iterator>;

namespace {
constexpr std::string_view MARGIN = "    ";

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
std::string truncate(std::string_view text, std::size_t max_length) {
	if (text.size() <= max_length)
		return std::string(text);

	if (max_length <= 5)
		return std::string(text.substr(0, max_length));

	return std::string(text.substr(0, max_length - 5)) + "...";
}

} // namespace

ProjCtl::ProjCtl()
	:git_interface(system_interface),
	gh_interface(system_interface),
	glab_interface(system_interface) {
		config_path = system_interface.config_home()/"projctl/config.ini";
		data_path = system_interface.data_home()/"projctl/projects.ini";
		load();
	}

ProjCtl::~ProjCtl() { save(); }


void ProjCtl::load() {
	config.load(config_path);

	std::ifstream projects(data_path);
	std::string line;
	std::string name;
	std::string path_str;
	std::string build_command;
	std::string run_command;
	std::size_t separator;
	while (std::getline(projects, line)) {
		line = trim(line);
		if (line.empty()) continue;
		if (line.front() == '#' || line.front() == ';') continue;

		switch (line.front()) {
			case '[':
				if (!name.empty()) [[likely]] {
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
	if (!name.empty()) [[likely]] {
		this->projects.emplace(
				name,
				ProjectContent {
				std::filesystem::path { path_str },
				detect_project_type(path_str),
				build_command.empty()?std::nullopt:std::optional<std::string> { build_command },
				run_command.empty()?std::nullopt:std::optional<std::string> { run_command },
				});
	}
}

void ProjCtl::save() {
	std::ofstream projects(data_path);

	for (const decltype(this->projects)::value_type &project : this->projects) {
		projects << '[' << project.first << "]\n";
		projects << "path=" << project.second.path.string() << '\n';

		if (project.second.build_command) projects << "build=" << *project.second.build_command << '\n' ;
		if (project.second.run_command) projects << "run=" << *project.second.run_command << '\n';

		projects << '\n';
	}
}

void ProjCtl::list() {
	auto list_one = [&](const std::string& name, const ProjectContent& content) { return std::format("{}{:<{}}{}{:<{}}{}", color_code(config.project_name_color), truncate(name, config.name_width), config.name_width, color_code(config.project_path_color), truncate(content.path.string(), config.path_width), config.path_width, RESET); };
	if (projects.empty()) [[unlikely]] {
		std::println("{}There are no git projects saved{}", color_code(config.error_color), RESET);
		return;
	}
	std::println("{}{:<{}}{}{}", color_code(config.header_color), "Name", config.name_width, "Path", RESET);
	std::vector<std::future<std::string>> tasks;
	tasks.reserve(projects.size());
	for (const decltype(projects)::value_type& project : projects) tasks.push_back(std::async(std::launch::async, list_one, std::cref(project.first), std::cref(project.second)));
	for (std::future<std::string>& task : tasks) std::println("{}", task.get());
}

void ProjCtl::list_gits() {
	auto list_one_git = [&](const std::string& name, const ProjectContent& content){
		return std::format("{}{:<{}}{}{:<{}}{}{:<{}}{}{:<{}}", color_code(config.project_name_color), truncate(name, config.name_width), config.name_width, color_code(config.project_path_color), truncate(content.path.string(), config.path_width), config.path_width, color_code(config.project_branch_color), truncate(*git_interface.branch(content), config.branch_width), config.branch_width, color_code(config.project_short_remote_color), truncate(*git_interface.remote_short(content), config.remote_width), config.remote_width);
	};

	std::vector<std::future<std::string>> tasks;
	tasks.reserve(projects.size());
	for (const decltype(projects)::value_type& project : projects) if (git_interface.is_repo(project.second)) tasks.push_back(std::async(std::launch::async, list_one_git, std::cref(project.first), std::cref(project.second)));

	if (tasks.empty()) [[unlikely]] {
		std::println("{}There are no git projects saved{}", color_code(config.error_color), RESET);
		return;
	}
	std::println("{}{:<{}}{:<{}}{:<{}}{:<{}}{}", color_code(config.header_color), "Name", config.name_width, "Path", config.path_width, "Branch", config.branch_width, "Repo", config.remote_width, RESET);
	for (std::future<std::string>& task : tasks) std::println("{}", task.get());
}

ProjectIteratorResult ProjCtl::project_find(const std::string &name) {
	std::map<std::string, ProjectContent>::const_iterator project = projects.find(name);
	if (project == projects.end()) {
		std::println("{}{}Project with name {} doesn't exist!{}", color_code(config.error_color), MARGIN, name, RESET);
		return std::nullopt;
	}
	return project;
}

void ProjCtl::status(const std::string &name) {
	auto status_one = [&](const std::string& name, const ProjectContent& content){
		std::string output;
		output.reserve(250);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Name:", config.name_width, color_code(config.project_name_color), truncate(name, config.max_width - 24), RESET);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Path:", config.name_width, color_code(config.project_path_color), truncate(content.path.string(), config.max_width - 24), RESET);
		bool exists = std::filesystem::exists(content.path);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}{}\n", MARGIN, color_code(config.header_color), "Exists:", config.name_width, exists ? color_code(config.tick_color) : color_code(config.cross_color), exists ? TICK : CROSS, exists ? "Yes" : "No", RESET);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Type:", config.name_width, color_code(config.project_type_color), project_type_to_string(content.type), RESET);
		std::expected<std::string, std::string> git = git_interface.remote_short(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}{}\n", MARGIN, color_code(config.header_color), "Git:", config.name_width, git ? color_code(config.tick_color) : color_code(config.cross_color), git ? TICK : CROSS , truncate(git ? *git: git.error(), config.max_width - 24), RESET);
		std::expected<std::string, std::string> branch = git_interface.branch(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Branch:", config.name_width, branch ? color_code(config.project_branch_color) : color_code(config.cross_color), truncate(branch ? *branch : branch.error(), config.max_width - 24), RESET);
		std::expected<std::string, std::string> remote = git_interface.remote(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Full remote:", config.name_width, remote ? color_code(config.project_remote_color) : color_code(config.cross_color), truncate(remote ? *remote: remote.error(), config.max_width - 24), RESET);
		std::expected<std::string, std::string> changes = git_interface.changes(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Status:", config.name_width, changes ? changes -> empty() ? color_code(config.tick_color) : color_code(config.warning_color) : color_code(config.error_color) , changes ? changes->empty() ? "No changes" : "Changes:\n\n" + *changes : changes.error(), RESET);
		return output;
	};

	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) tasks.push_back(std::async(std::launch::async, status_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) println("{}", task.get());
		return;
	}

	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::println("{}", status_one(name, content));
}

void ProjCtl::add(const std::string name, std::filesystem::path path) {
	if (projects.contains(name)) {
		std::println("{}Project with name {}{}{} already exists!{}", color_code(config.error_color), color_code(config.project_name_color), truncate(name, config.name_width), color_code(config.error_color), RESET);
		return;
	}
	if (path.string() == ".") path = std::filesystem::current_path();
	std::pair<std::string, ProjectContent>  new_project = { name, ProjectContent { path, detect_project_type(path) } };
	projects.emplace(new_project);
	std::println("{}{:<{}}{}{}{}", color_code(config.header_color), "Added project:", config.name_width, color_code(config.project_name_color), truncate(new_project.first, config.max_width - 20), RESET);
	std::println("{}{:<{}}{}{}{}", color_code(config.header_color), "With path:", config.name_width, color_code(config.project_path_color), truncate(new_project.second.path.string(), config.max_width - 20),RESET);
}
void ProjCtl::add_current() {
	std::filesystem::path path = std::filesystem::current_path();
	std::string name = std::filesystem::current_path().filename().string();
	if (projects.contains(name)) {
		std::println("{}Project with name {}{}{} already exists!{}", color_code(config.error_color), color_code(config.project_name_color), truncate(name, config.name_width), color_code(config.error_color), RESET);
		return;
	}
	std::pair<std::string, ProjectContent> new_project = { name, { path, detect_project_type(path) } };
	projects.emplace(new_project);
	std::println("{}{:<{}}{}{}{}", color_code(config.header_color), "Added project:", config.name_width, color_code(config.project_name_color), truncate(new_project.first, config.max_width - 20), RESET);
	std::println("{}{:<{}}{}{}{}", color_code(config.header_color), "With path:", config.name_width, color_code(config.project_path_color), truncate(new_project.second.path.string(), config.max_width - 20),RESET);
}

void ProjCtl::remove(const std::string& name) {
	if (projects.erase(name) == 0) {
		std::println("{}Project with name {}{}{} doesn't exist!{}", color_code(config.error_color), color_code(config.project_name_color), truncate(name, config.name_width), color_code(config.error_color), RESET);
		return;
	}
	std::println("{}{:<{}}{}{}{}", color_code(config.header_color), "Removed project:", config.name_width, color_code(config.project_name_color), name, RESET);
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
	auto build_one = [&](const std::string& name, const ProjectContent& content){
		std::string output;
		std::format_to(std::back_inserter(output), "{}Building {}{}{}\n", color_code(config.header_color), color_code(config.project_name_color), name, RESET);

		const std::optional<std::string_view> command_option = content.build_command ? content.build_command : build_command_for(content.type);
		if (!command_option) {
			std::format_to(std::back_inserter(output),"{}{}There's no default command for {}{}{} as type of project {}{}{}", color_code(config.error_color), MARGIN, color_code(config.project_type_color), project_type_to_string(content.type), color_code(config.error_color), color_code(config.project_name_color), name, RESET);
			return output;
		}

		std::string command = std::format("cd \"{}\" && {}", content.path.string(), *command_option);

		std::format_to(std::back_inserter(output), "{}\n", *system_interface.run(command));
		return output;
	};
	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) tasks.push_back(std::async(std::launch::async, build_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) println("{}", task.get());
		return;
	}

	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::println("{}", build_one(name, content));
}

void ProjCtl::run(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	const std::optional<std::string_view> command_option = content.run_command ? content.run_command : run_command_for(content.type);
	if (!command_option) {
		std::println("{}{}There's no default command for {}{}{} as type of project {}{}{}", color_code(config.header_color), MARGIN, color_code(config.project_type_color), project_type_to_string(content.type), color_code(config.header_color), color_code(config.project_name_color), name, RESET);
		return;
	}

	std::string command = std::format("cd \"{}\" && ", content.path.string(), *command_option);

	std::println("{}", *system_interface.run(command));
}

void ProjCtl::git_commit(const std::string& name, std::string_view message) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.commit(content, message);

	std::println("{}{}\n{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? TICK : CROSS, result ? *result : result.error(), RESET);
}

void ProjCtl::git_push(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.push(content);

	std::println("{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? *result : result.error(), RESET);
}

void ProjCtl::git_push_branch(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.branch(content);
	if (!result) {
		std::println("{}{}{}", color_code(config.error_color), result.error(), RESET);
		return;
	}

	std::string branch = *result;

	result = git_interface.push_branch(content, branch);

	std::println("{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? *result : result.error(), RESET);
}

void ProjCtl::git_pull(const std::string& name) {
	auto pull_one = [&](const std::string& project_name, const ProjectContent& content) {
		std::string output;
		std::format_to(std::back_inserter(output), "{}Pulling: {}{}{}\n", color_code(config.header_color), color_code(config.project_name_color), project_name, RESET);
		std::expected<std::string, std::string> result = git_interface.pull(content);

		std::format_to(std::back_inserter(output), "{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? *result : result.error(), RESET);
		return output;
	};

	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project: projects) if (git_interface.is_repo(project.second)) tasks.push_back(std::async(std::launch::async, pull_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) std::println("{}\n", task.get());
		return;
	}
	ProjectIteratorResult project = project_find(name);
	if (!project) return;

	std::println("{}", pull_one((*project)->first, (*project)->second));
}

void ProjCtl::git_fetch(const std::string& name) {
	auto fetch_one = [&](const std::string& project_name, const ProjectContent& content) {
		std::string output;
		std::format_to(std::back_inserter(output), "{}Fetching: {}{}{}\n", color_code(config.header_color), color_code(config.project_name_color), project_name, RESET);
		std::expected<std::string, std::string> result = git_interface.fetch(content);

		std::format_to(std::back_inserter(output), "{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? *result : result.error(), RESET);
		return output;
	};

	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) if (git_interface.is_repo(project.second)) tasks.push_back(std::async(std::launch::async, fetch_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) std::println("{}\n", task.get());
		return;
	}
	ProjectIteratorResult project = project_find(name);
	if (!project) return;

	std::println("{}", fetch_one((*project)->first, (*project)->second));
}

void ProjCtl::git_branch_list(const std::string& name) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.branch_list(content);

	std::println("{}{}{}", result ? color_code(config.project_branch_color) : color_code(config.error_color), result ? *result : result.error(), RESET);
}

void ProjCtl::git_branch(const std::string& name, std::string_view branch) {
	ProjectIteratorResult project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = git_interface.branch_switch(content, branch);

	std::println("{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? *result : result.error(), RESET);
}

void ProjCtl::g_status () {
	auto status_one = [&](const std::string_view name, auto* interface){
		std::string output;
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}{}\n", color_code(config.header_color), MARGIN, std::format("{} Cli", name), config.name_width, interface->installed() ? color_code(config.tick_color) : color_code(config.error_color), interface->installed() ? TICK : CROSS, interface->installed() ? "Installed" : "Not installed", RESET);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}{}\n", color_code(config.header_color), MARGIN, "Auth", config.name_width, interface->authenticated() ? color_code(config.tick_color) : color_code(config.error_color), interface->authenticated() ? TICK : CROSS, interface->authenticated() ? "Authenticated" : "Not authenticated", RESET);
		return output;
	};
	std::future<std::string> gh_task = std::async(std::launch::async, status_one, "GitHub", &gh_interface);
	std::future<std::string> glab_task = std::async(std::launch::async, status_one, "GitLab", &glab_interface);
	std::println("{}", gh_task.get());
	std::println("{}", glab_task.get());
}
// colors from here
void ProjCtl::repo_list() {
	auto list_one = [&](const std::string_view name, auto* interface){
		std::string output;
		if (!interface->installed()) return std::format("{}{} is not installed{}", color_code(config.error_color), name, RESET);
		if (!interface->authenticated()) return std::format("{}{} is not authenticated{}", color_code(config.error_color), name, RESET);
		std::expected<std::string, std::string> repo_list = interface->repo_list();
		std::format_to(std::back_inserter(output), "{}{} {}repos\n{}{}{}", color_code(config.project_name_color), name, color_code(config.header_color), repo_list ? color_code(config.project_short_remote_color) : color_code(config.error_color), repo_list ? *repo_list : repo_list.error(), RESET);
		return output;
	};
	std::future<std::string> gh_task = std::async(std::launch::async, list_one, "GitHub", &gh_interface);
	std::future<std::string> glab_task = std::async(std::launch::async, list_one, "GitLab", &glab_interface);
	std::println("{}\n", gh_task.get());
	std::println("{}", glab_task.get());
}



void ProjCtl::issue_list(const std::string& name) {
	auto format_issues = [&](const std::vector<Issue>& issues) -> std::string {
		std::string output;
		auto field_name = [&](IssueField field) -> std::string_view {
			switch (field) {
				case IssueField::Number:  return "Number";
				case IssueField::Title:   return "Title";
				case IssueField::State:   return "State";
				case IssueField::Author:  return "Author";
				case IssueField::Labels:  return "Labels";
				case IssueField::Created: return "Created";
				case IssueField::Updated: return "Updated";
				case IssueField::Parent:  return "Parent";
				default:				  return "";
			}
		};
		auto field_width = [&](IssueField field) -> std::size_t {
			switch (field) {
				case IssueField::Number:  return 8;
				case IssueField::Title:   return config.issue_title_width;
				case IssueField::State:   return config.issue_state_width;
				case IssueField::Author:  return config.issue_author_width;
				case IssueField::Labels:  return 20;
				case IssueField::Created: return 22;
				case IssueField::Updated: return 22;
				case IssueField::Parent:  return 10;
				default:				  return 0;
			}
		};
		auto field_color = [&](IssueField field, const Issue& issue) -> Color {
			switch (field) {
				case IssueField::Number:
					return config.issue_number_color;
				case IssueField::Title:
					return config.issue_title_color;
				case IssueField::State:
					return issue.state == "OPEN" || issue.state == "opened" ? config.issue_open_color : config.issue_closed_color;
				case IssueField::Author:
					return config.issue_author_color;
				case IssueField::Labels:
					return config.issue_labels_color;
				case IssueField::Parent:
					return config.issue_parent_color;
				default:
					return Color::Default;
			}
		};
		output += color_code(config.header_color);
		for (IssueField field : config.issue_columns) std::format_to(std::back_inserter(output), "{:<{}}", field_name(field), field_width(field));
		output += RESET;
		output += '\n';
		for (const Issue& issue : issues) {
			for (IssueField field : config.issue_columns) {
				std::string value;

				switch (field) {
					case IssueField::Number:
						value = std::format("#{}", issue.number);
						break;
					case IssueField::Title:
						value = truncate(issue.title, config.issue_title_width - 1);
						break;
					case IssueField::State:
						value = issue.state;
						break;
					case IssueField::Author:
						value = issue.author;
						break;
					case IssueField::Labels:
						for (std::size_t i = 0; i < issue.labels.size(); ++i) {
							if (i != 0) value += ", ";
							value += issue.labels[i];
						}
						break;
					case IssueField::Created:
						value = issue.created;
						break;
					case IssueField::Updated:
						value = issue.updated;
						break;
					case IssueField::Parent:
						value = issue.parent ? std::format("#{}", *issue.parent) : "-";
						break;
				}
				std::format_to(std::back_inserter(output), "{}{:<{}}{}", color_code(field_color(field, issue)), value, field_width(field), RESET);
			}
			output += '\n';
		}
		return output;
	};
	auto issue_one = [&](const std::string& name, const ProjectContent& content) -> std::string {
		std::expected<HostOption, std::string> host_option = git_interface.host(content);
		if (!host_option) return std::format("{}Failed to get host {}{}", color_code(config.error_color), host_option.error(), RESET);
		HostOption host = *host_option;

		std::expected<std::string, std::string> remote_short_option = git_interface.remote_short(content);
		if (!remote_short_option) return std::format("{}Failed to get short remote for {}{}", color_code(config.error_color), name, RESET);
		std::string_view remote_short = *remote_short_option;

		std::string output = std::format("{}Issues for {}{}{}:\n", color_code(config.header_color), color_code(config.project_name_color), name, RESET);
		std::expected<std::vector<Issue>, std::string> command_output;
		switch (host) {
			case HostOption::GitHub:
				command_output = gh_interface.issue_list(remote_short);
				break;
			case HostOption::GitLab:
				command_output = glab_interface.issue_list(remote_short);
				break;
			case HostOption::Unknown:
				command_output = std::unexpected(std::format("No host found for {}", name));
				break;
		}
		if (!command_output) {
			std::format_to(std::back_inserter(output), "Failed to list issues\n{}", command_output.error());
			return output;
		}
		if (command_output->empty()) {
			std::format_to(std::back_inserter(output), "{}No issues found{}\n", color_code(config.error_color), RESET);
			return output;
		}
		std::vector<Issue> issues = *command_output;

		std::format_to(std::back_inserter(output), "{}\n", format_issues(issues));

		return output;
	};
	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) if (git_interface.is_repo(project.second)) tasks.push_back(std::async(std::launch::async, issue_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) println("{}", task.get());
		return;
	}

	ProjectIteratorResult project_option = project_find(name);
	if (!project_option) return;
	const ProjectContent& content = (*project_option)->second;
	std::println("{}", issue_one(name, content));
}
