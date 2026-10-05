#include "projctl.hpp"

#include <format>
#include <fstream>
#include <print>
#include "config.hpp"
#include "systeminterface.hpp"

ProjCtl::ProjCtl() {
		config_path = SystemInterface::config_home()/"projctl/config.ini";
		data_path = SystemInterface::data_home()/"projctl/projects.ini";
		load();
		config.load(config_path);
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

std::optional<std::string> ProjCtl::current_project_name() const {
	std::filesystem::path cwd = std::filesystem::weakly_canonical(std::filesystem::current_path());
	for (const decltype(projects)::value_type& project : projects) {
		std::filesystem::path project_path = std::filesystem::weakly_canonical(project.second.path);

		if (cwd == project_path) return project.first;
	}
	return std::nullopt;
}

ProjectIteratorResult ProjCtl::project_find(const std::string &name) {
	std::optional<std::string> actual_name;
	if (name == ".") {
		actual_name = current_project_name();
		if (!actual_name) {
			std::println("{}{}Current directory isn't a saved project!{}", color_code(config.error_color), MARGIN, RESET);
			return std::nullopt;
		}
	}
	else actual_name = name;
	std::map<std::string, ProjectContent>::const_iterator project = projects.find(*actual_name);
	if (project == projects.end()) {
		std::println("{}{}Project with name {} doesn't exist!{}", color_code(config.error_color), MARGIN, name, RESET);
		return std::nullopt;
	}
	return project;
}
