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
	GStatus,
	RepoList,
	IssueList,
	IssueCreate,
	AutoMerge,
	RequestList,
	RequestCreate,
	RequestMerge,
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
				if (command == "automerge") return CommandOption::AutoMerge;
				return CommandOption::Add;
			case 'p':
				if (command == "push") return CommandOption::Push;
				if (command == "path") return CommandOption::Path;
				return CommandOption::Pull;
			case 'r':
				if (command == "repos") return CommandOption::RepoList;
				if (command == "run") return CommandOption::Run;
				if (command == "reqs") return CommandOption::RequestList;
				if (command == "rm" || command == "remove") return CommandOption::Remove;
				if (command == "reqcreate") return CommandOption::RequestCreate;
				if (command == "reqmerge") return CommandOption::RequestMerge;
				return CommandOption::Idk;
			case 'o':
				return CommandOption::Open;
			case 'b':
				if (command == "branch") return CommandOption::Branch;
				return CommandOption::Build;
			case 'c':
				return CommandOption::Commit;
			case 'f':
				return CommandOption::Fetch;
			case 'g':
				if (command == "git") return CommandOption::GStatus;
				break;
			case 'i':
				if (command == "icreate") return CommandOption::IssueCreate;
				return CommandOption::IssueList;
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
												std::println("You're missing some arguments!");
												continue;
											}
											proj_ctl.status(std::string(argv[arg_iterator++]));
											continue;
										}
			case CommandOption::Add: {
										 if (arg_iterator >= argc) {
											 std::println("You're missing some arguments!");
											 continue;
										 }
										 if (std::string_view(argv[arg_iterator]) == ".") {
											 ++arg_iterator;
											 proj_ctl.add_current();
											 continue;
										 }
										 if (arg_iterator + 2 > argc) {
											 std::println("You're missing some arguments!");
											 continue;
										 }
										 std::string name = std::string(argv[arg_iterator++]);
										 std::filesystem::path path = argv[arg_iterator++];
										 proj_ctl.add(name, path);
										 continue;
									 }
			case CommandOption::Path: {
										  if (arg_iterator >= argc) {
											  std::println("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.path_show(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Remove: {
											if (arg_iterator >= argc) {
												std::println("You're missing some arguments!");
												continue;
											}
											proj_ctl.remove(std::string(argv[arg_iterator++]));
											continue;
										}
			case CommandOption::Open: {
										  if (arg_iterator >= argc) {
											  std::println("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.open(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Build: {
										   if (arg_iterator >= argc) {
											   std::println("You're missing some arguments!");
											   continue;
										   }
										   proj_ctl.build(std::string(argv[arg_iterator++]));
										   continue;
									   }
			case CommandOption::Run: {
										 if (arg_iterator >= argc) {
											 std::println("You're missing some arguments!");
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
												std::println("You're missing some arguments!");
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
			case CommandOption::AutoMerge: {
											   if (arg_iterator + 1 >= argc) {
												   std::println("You're missing some arguments!");
												   continue;
											   }
											   std::string name = argv[arg_iterator++];
											   std::string_view message = argv[arg_iterator++];
											   proj_ctl.git_automerge(name, message);
											   continue;
										   }
			case CommandOption::Push: {
										  if (arg_iterator < argc && std::string(argv[arg_iterator]) == "branch") {
											  ++arg_iterator;
											  proj_ctl.git_push_branch(std::string(argv[arg_iterator++]));
											  continue;
										  } else if (arg_iterator >= argc) {
											  std::println("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.git_push(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Pull: {
										  if (arg_iterator >= argc) {
											  std::println("You're missing some arguments!");
											  continue;
										  }
										  proj_ctl.git_pull(std::string(argv[arg_iterator++]));
										  continue;
									  }
			case CommandOption::Branch: {
											if (arg_iterator >= argc) {
												std::println("You're missing some arguments!");
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
											   std::println("You're missing some arguments!");
											   continue;
										   }
										   proj_ctl.git_fetch(std::string(argv[arg_iterator++]));
										   continue;
									   }
			case CommandOption::GStatus: {
											 proj_ctl.g_status();
											 continue;
										 }
			case CommandOption::RepoList: {
											  proj_ctl.repo_list();
											  continue;
										  }
			case CommandOption::IssueList: {
											   if (arg_iterator >= argc) {
												   std::println("You're missing some arguments!");
												   continue;
											   }
											   proj_ctl.issue_list(std::string(argv[arg_iterator++]));
											   continue;
										   }
			case CommandOption::IssueCreate: {
												 if (arg_iterator >= argc) {
													 std::println("You're missing some arguments!");
													 continue;
												 }
												 proj_ctl.issue_create(std::string(argv[arg_iterator++]));
												 continue;
											 }
			case CommandOption::RequestCreate: {
												   if (arg_iterator >= argc) {
													   std::println("You're missing some arguments!");
													   continue;
												   }
												   proj_ctl.request_create(std::string(argv[arg_iterator++]));
												   continue;
											   }
			case CommandOption::RequestList: {
												 if (arg_iterator >= argc) {
													 std::println("You're missing some arguments");
													 continue;
												 }
												 proj_ctl.request_list(std::string(argv[arg_iterator++]));
												 continue;
											 }
			case CommandOption::RequestMerge: {
												 if (arg_iterator >= argc) {
													 std::println("You're missing some arguments");
													 continue;
												 }
												 proj_ctl.request_merge(std::string(argv[arg_iterator++]));
												 continue;
											  }
			default: {
						 std::print("Unrecognized command");
						 continue;
					 }
		}
		if (command != "path")	std::print("\n\n");
	}

	return 0;
}
