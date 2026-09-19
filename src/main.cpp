#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

struct Project
{
	std::string name;
	std::filesystem::path path;
};


struct ProjCtl
{
	std::vector<Project> projects;
	std::filesystem::path config;
	ProjCtl();
	~ProjCtl();
	void load_projects();
	void add_project(Project&);
	void list_projects();
	void save_projects();
};

ProjCtl::ProjCtl()
{
	config = "/home/evrandil/.config/projctl/projctl";
}
ProjCtl::~ProjCtl()
{
}
void ProjCtl::load_projects()
{
	std::fstream projects;
	std::string path_str;
	std::string name;
	projects.open(config, std::ios::in);
	while (std::getline(projects, name, ' '))
	{
		std::getline(projects, path_str);
		this->projects.push_back(Project { name, path_str } );
	}
	projects.close();
}
void ProjCtl::list_projects()
{
	if	(projects.size() == 0)	std::cout << "There are no projects saved\n";
	for	(Project& a : projects)	std::cout << a.name << '\t' << a.path << '\n';
}

void ProjCtl::add_project(Project& project)
{
	projects.push_back(project);
}


int main(int argc, char** argv)
{
	ProjCtl proj_ctl;
	proj_ctl.load_projects();
	proj_ctl.list_projects();
	return 0;
}
