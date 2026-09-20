#include <string>
#include <print>

#include "projctl.hpp"





int main(int argc, char** argv)
{
	ProjCtl proj_ctl;
	int arg_iterator = 1;

	while (arg_iterator != argc)
	{
		std::println("\n\n");
		std::string command = argv[arg_iterator++];
		if (command == "list") {
			proj_ctl.list_projects();
		}
		if (command == "status")
		{
			proj_ctl.project_status(std::string (argv[arg_iterator++]));
		}
		if (command == "add")
		{
			std::pair<std::string, ProjectContent> new_project = {argv[arg_iterator++], { argv[arg_iterator++] }};
			proj_ctl.project_add(new_project);
		}
		if (command == "path")
		{
			proj_ctl.path_show(std::string (argv[arg_iterator++]));
		}
		if (command == "remove" || command == "rm") {
			proj_ctl.project_remove(std::string (argv[arg_iterator++]));
		}
	}
	std::println("\n\n");

	return 0;
}
