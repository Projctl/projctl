cmake_minimum_required(VERSION 3.20)

project(projctl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(projctl
    src/main.cpp
)

add_library(projctl_core
	src/projctl.cpp
	src/projectcontent.cpp
	src/systeminterface.cpp
	src/gitinterface.cpp
)

target_include_directories(projctl_core
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)

target_compile_options(projctl_core PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-Wshadow
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
