#include "gitinterface.hpp"


GitInterface::GitInterface(SystemInterface& system_interface)
	: system_interface(system_interface) {
}

std::optional<std::string> GitInterface::add(const ProjectContent& project) {
	std::string command = std::string("cd \"").append(project.path.string()).append("\" && git add .");
	return system_interface.run(command);
}

std::optional<std::string> GitInterface::commit(const ProjectContent& project, std::string_view message) {
	std::optional<std::string> output = add(project);
	if (!output) return std::nullopt;

	std::string command = std::string("cd \"").append(project.path.string()).append("\" && git commit -m \"").append(message).append("\"");;

	output->append(*system_interface.run(command));

	return output;
}
