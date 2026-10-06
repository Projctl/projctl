#include "projctl.hpp"

#include <future>
#include <print>
#include "gitinterface.hpp"

void ProjCtl::request_list(const std::string& name) const {
	auto format_requests = [&](const std::vector<Request>& requests) -> std::string {
		std::string output;
		auto field_name = [&](RequestField field) -> std::string_view {
			switch (field) {
				case RequestField::Number:  return "Number";
				case RequestField::Title:   return "Title";
				case RequestField::State:   return "State";
				case RequestField::Author:  return "Author";
				case RequestField::Labels:  return "Labels";
				case RequestField::Created: return "Created";
				case RequestField::Updated: return "Updated";
				case RequestField::Parent:  return "Parent";
				default:				  return "";
			}
		};
		auto field_width = [&](RequestField field) -> std::size_t {
			switch (field) {
				case RequestField::Number:  return 8;
				case RequestField::Title:   return config.request_title_width;
				case RequestField::State:   return config.request_state_width;
				case RequestField::Author:  return config.request_author_width;
				case RequestField::Labels:  return 20;
				case RequestField::Created: return 22;
				case RequestField::Updated: return 22;
				case RequestField::Parent:  return 10;
				default:				  return 0;
			}
		};
		auto field_color = [&](RequestField field, const Request& request) -> Style::Color {
			switch (field) {
				case RequestField::Number:
					return config.request_number_color;
				case RequestField::Title:
					return config.request_title_color;
				case RequestField::State:
					return request.state == "OPEN" || request.state == "opened" ? config.request_open_color : config.request_closed_color;
				case RequestField::Author:
					return config.request_author_color;
				case RequestField::Labels:
					return config.request_labels_color;
				case RequestField::Parent:
					return config.request_parent_color;
				default:
					return Style::Color::Default;
			}
		};
		output += color_code(config.header_color);
		for (RequestField field : config.request_columns) std::format_to(std::back_inserter(output), "{:<{}}", field_name(field), field_width(field));
		output += Style::RESET;
		output += '\n';
		for (const Request& request : requests) {
			for (RequestField field : config.request_columns) {
				std::string value;

				switch (field) {
					case RequestField::Number:
						value = std::format("#{}", request.number);
						break;
					case RequestField::Title:
						value = truncate(request.title, config.request_title_width - 1);
						break;
					case RequestField::State:
						value = request.state;
						break;
					case RequestField::Author:
						value = request.author;
						break;
					case RequestField::Labels:
						for (std::size_t i = 0; i < request.labels.size(); ++i) {
							if (i != 0) value += ", ";
							value += request.labels[i];
						}
						break;
					case RequestField::Created:
						value = request.created;
						break;
					case RequestField::Updated:
						value = request.updated;
						break;
					case RequestField::Parent:
						value = request.parent ? std::format("#{}", *request.parent) : "-";
						break;
				}
				std::format_to(std::back_inserter(output), "{}{:<{}}{}", color_code(field_color(field, request)), value, field_width(field), Style::RESET);
			}
			output += '\n';
		}
		return output;
	};
	auto request_one = [&](const std::string& name, const ProjectContent& content) -> std::string {
		std::expected<HostOption, std::string> host_option = GitInterface::host(content);
		if (!host_option) return std::format("{}Failed to get host {}{}", color_code(config.error_color), host_option.error(), Style::RESET);
		HostOption host = *host_option;

		std::expected<std::string, std::string> remote_short_option = GitInterface::remote_short(content);
		if (!remote_short_option) return std::format("{}Failed to get short remote for {}{}", color_code(config.error_color), name, Style::RESET);
		std::string_view remote_short = *remote_short_option;

		std::string output = std::format("{}Issues for {}{}{}:\n", color_code(config.header_color), color_code(config.project_name_color), name, Style::RESET);
		std::expected<std::vector<Request>, std::string> command_output;
		switch (host) {
			case HostOption::GitHub:
				command_output = gh_interface.request_list(remote_short);
				break;
			case HostOption::GitLab:
				command_output = glab_interface.request_list(remote_short);
				break;
			case HostOption::Unknown:
				command_output = std::unexpected(std::format("{}No host found for {}{}", color_code(config.error_color), name, Style::RESET));
				break;
		}
		if (!command_output) {
			std::format_to(std::back_inserter(output), "{}Failed to list requests\n{}{}", color_code(config.error_color), command_output.error(), Style::RESET);
			return output;
		}
		if (command_output->empty()) {
			std::format_to(std::back_inserter(output), "{}No requests found{}\n", color_code(config.error_color), Style::RESET);
			return output;
		}
		std::vector<Request> requests = *command_output;

		std::format_to(std::back_inserter(output), "{}\n", format_requests(requests));

		return output;
	};
	if (name == "--all") {
		std::vector<std::future<std::string>> tasks;
		tasks.reserve(projects.size());
		for (const decltype(projects)::value_type& project : projects) if (GitInterface::is_repo(project.second)) tasks.push_back(std::async(std::launch::async, request_one, std::cref(project.first), std::cref(project.second)));
		for (std::future<std::string>& task : tasks) std::println("{}", task.get());
		return;
	}

	std::optional project_option = project_find(name);
	if (!project_option) return;

	const ProjectContent& content = (*project_option)->second;
	std::println("{}", request_one((*project_option)->first, content));
}
