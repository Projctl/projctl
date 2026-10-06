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

void ProjCtl::issue_list(const std::string& name) const {
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
		auto field_color = [&](IssueField field, const Issue& issue) -> Style::Color {
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
					return Style::Color::Default;
			}
		};
		output += color_code(config.header_color);
		for (IssueField field : config.issue_columns) std::format_to(std::back_inserter(output), "{:<{}}", field_name(field), field_width(field));
		output += Style::RESET;
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
				std::format_to(std::back_inserter(output), "{}{:<{}}{}", color_code(field_color(field, issue)), value, field_width(field), Style::RESET);
			}
			output += '\n';
		}
		return output;
	};
	auto issue_one = [&](const std::string& name, const ProjectContent& content) -> std::string {
		std::expected<HostOption, std::string> host_option = GitInterface::host(content);
		if (!host_option) return std::format("{}Failed to get host {}{}", color_code(config.error_color), host_option.error(), Style::RESET);
		HostOption host = *host_option;

		std::expected<std::string, std::string> remote_short_option = GitInterface::remote_short(content);
		if (!remote_short_option) return std::format("{}Failed to get short remote for {}{}", color_code(config.error_color), name, Style::RESET);
		std::string_view remote_short = *remote_short_option;

		std::string output = std::format("{}Issues for {}{}{}:\n", color_code(config.header_color), color_code(config.project_name_color), name, Style::RESET);
		std::expected<std::vector<Issue>, std::string> command_output;
		switch (host) {
			case HostOption::GitHub:
				command_output = gh_interface.issue_list(remote_short);
				break;
			case HostOption::GitLab:
				command_output = glab_interface.issue_list(remote_short);
				break;
			case HostOption::Unknown:
				command_output = std::unexpected(std::format("{}No host found for {}{}", color_code(config.error_color), name, Style::RESET));
				break;
		}
		if (!command_output) {
			std::format_to(std::back_inserter(output), "{}Failed to list issues\n{}{}", color_code(config.error_color), command_output.error(), Style::RESET);
			return output;
		}
		if (command_output->empty()) {
			std::format_to(std::back_inserter(output), "{}No issues found{}\n", color_code(config.error_color), Style::RESET);
			return output;
		}
		std::vector<Issue> issues = *command_output;

		std::format_to(std::back_inserter(output), "{}\n", format_issues(issues));

		return output;
	};
	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) if (GitInterface::is_repo(project.second)) tasks.push_back(std::async(std::launch::async, issue_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) println("{}", task.get());
		return;
	}

	std::optional project_option = project_find(name);
	if (!project_option) return;

	const ProjectContent& content = (*project_option)->second;
	std::println("{}", issue_one((*project_option)->first, content));
}

void ProjCtl::issue_create(const std::string& name) const {
	std::optional project_option = project_find(name);
	if (!project_option) return;

	const ProjectContent& content = (*project_option)->second;
	std::expected<HostOption, std::string> host_option = GitInterface::host(content);
	if (!host_option) {
		std::println("{}Failed to get host {}{}", color_code(config.error_color), host_option.error(), Style::RESET);
		return;
	}
	HostOption host = *host_option;

	std::expected<std::string, std::string> remote_short_option = GitInterface::remote_short(content);
	if (!remote_short_option) {
		std::println("{}Failed to get short remote for {}{}", color_code(config.error_color), name, Style::RESET);
		return;
	}
	std::string_view remote_short = *remote_short_option;

	std::expected<void, std::string> success;
	switch (host) {
		case HostOption::GitHub:
			success = gh_interface.issue_create(remote_short);
			break;
		case HostOption::GitLab:
			success = glab_interface.issue_create(remote_short);
			break;
		case HostOption::Unknown:
			std::println("{}No host found for {}{}", color_code(config.error_color), name, Style::RESET);
			break;
	}
}
