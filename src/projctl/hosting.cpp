#include "projctl.hpp"

#include "config.hpp"
#include <future>
#include <print>

void ProjCtl::g_status () const {
	auto status_one = [&](const std::string_view name, auto* interface){
		std::string output;
		std::format_to(std::back_inserter(output), "{}{:<{}}{}{}{}{}\n", color_code(config.header_color), std::format("{} Cli", name), config.name_width, interface->installed() ? color_code(config.tick_color) : color_code(config.error_color), interface->installed() ? Style::TICK : Style::CROSS, interface->installed() ? "Installed" : "Not installed", Style::RESET);
		std::format_to(std::back_inserter(output), "{}{:<{}}{}{}{}{}\n", color_code(config.header_color), "Auth", config.name_width, interface->authenticated() ? color_code(config.tick_color) : color_code(config.error_color), interface->authenticated() ? Style::TICK : Style::CROSS, interface->authenticated() ? "Authenticated" : "Not authenticated", Style::RESET);
		return output;
	};
	std::future<std::string> gh_task = std::async(std::launch::async, status_one, "GitHub", &gh_interface);
	std::future<std::string> glab_task = std::async(std::launch::async, status_one, "GitLab", &glab_interface);
	std::println("{}", gh_task.get());
	std::println("{}", glab_task.get());
}

void ProjCtl::repo_list() const {
	auto list_one = [&](const std::string_view name, auto* interface){
		std::string output;
		if (!interface->installed()) return std::format("{}{}{} is not installed{}", color_code(config.error_color), Style::CROSS, name, Style::RESET);
		if (!interface->authenticated()) return std::format("{}{}{} is not authenticated{}", color_code(config.error_color), Style::CROSS, name, Style::RESET);
		std::expected<std::string, std::string> repo_list = interface->repo_list();
		std::format_to(std::back_inserter(output), "{}{} {}repos\n{}{}{}", color_code(config.project_name_color), name, color_code(config.header_color), repo_list ? color_code(config.project_short_remote_color) : color_code(config.error_color), repo_list ? *repo_list : repo_list.error(), Style::RESET);
		return output;
	};
	std::future<std::string> gh_task = std::async(std::launch::async, list_one, "GitHub", &gh_interface);
	std::future<std::string> glab_task = std::async(std::launch::async, list_one, "GitLab", &glab_interface);
	std::println("{}\n", gh_task.get());
	std::println("{}", glab_task.get());
}
