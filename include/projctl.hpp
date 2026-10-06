#pragma once

#include "projectcontent.hpp"
#include "githubinterface.hpp"
#include "gitlabinterface.hpp"
#include "config.hpp"

#include <filesystem>
#include <map>

constexpr std::string_view MARGIN = "    ";

ProjectType detect_project_type(const std::filesystem::path&);
std::string trim(std::string_view);
std::string truncate(std::string_view, size_t);

class ProjCtl
{
	std::map<std::string, ProjectContent> projects;
	std::filesystem::path config_path;
	Config config;
	std::filesystem::path data_path;
	GitHubInterface gh_interface;
	GitLabInterface glab_interface;
	void load();
	void save() const;
	std::optional<std::map<std::string, ProjectContent>::const_iterator> project_find(const std::string&) const;
	std::optional<std::string> current_project_name() const;

	public:
	ProjCtl();
	~ProjCtl();
	void list() const;
	void list_gits() const;
	void path_show(const std::string&) const;
	void add(const std::string, std::filesystem::path);
	void add_current();
	void remove(const std::string&);
	void status(const std::string&) const;
	void open(const std::string&) const;
	void build(const std::string&) const;
	void run(const std::string&) const;
	void git_commit(const std::string&, std::string_view) const;
	void git_push(const std::string&) const;
	void git_push_branch(const std::string&) const;
	void git_pull(const std::string&) const;
	void git_fetch(const std::string&) const;
	void git_branch_list(const std::string&) const;
	void git_branch(const std::string&, std::string_view) const;
	bool git_merge(const std::string&) const;
	void git_automerge(const std::string&, std::string_view) const;
	void g_status() const;
	void repo_list() const;
	void issue_list(const std::string&) const;
	void issue_create(const std::string&) const;
};
