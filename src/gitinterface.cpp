#include "gitinterface.hpp"


GitInterface::GitInterface(SystemInterface& system_interface)
	: system_interface(system_interface) {
}

bool GitInterface::is_repo(const ProjectContent& project) {
	return std::filesystem::exists(project.path/".git");
}

std::optional<std::string> GitInterface::add(const ProjectContent& project) {
	std::string command = std::string("cd \"").append(project.path.string()).append("\" && git add .");
	return system_interface.run(command);
}

std::expected<std::string, std::string> GitInterface::commit(const ProjectContent& project, std::string_view message) {
	if (!is_repo(project)) return std::unexpected("Project is not git repository");

	std::optional<std::string> add_output = add(project);
	if (!add_output) return std::unexpected("Git add failed");

	std::string command = std::string("cd \"").append(project.path.string()).append("\" && git commit -m \"").append(message).append("\"");;

	std::optional<std::string> commit_output = system_interface.run(command);
	if (!commit_output) return std::unexpected("Git commit failed");

	add_output->append(*commit_output);
	return *add_output;
}
