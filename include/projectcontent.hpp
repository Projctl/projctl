#pragma once

#include <filesystem>
#include <string>
#include <string_view>

enum class ProjectType {
	Rust,
	CMake,
	Node,
	Python,
	Zig,
	Unknown
};

constexpr std::string_view project_type_to_string(ProjectType type) {
	switch (type) {
		case ProjectType::Rust:
			return "Rust";

		case ProjectType::CMake:
			return "CMake/C++";

		case ProjectType::Node:
			return "Node";

		case ProjectType::Python:
			return "Python";

		case ProjectType::Zig:
			return "Zig";

		case ProjectType::Unknown:
			return "Unknown";
	}

	return "No type";
}

constexpr std::optional<std::string_view> build_command_for(ProjectType type) {
	switch (type) {
	case ProjectType::Rust:
		return "cargo build";

	case ProjectType::CMake:
		return "cmake -S . -B build -G Ninja && cmake --build build";

	case ProjectType::Node:
		return "npm run build";

	case ProjectType::Zig:
		return "zig build";

	case ProjectType::Python:
	case ProjectType::Unknown:
		return std::nullopt;
	}

	return std::nullopt;
}

struct ProjectContent
{
	std::filesystem::path path;
	ProjectType type;
	std::optional<std::string> custom_build_command = std::nullopt;
	std::string display();
};
