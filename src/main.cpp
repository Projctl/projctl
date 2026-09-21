#include <print>
#include <string>

#include "projctl.hpp"

enum class CommandOption {
	List,
	Status,
	Add,
	Path,
	Remove,
	Open,
	Build,
	Run,
	Commit,
	Idk
};

namespace {
	CommandOption get_option_from_command(const std::string& command) {
		switch (command[0]) {
			case 'l':
				return CommandOption::List;
			case 's':
				return CommandOption::Status;
			case 'a':
				return CommandOption::Add;
			case 'p':
				return CommandOption::Path;
			case 'r':
				if (command[1] == 'u') return CommandOption::Run;
				return CommandOption::Remove;
			case 'o':
				return CommandOption::Open;
			case 'b':
				return CommandOption::Build;
			case 'c':
				return CommandOption::Commit;
		}
		return CommandOption::Idk;
	}
}

int main(const int argc, const char **argv) {
	ProjCtl proj_ctl;
	int arg_iterator = 1;

	while (arg_iterator < argc) {
		std::print("\n\n");
		std::string command = argv[arg_iterator++];
		switch (get_option_from_command(command)) {
			case CommandOption::List: {
										  proj_ctl.list();
										  continue;
									  }
			case CommandOption::Status: {
											if (arg_iterator >= argc) {
												std::print("You're missing some arguments!");
												continue;
											}
											proj_ctl.status(std::string(argv[arg_iterator++]));
											continue;
										}
			case CommandOption::Add: {
										 if (arg_iterator >= argc) {
											 std::print("You're missing some arguments!");
											 continue;
										 }
										 if (std::string_view(argv[arg_iterator]) == ".") {
											 ++arg_iterator;
											 proj_ctl.add_current();
											 continue;
										 }
										 if (arg_iterator + 2 > argc) {
											 std::print("You're missing some arguments!");
											 continue;
										 }
										 std::string name = std::string(argv[arg_iterator++]);
										 std::filesystem::path path = argv[arg_iterator++];
										 proj_ctl.add(name, path);
										 continue;
									 }
			case CommandOption::Path: {
										  if (arg_iterator >= argc) {
											  std::print("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.path_show(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Remove: {
											if (arg_iterator >= argc) {
												std::print("You're missing some arguments!");
												continue;
											}
											proj_ctl.remove(std::string(argv[arg_iterator++]));
											continue;
										}
			case CommandOption::Open: {
										  if (arg_iterator >= argc) {
											  std::print("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.open(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Build: {
										   if (arg_iterator >= argc) {
											   std::print("You're missing some arguments!");
											   continue;
										   }
										   proj_ctl.build(std::string(argv[arg_iterator++]));
										   continue;
									   }
			case CommandOption::Run: {
										 if (arg_iterator >= argc) {
											 std::print("You're missing some arguments!");
											 continue;
										 }
										 proj_ctl.run(std::string(argv[arg_iterator++]));
										 continue;
									 }
			case CommandOption::Commit: {
											if (arg_iterator + 1 >= argc) {
												std::print("You're missing some arguments!");
												continue;
											}
											std::string name = argv[arg_iterator++];
											std::string_view message = argv[arg_iterator++];
											proj_ctl.git_commit(name, message);
											continue;
										}
			case CommandOption::Idk: {
										 std::print("Unrecognized command");
										 continue;
									 }
		}
	}
	std::print("\n\n");

	return 0;
}
