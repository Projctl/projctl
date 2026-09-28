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

std::expected<std::string, std::string> GitInterface::changes(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" status --porcelain", project.path.string());
	std::optional<std::string> output = system_interface.run(command);

	if (!output) [[unlikely]] return std::unexpected("Git status failed");
	return *output;
}

std::expected<std::string, std::string> GitInterface::branch(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" branch --show-current", project.path.string());
	std::optional<std::string> output = system_interface.run(command);

	if (!output) [[unlikely]] return std::unexpected("Git remote failed");
	return *output;
}

std::expected<std::string, std::string> GitInterface::branch_list(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" branch --list", project.path.string());
	std::optional<std::string> output = system_interface.run(command);

	if (!output) [[unlikely]] return std::unexpected("Git remote failed");
	return *output;
}

std::expected<std::string, std::string> GitInterface::branch_switch(const ProjectContent& project, std::string_view branch) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" branch --list \"{}\" 2>&1", project.path.string(), branch);
	std::optional<std::string> output = system_interface.run(command);
	if (!output) [[unlikely]] return std::unexpected("Git branch --list failed");

	bool branch_exists = !output->empty();
	command = std::format("git -C \"{}\" switch {}\"{}\" 2>&1", project.path.string(), branch_exists ? "" : "-c " , branch);
	output = system_interface.run(command);
	if (!output) [[unlikely]] return std::unexpected(branch_exists ? "Git branch switch failed" : "Git branch creation failed");
	return *output;
}

std::expected<std::string, std::string> GitInterface::remote(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" remote get-url origin", project.path.string());
	std::optional<std::string> output = system_interface.run(command);

	if (!output) [[unlikely]] return std::unexpected("Git remote failed");
	return *output;
}
std::expected<std::string, std::string> GitInterface::remote_short(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::expected<std::string, std::string> remote_output = remote(project);
	if (!remote_output) [[unlikely]] return std::unexpected(remote_output.error());

	std::string remote = *remote_output;
	if (remote.ends_with(".git")) remote.erase(remote.size() - 4);

	std::size_t separator = remote.find_last_of('/');
	if (separator == std::string::npos) [[unlikely]] return std::unexpected("Something went wrong while parsing remote's name");

	const std::string repo = remote.substr(separator + 1);

	remote.erase(separator);
	separator = remote.find_last_of("/:");
	if (separator == std::string::npos) [[unlikely]] return std::unexpected("Something went wrong while parsing remote's userspace");

	const std::string owner = remote.substr(separator + 1);

	return std::format("{}/{}", owner, repo);
}
std::expected<HostOption, std::string> GitInterface::host(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::expected<std::string, std::string> remote_output = remote(project);
	if (!remote_output) [[unlikely]] return std::unexpected(remote_output.error());

	std::string remote = *remote_output;
	std::size_t separator = remote.find_last_of('/'); // repo name
	if (separator == std::string::npos) [[unlikely]] return std::unexpected("Something went wrong while parsing host's name");
	remote.erase(separator);

	separator = remote.find_last_of("/:"); // repo userspace
	if (separator == std::string::npos) [[unlikely]] return std::unexpected("Something went wrong while parsing host's name");
	remote.erase(separator);

	separator = remote.find_last_of('.'); // .com / other domain
	if (separator == std::string::npos) [[unlikely]] return std::unexpected("Something went wrong while parsing host's name");
	remote.erase(separator);

	separator = remote.find_last_of("@."); // hostname
	if (separator == std::string::npos) [[unlikely]] return std::unexpected("Something went wrong while parsing host's name");

	std::string host = remote.substr(separator +  1);
	if (host == "github") return HostOption::GitHub;
	if (host == "gitlab") return HostOption::GitLab;
	return HostOption::Unknown;
}

std::expected<std::string, std::string> GitInterface::add(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" add . 2>&1", project.path.string());
	std::optional<std::string> result = system_interface.run(command);

	if (!result) [[unlikely]] return std::unexpected("Git add failed");
	return *result;
}

std::expected<std::string, std::string> GitInterface::commit(const ProjectContent& project, std::string_view message) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Project is not git repository");

	std::expected<std::string, std::string> add_output = add(project);
	if (!add_output) [[unlikely]] return std::unexpected(add_output.error());

	std::string command = std::format("git -C \"{}\" commit -m \"{}\" 2>&1", project.path.string(), message);

	std::optional<std::string> commit_output = system_interface.run(command);
	if (!commit_output) [[unlikely]] return std::unexpected("Git commit failed");

	add_output->append("\n");
	add_output->append(*commit_output);

	return *add_output;
}

std::expected<std::string, std::string> GitInterface::push(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" push 2>&1", project.path.string());

	std::optional<std::string> result = system_interface.run(command);

	if (!result) [[unlikely]] return std::unexpected("Git push failed");

	return *result;
}

std::expected<std::string, std::string> GitInterface::push_branch(const ProjectContent& project, std::string_view branch) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" push -u origin {} 2>&1", project.path.string(), branch);

	std::optional<std::string> result = system_interface.run(command);

	if (!result) [[unlikely]] return std::unexpected("Git push failed");

	return *result;
}

std::expected<std::string, std::string> GitInterface::pull(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" pull 2>&1", project.path.string());

	std::optional<std::string> result = system_interface.run(command);

	if (!result) [[unlikely]] return std::unexpected("Git pull failed");

	return *result;
}
std::expected<std::string, std::string> GitInterface::fetch(const ProjectContent& project) {
	if (!is_repo(project)) [[unlikely]] return std::unexpected("Not a git repo");

	std::string command = std::format("git -C \"{}\" fetch 2>&1", project.path.string());

	std::optional<std::string> result = system_interface.run(command);

	if (!result) [[unlikely]] return std::unexpected("Git fetch failed");

	return *result;
}
