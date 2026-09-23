# Project Control
Have you ever wondered if you're in gh or glab repo so what command to run?
Have you ever mistook one for other?
I did
And it was annoying
So here it is wrapper for git management \(haven't rly done anything on CodeBerg so I don't have that one wrapped\)
Tho honestly it's useful not only for git projects

`projctl branch has to be used at the end if you want branches to be listed`
Dependencies:
```
cmake 3.20
ninja
just
```
C++ standard is set to 2023 so I guess that's requirement for compiler
Also have $XDG_CONFIG_HOME and $XDG_CACHE_HOME set to some location
```
./install.fish
```


# Usage \[fast docs\]

### List
Lists projects
With gits or git will list only git repos
```
projctl list 
projctl list gits
```
### Status
Gives status of project \(/ all projects if used with --all flag\)
Status contains
    Name:               \[Name\]<br>
    Path:               \[Path\]
    Exists:             \[If project exists on disk, "No" if it has been removed / moved\]
    Type:               \[Lang / environment, currently supported are: \{ "C++/CMake", "Rust", "Node", "Python" \} \]
    Git:                \[\(If git repo\) current workspace/repo_name\]
    Branch:             \[\(If git repo\) current working branch\]
    Full remote:        \[\(If git repo\) full remote path\]
    Status:             \[\(If git repo\)If there were unpushed / uncommitted changes, if yes lists files with changes below\]
```
projctl status [name]
projctl status --all
```
### Add
Adds project to saved projects
You can set it to auto-add current directory as project with name of dir
Also you can set name of dir and add 
USE OBJECTIVE PATHS for now at least
Tho "." I did make to be converted to objective path of current dir
```
projctl add [name] [path]
projctl add [name] .
projctl add .
```
### Remove
Removes project
\(Doesn't actually remove files, just from projctl list\)
```
projctl remove [name]
projctl rm [name]
```
### Open
Opens neovim in project location
Soon \(I hope\) will open editor set in config but well not on that stage yet
```
projctl open [name]
```
### Path
Gives path of project
Nice for `cd (projctl path $argv[1])` terminal macro
```
projctl path [name]
```
### Build && run
Respectively builds and runs projects that either have customely added `build` and `run` commands or have supported default commands \(Rust && node supported for both, CMake + Ninja does have build\)
Build also supports --all flag
```
projctl build [name]
projctl build --all
projctl run [name]
```
## Git wrapper
Here we go into the rabbithole
### Branch
Switches / creates and switches to branch of given name
```
projctl branch [name] [branch_name]
```
### Fetch
Fetches project
Supports --all flag \(performs fetch to all git projects\)
```
projctl fetch [name]
projctl fetch --all
```
### Pull
Updates project
Supports --all flag \(performs pull for all git projects\)
```
projctl pull [name]
projctl pull --all
```
### Commit
Adds all changes and makes commit with provided message \(equivalent to `git add . && git commit -m \[commit_message\]`\)
```
projctl commit [name] [commit_message]
```
### Push
Pushes committed changes
If used with branch pushes changes to uninitialized branch \(equivalent to `git push -u origin \[current_branch\]`\)
```
projctl push [name]
projctl push branch [name]
```
### Commit push
Performs both actions, you can combo it with branch as well to set upstream to new branch
```
projctl commit push [name] [commit_message]
projctl commit push branch [name] [commit_message]
```

## Important note
Just so you know it does wrap around all args so you can do mutliple actions in one prompt but just beware that for every action you do need to provide all needed components
```
projctl status project_1 commit push project_2 "commit_message" build project_3 rm project_4
```

Will perform
```
projctl status project_1
projctl commit push project_2 "commit_message"
projctl build project_3
projctl rm project_4
```
In that exact order with those arguments provided
Also minor thing but for process to be actually saving some time everything with --all flag is done in parrallel but for that to happen program stores output and when everything is done prints it in order so there's no classic `some console print -> some waiting -> some more printing` for example in pull command, everything gets printed all at once when waiting is done, but thanks to that all thing are async so it' kinda faster one could argue
