#include "projctl.hpp"

#include <format>
#include <print>
#include <string_view>
#include <future>
#include <vector>
#include <functional>
#include <iterator>
#include "config.hpp"

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

	if (config.editor == "neovide" || config.editor == "emacs") system_interface.run_interactive(std::format("cd \"{}\" && {} . &", (*project)->second.path.string(), config.editor));
	else system_interface.run_interactive(std::format("cd \"{}\" && {} .", (*project)->second.path.string(), config.editor));
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
