#include "githubinterface.hpp"

#include "systeminterface.hpp"
#include <nlohmann/json.hpp>

bool GitHubInterface::installed() const {
	return SystemInterface::run("command -v gh").has_value();
}
bool GitHubInterface::authenticated() const {
	return SystemInterface::run("gh auth status").has_value();
}

std::expected<std::string, std::string> GitHubInterface::repo_list() const {
	std::optional<std::string> output = SystemInterface::run("gh repo list | awk '{ print $1 }'");

	if (!output) return std::unexpected("Failed to get GitHub repo list");

	return *output;
}

std::expected<std::vector<Issue>, std::string> GitHubInterface::issue_list(std::string_view remote_short) const {
	if (!installed()) return std::unexpected("GitHub not installed");

	std::string command = std::format("CLICOLOR_FORCE=0 NO_COLOR=1 gh issue list --repo {} --json number,title,state,author,labels,createdAt,updatedAt,parent", remote_short);
	std::optional<std::string> output = SystemInterface::run(command);

	if (!output) return std::unexpected(std::format("Failed to get GitHub issue list for repo {}", remote_short));
	nlohmann::json json = nlohmann::json::parse(*output);
	std::vector<Issue> issues;
	issues.reserve(json.size());
	for (const nlohmann::json& json_issue : json) {
		Issue issue;
		issue.number = json_issue.at("number").get<int>();
		issue.title = json_issue.at("title").get<std::string>();
		issue.state = json_issue.at("state").get<std::string>();
		issue.created = json_issue.at("createdAt").get<std::string>();
		issue.updated = json_issue.at("updatedAt").get<std::string>();
		if(!json_issue.at("parent").is_null()) issue.parent = json_issue.at("parent").at("title").get<std::string>();

		if (!json_issue.at("author").is_null()) issue.author = json_issue.at("author").at("name").get<std::string>();

		for (const nlohmann::json& label : json_issue.at("labels")) issue.labels.push_back(label.at("name").get<std::string>());

		issues.push_back(std::move(issue));
	}

	return issues;
}

std::expected<void, std::string> GitHubInterface::issue_create(std::string_view remote_short) const {
	if (!installed()) return std::unexpected("GitHub not installed");

	std::string command = std::format("gh issue create --repo {}", remote_short);
	SystemInterface::run_interactive(command);
}
