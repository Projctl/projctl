#pragma once

#include "projectcontent.hpp"
#include "systeminterface.hpp"
#include "gitinterface.hpp"
#include "githubinterface.hpp"
#include "gitlabinterface.hpp"
#include "config.hpp"

#include <filesystem>
#include <map>

class ProjCtl
{
	std::map<std::string, ProjectContent> projects;
	std::filesystem::path config_path;
	Config config;
	std::filesystem::path data_path;
	SystemInterface system_interface;
	GitInterface git_interface;
	GitHubInterface gh_interface;
	GitLabInterface glab_interface;
	void load();
	void save();
	std::optional<std::map<std::string, ProjectContent>::const_iterator> project_find(const std::string&);
	std::optional<std::string> current_project_name() const;

	public:
	ProjCtl();
	~ProjCtl();
	void list();
	void list_gits();
	void path_show(const std::string&);
	void add(const std::string, std::filesystem::path);
	void add_current();
	void remove(const std::string&);
	void status(const std::string&);
	void open(const std::string&);
	void build(const std::string&);
	void run(const std::string&);
	void git_commit(const std::string&, std::string_view);
	void git_push(const std::string&);
	void git_push_branch(const std::string&);
	void git_pull(const std::string&);
	void git_fetch(const std::string&);
	void git_branch_list(const std::string&);
	void git_branch(const std::string&, std::string_view);
	bool git_merge(const std::string&);
	void git_automerge(const std::string&, std::string_view);
	void g_status();
	void repo_list();
	void issue_list(const std::string& name);
};
