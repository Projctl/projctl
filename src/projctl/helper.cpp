#include "projctl.hpp"

ProjectType detect_project_type(const std::filesystem::path& path) {
	if (std::filesystem::exists(path / "Cargo.toml")) return ProjectType::Rust;
	if (std::filesystem::exists(path / "CMakeLists.txt")) return ProjectType::CMake;
	if (std::filesystem::exists(path / "package.json")) return ProjectType::Node;
	if (std::filesystem::exists(path / "pyproject.toml")) return ProjectType::Python;
	return ProjectType::Unknown;
}

std::string trim(std::string_view text) {
	std::size_t first = text.find_first_not_of(" \t\r\n");

	if (first == std::string_view::npos) return "";

	std::size_t last = text.find_last_not_of(" \t\r\n");

	return std::string(text.substr(first, last - first + 1));
}
std::string truncate(std::string_view text, std::size_t max_length) {
	if (text.size() <= max_length)
		return std::string(text);

	if (max_length <= 5)
		return std::string(text.substr(0, max_length));

	return std::string(text.substr(0, max_length - 5)) + "...";
}
