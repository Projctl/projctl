cmake_minimum_required(VERSION 3.20)

project(projctl VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(projctl
    src/main.cpp
)

add_library(projctl_core
	src/projctl/core.cpp
	src/projctl/git.cpp
	src/projctl/hosting.cpp
	src/projctl/issues.cpp
	src/projctl/projects.cpp
	src/projctl/helper.cpp
	src/systeminterface.cpp
	src/gitinterface.cpp
	src/githubinterface.cpp
	src/gitlabinterface.cpp
	src/config.cpp
)


find_package(nlohmann_json 3.12.0 QUIET)

if(NOT nlohmann_json_FOUND)
	include(FetchContent)

	FetchContent_Declare(
		nlohmann_json
		GIT_REPOSITORY https://github.com/nlohmann/json.git
		GIT_TAG v3.12.0
	)

	FetchContent_MakeAvailable(nlohmann_json)
endif()

target_link_libraries(projctl_core
	PRIVATE
		nlohmann_json::nlohmann_json
)
target_include_directories(projctl_core
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)

target_compile_options(projctl_core PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-Wconversion
	-Wsign-conversion
	-O2
)

target_link_libraries(projctl
	PRIVATE
		projctl_core
)

target_compile_options(projctl PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-Wshadow
	-Wconversion
	-Wsign-conversion
	-O2
)
