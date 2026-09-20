#include <print>
#include <string>

#include "projctl.hpp"

int main(const int argc, const char **argv) {
	ProjCtl proj_ctl;
	int arg_iterator = 1;

	while (arg_iterator != argc) {
		std::print("\n\n");
		std::string command = argv[arg_iterator++];
		if (command == "list") {
			proj_ctl.list_projects();
		}
		if (command == "status") {
			if (argc < arg_iterator + 1) {
				std::print("You're missing some arguments!");
				continue;
			}
			proj_ctl.project_status(std::string(argv[arg_iterator++]));
		}
		if (command == "add") {
			if (argc < arg_iterator) {
				std::print("You're missing some arguments!");
				continue;
			}
			if (std::string(argv[arg_iterator++]).contains('.')) {
				proj_ctl.project_add_current();
				continue;
			} else --arg_iterator;
			if (argc < arg_iterator + 2) {
				std::print("You're missing some arguments!");
				continue;
			}
			std::string name = std::string(argv[arg_iterator++]);
			std::filesystem::path path = argv[arg_iterator++];
			std::println("Ayo");
			proj_ctl.project_add(name, path);
		}
		if (command == "path") {
			if (argc < arg_iterator + 1) {
				std::print("You're missing some arguments!");
				continue;
			}
			proj_ctl.path_show(std::string(argv[arg_iterator++]));
		}
		if (command == "remove" || command == "rm") {
			if (argc < arg_iterator + 1) {
				std::print("You're missing some arguments!");
				continue;
			}
			proj_ctl.project_remove(std::string(argv[arg_iterator++]));
		}
		if (command == "open") {
			if (argc < arg_iterator + 1) {
				std::print("You're missing some arguments!");
				continue;
			}
			proj_ctl.project_open(std::string(argv[arg_iterator++]));
		}
		if (command == "build") {
			if (argc < arg_iterator + 1) {
				std::print("You're missing some arguments!");
				continue;
			}
			proj_ctl.project_build(std::string(argv[arg_iterator++]));
		}
	}
	std::print("\n\n");

	return 0;
}
