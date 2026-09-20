#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <print>

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
	save_projects();
}
void ProjCtl::load_projects()
{
	std::fstream projects;
	std::string path_str;
	std::string name;
	projects.open(config, std::ios::in);
	while (std::getline(projects, name, '\t'))
	{
		std::getline(projects, path_str);
		this->projects.push_back(Project { name, std::filesystem::path(path_str) } );
	}
	projects.close();
}
void ProjCtl::list_projects()
{
	std::println("\n\n");
	if	(projects.size() == 0)	std::println("There are no projects saved");
	else std::println("\t{:<20}{}\n", "Name", "Path");
	for	(Project& project : projects)	std::println("\t{:<20}{}", project.name, project.path.string());
}

void ProjCtl::add_project(Project& new_project)
{
	projects.push_back(new_project);
	std::println("Added project:\t{}", projects[projects.size()-1].name);
	std::println("With path:\t{}", projects[projects.size()-1].path.string());
}

void ProjCtl::save_projects()
{
	std::fstream projects;
	projects.open(config, std::ios::out);
	for (Project& project : this->projects) projects << project.name << '\t' << project.path.string() << std::endl;
}


int main(int argc, char** argv)
{
	ProjCtl proj_ctl;
	proj_ctl.load_projects();
	int arg_iterator = 1;

	while (arg_iterator != argc)
	{
		std::string command = argv[arg_iterator++];
		if (command == "add") {
			Project new_project = Project { argv[arg_iterator++], argv[arg_iterator++] };
			proj_ctl.add_project(new_project);
		}
		if (command == "list") {
			proj_ctl.list_projects();
		}
	}

	return 0;
}
