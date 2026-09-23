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
	Branch,
	Run,
	Commit,
	Push,
	Pull,
	Fetch,
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
				if (command == "push") return CommandOption::Push;
				if (command == "path") return CommandOption::Path;
				return CommandOption::Pull;
			case 'r':
				if (command == "run") return CommandOption::Run;
				return CommandOption::Remove;
			case 'o':
				return CommandOption::Open;
			case 'b':
				if (command == "branch") return CommandOption::Branch;
				return CommandOption::Build;
			case 'c':
				return CommandOption::Commit;
			case 'f':
				return CommandOption::Fetch;
		}
		return CommandOption::Idk;
	}
}

int main(const int argc, const char **argv) {
	ProjCtl proj_ctl;
	int arg_iterator = 1;

	while (arg_iterator < argc) {
		std::string command = argv[arg_iterator++];
		if (command != "path") std::print("\n");
		switch (get_option_from_command(command)) {
			case CommandOption::List: {
										  if (arg_iterator < argc && argv[arg_iterator][0] == 'g') {
											  ++arg_iterator;
											  proj_ctl.list_gits();
										  }
										  else proj_ctl.list();
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
											bool push = false;
											bool branch = false;
											if (arg_iterator < argc && std::string(argv[arg_iterator]) == "push") {
												push = true;
												++arg_iterator;
												if (arg_iterator < argc && std::string(argv[arg_iterator]) == "branch") {
													branch = true;
													++arg_iterator;
												}
											}
											if (arg_iterator + 1 >= argc) {
												std::print("You're missing some arguments!");
												continue;
											}
											std::string name = argv[arg_iterator++];
											std::string_view message = argv[arg_iterator++];
											proj_ctl.git_commit(name, message);
											if (push) {
												if(branch) proj_ctl.git_push_branch(name);
												else proj_ctl.git_push(name);
											}
											continue;
										}
			case CommandOption::Push: {
										  if (arg_iterator < argc && std::string(argv[arg_iterator]) == "branch") {
											  ++arg_iterator;
											  proj_ctl.git_push_branch(std::string(argv[arg_iterator++]));
											  continue;
										  } else if (arg_iterator >= argc) {
											  std::print("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.git_push(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Pull: {
										  if (arg_iterator >= argc) {
											  std::print("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.git_pull(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Branch: {
											if (arg_iterator >= argc) {
												std::print("You're missing some arguments!");
												continue;
											}
											if (arg_iterator + 1 == argc) {
												proj_ctl.git_branch_list(std::string(argv[arg_iterator++]));
												continue;
											}
											std::string name = std::string(argv[arg_iterator++]);
											std::string_view branch = std::string_view(argv[arg_iterator++]);
											proj_ctl.git_branch(name, branch);
											continue;
										}
			case CommandOption::Fetch: {
										   if (arg_iterator >= argc) {
											   std::print("You're missing some arguments!");
											   continue;
										   }
										   proj_ctl.git_fetch(std::string(argv[arg_iterator++]));
										   continue;
									   }
			case CommandOption::Idk: {
										 std::print("Unrecognized command");
										 continue;
									 }
		}
		if (command != "path")	std::print("\n\n");
	}

	return 0;
}
