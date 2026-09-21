#include "gitinterface.hpp"

#include <format>
#include <optional>

GitInterface::GitInterface(SystemInterface& system_interface)
	: system_interface(system_interface) {
}

bool GitInterface::is_repo(const ProjectContent& project) {
	std::string command = std::format("git -C \"{}\" rev-parse --is-inside-work-tree", project.path.string());
	std::optional<std::string> output = system_interface.run(command);
	return output && *output == "true";
}

std::expected<std::string, std::string> GitInterface::add(const ProjectContent& project) {
	if (!is_repo(project)) return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" add .", project.path.string());
	std::optional<std::string> result = system_interface.run(command);

	if (!result) return std::unexpected("Git add failed");
	return *result;
}

std::expected<std::string, std::string> GitInterface::commit(const ProjectContent& project, std::string_view message) {
	if (!is_repo(project)) return std::unexpected("Project is not git repository");

	std::expected<std::string, std::string> add_output = add(project);
	if (!add_output) return std::unexpected(add_output.error());

	std::string command = std::format("git -C \"{}\" commit -m \"{}\"", project.path.string(), message);

	std::optional<std::string> commit_output = system_interface.run(command);
	if (!commit_output) return std::unexpected("Git commit failed");

	add_output->append("\n");
	add_output->append(*commit_output);

	return *add_output;
}

std::expected<std::string, std::string> GitInterface::push(const ProjectContent& project) {
	if (!is_repo(project)) return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" push", project.path.string());

	std::optional<std::string> result = system_interface.run(command);

	if (!result) return std::unexpected("Git push failed");

	return *result;
}
