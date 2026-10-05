#include "projctl.hpp"

#include <format>
#include <print>
#include <string_view>
#include <future>
#include <vector>
#include <functional>
#include <iterator>
#include "config.hpp"
#include "gitinterface.hpp"

void ProjCtl::list_gits() const {
	auto list_one_git = [&](const std::string& name, const ProjectContent& content){
		return std::format("{}{:<{}}{}{:<{}}{}{:<{}}{}{:<{}}", color_code(config.project_name_color), truncate(name, config.name_width), config.name_width, color_code(config.project_path_color), truncate(content.path.string(), config.path_width), config.path_width, color_code(config.project_branch_color), truncate(*GitInterface::branch(content), config.branch_width), config.branch_width, color_code(config.project_short_remote_color), truncate(*GitInterface::remote_short(content), config.remote_width), config.remote_width);
	};

	std::vector<std::future<std::string>> tasks;
	tasks.reserve(projects.size());
	for (const decltype(projects)::value_type& project : projects) if (GitInterface::is_repo(project.second)) tasks.push_back(std::async(std::launch::async, list_one_git, std::cref(project.first), std::cref(project.second)));

	if (tasks.empty()) [[unlikely]] {
		std::println("{}There are no git projects saved{}", color_code(config.error_color), Style::RESET);
		return;
	}
	std::println("{}{:<{}}{:<{}}{:<{}}{:<{}}{}", color_code(config.header_color), "Name", config.name_width, "Path", config.path_width, "Branch", config.branch_width, "Repo", config.remote_width, Style::RESET);
	for (std::future<std::string>& task : tasks) std::println("{}", task.get());
}

void ProjCtl::status(const std::string &name) const {
	auto status_one = [&](const std::string& name, const ProjectContent& content){
		std::string output;
		output.reserve(250);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Name:", config.name_width, color_code(config.project_name_color), truncate(name, config.max_width - 24), Style::RESET);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Path:", config.name_width, color_code(config.project_path_color), truncate(content.path.string(), config.max_width - 24), Style::RESET);
		bool exists = std::filesystem::exists(content.path);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}{}\n", MARGIN, color_code(config.header_color), "Exists:", config.name_width, exists ? color_code(config.tick_color) : color_code(config.cross_color), exists ? Style::TICK : Style::CROSS, exists ? "Yes" : "No", Style::RESET);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Type:", config.name_width, color_code(config.project_type_color), project_type_to_string(content.type), Style::RESET);
		std::expected<std::string, std::string> git = GitInterface::remote_short(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}{}\n", MARGIN, color_code(config.header_color), "Git:", config.name_width, git ? color_code(config.tick_color) : color_code(config.cross_color), git ? Style::TICK : Style::CROSS , truncate(git ? *git: git.error(), config.max_width - 24), Style::RESET);
		std::expected<std::string, std::string> branch = GitInterface::branch(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Branch:", config.name_width, branch ? color_code(config.project_branch_color) : color_code(config.cross_color), truncate(branch ? *branch : branch.error(), config.max_width - 24), Style::RESET);
		std::expected<std::string, std::string> remote = GitInterface::remote(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Full remote:", config.name_width, remote ? color_code(config.project_remote_color) : color_code(config.cross_color), truncate(remote ? *remote: remote.error(), config.max_width - 24), Style::RESET);
		std::expected<std::string, std::string> changes = GitInterface::changes(content);
		std::format_to(std::back_inserter(output), "{}{}{:<{}}{}{}{}\n", MARGIN, color_code(config.header_color), "Status:", config.name_width, changes ? changes -> empty() ? color_code(config.tick_color) : color_code(config.warning_color) : color_code(config.error_color) , changes ? changes->empty() ? "No changes" : "Changes:\n\n" + *changes : changes.error(), Style::RESET);
		return output;
	};

	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) tasks.push_back(std::async(std::launch::async, status_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) println("{}", task.get());
		return;
	}

	std::optional project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;
	std::println("{}", status_one((*project)->first, content));
}


void ProjCtl::git_commit(const std::string& name, std::string_view message) const {
	std::optional project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = GitInterface::commit(content, message);

	std::println("{}{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? Style::TICK : Style::CROSS, result ? *result : result.error(), Style::RESET);
}

bool ProjCtl::git_merge(const std::string& name) const {
	std::optional project = project_find(name);
	if (!project) return false;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = GitInterface::default_merge(content);
	if (!result) {
		std::println("{}{}{}", color_code(config.error_color), Style::CROSS, result.error());
		return false;
	}

	std::println("{}{}{}", color_code(config.tick_color), Style::TICK, *result);
	return true;
}

void ProjCtl::git_automerge(const std::string& name, std::string_view message) const {
	if (!git_merge(name)) return;
	git_commit(name, message);
	git_push(name);
}

void ProjCtl::git_push(const std::string& name) const {
	std::optional project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = GitInterface::push(content);

	std::println("{}{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? Style::TICK : Style::CROSS, result ? *result : result.error(), Style::RESET);
}

void ProjCtl::git_push_branch(const std::string& name) const {
	std::optional project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = GitInterface::branch(content);
	if (!result) {
		std::println("{}{}{}{}", color_code(config.error_color), Style::CROSS, result.error(), Style::RESET);
		return;
	}

	std::string branch = *result;

	result = GitInterface::push_branch(content, branch);

	std::println("{}{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? Style::TICK : Style::CROSS, result ? *result : result.error(), Style::RESET);
}

void ProjCtl::git_pull(const std::string& name) const {
	auto pull_one = [&](const std::string& project_name, const ProjectContent& content) {
		std::string output;
		std::format_to(std::back_inserter(output), "{}Pulling: {}{}{}\n", color_code(config.header_color), color_code(config.project_name_color), project_name, Style::RESET);
		std::expected<std::string, std::string> result = GitInterface::pull(content);

		std::format_to(std::back_inserter(output), "{}{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? Style::TICK : Style::CROSS, result ? *result : result.error(), Style::RESET);
		return output;
	};

	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project: projects) if (GitInterface::is_repo(project.second)) tasks.push_back(std::async(std::launch::async, pull_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) std::println("{}\n", task.get());
		return;
	}
	std::optional project = project_find(name);
	if (!project) return;

	std::println("{}", pull_one((*project)->first, (*project)->second));
}

void ProjCtl::git_fetch(const std::string& name) const {
	auto fetch_one = [&](const std::string& project_name, const ProjectContent& content) {
		std::string output;
		std::format_to(std::back_inserter(output), "{}Fetching: {}{}{}\n", color_code(config.header_color), color_code(config.project_name_color), project_name, Style::RESET);
		std::expected<std::string, std::string> result = GitInterface::fetch(content);

		std::format_to(std::back_inserter(output), "{}{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? Style::TICK : Style::CROSS, result ? *result : result.error(), Style::RESET);
		return output;
	};

	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) if (GitInterface::is_repo(project.second)) tasks.push_back(std::async(std::launch::async, fetch_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) std::println("{}\n", task.get());
		return;
	}
	std::optional project = project_find(name);
	if (!project) return;

	std::println("{}", fetch_one((*project)->first, (*project)->second));
}

void ProjCtl::git_branch_list(const std::string& name) const {
	std::optional project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = GitInterface::branch_list(content);

	std::println("{}{}{}", result ? color_code(config.project_branch_color) : color_code(config.error_color), result ? *result : result.error(), Style::RESET);
}

void ProjCtl::git_branch(const std::string& name, std::string_view branch) const {
	std::optional project = project_find(name);
	if (!project) return;
	const ProjectContent& content = (*project)->second;

	std::expected<std::string, std::string> result = GitInterface::branch_switch(content, branch);

	std::println("{}{}{}{}", result ? color_code(config.tick_color) : color_code(config.error_color), result ? Style::TICK : Style::CROSS, result ? *result : result.error(), Style::RESET);
}
