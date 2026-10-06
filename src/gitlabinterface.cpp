#include "gitlabinterface.hpp"

#include "systeminterface.hpp"
#include <nlohmann/json.hpp>
#include <print>

bool GitLabInterface::installed() const {
	return SystemInterface::run("command -v glab").has_value();
}
bool GitLabInterface::authenticated() const {
	return SystemInterface::run("glab auth status >/dev/null 2>&1").has_value();
}

std::expected<std::string, std::string> GitLabInterface::repo_list() const {
	std::optional<std::string> output = SystemInterface::run("glab repo list | awk 'NR > 3 { print $1 }'");

	if (!output) return std::unexpected("Failed to get GitLab repo list");
	return *output;
}

std::expected<std::vector<Issue>, std::string> GitLabInterface::issue_list(std::string_view remote_short) const {
	if (!installed()) return std::unexpected("GitLab not installed");

	std::string command = std::format("glab issue list --repo {} --output json", remote_short);
	std::optional<std::string> output = SystemInterface::run(command);

	if (!output) return std::unexpected(std::format("Failed to get GitLab issue list for repo {}", remote_short));
	nlohmann::json json = nlohmann::json::parse(*output);
	std::vector<Issue> issues;
	issues.reserve(json.size());
	for (const nlohmann::json& json_issue : json) {
		Issue issue;
		issue.number = json_issue.at("iid").get<int>();
		issue.title = json_issue.at("title").get<std::string>();
		issue.state = json_issue.at("state").get<std::string>();
		issue.created = json_issue.at("created_at").get<std::string>();
		issue.updated = json_issue.at("updated_at").get<std::string>();
		issue.author = json_issue.at("author").at("username").get<std::string>();

		for (const nlohmann::json& label : json_issue.at("labels")) issue.labels.push_back(label.get<std::string>());

		issues.push_back(std::move(issue));
	}

	return issues;
}

std::expected<void, std::string> GitLabInterface::issue_create(std::string_view remote_short) const {
	if (!installed()) return std::unexpected("GitLab not installed");

	std::string command = std::format("glab issue create --repo {}", remote_short);
	SystemInterface::run_interactive(command);
}

std::expected<std::vector<Request>, std::string> GitLabInterface::request_list(std::string_view remote_short) const {
	if (!installed()) return std::unexpected("GitLab not installed");

	std::string command = std::format("glab mr list --repo {} --output json", remote_short);
	std::optional<std::string> output = SystemInterface::run(command);

	if (!output) return std::unexpected(std::format("Failed to get GitLab mr list for repo {}", remote_short));
	nlohmann::json json = nlohmann::json::parse(*output);
	std::vector<Request> requests;
	requests.reserve(json.size());
	for (const nlohmann::json& json_request : json) {
		Request request;
		request.number = json_request.at("iid").get<int>();
		request.title = json_request.at("title").get<std::string>();
		request.state = json_request.at("state").get<std::string>();
		request.created = json_request.at("created_at").get<std::string>();
		request.updated = json_request.at("updated_at").get<std::string>();
		request.author = json_request.at("author").at("username").get<std::string>();

		for (const nlohmann::json& label : json_request.at("labels")) request.labels.push_back(label.get<std::string>());

		requests.push_back(std::move(request));
	}

	return requests;
}

std::expected<void, std::string> GitLabInterface::request_create(std::string_view path) const {
	if (!installed()) return std::unexpected("GitLab not installed");

	std::string command = std::format("cd {} && glab mr create", path);
	SystemInterface::run_interactive(command);
}

std::expected<std::string, std::string> GitLabInterface::request_merge(std::string_view path) const {
	if (!installed()) return std::unexpected("GitLab not installed");

	std::string command = std::format("cd {} && glab mr merge --squash --remove-source-branch", path);
	std::optional output = SystemInterface::run(command);
	if (!output) return std::unexpected(std::format("Failed to run GitLab merge for repo {}", path));

	return *output;
}
