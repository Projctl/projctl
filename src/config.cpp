#include "config.hpp"

#include <fstream>

namespace {

std::string trim(std::string_view text) {
	std::size_t first = text.find_first_not_of(" \t\r\n");

	if (first == std::string_view::npos) return "";

	std::size_t last = text.find_last_not_of(" \t\r\n");

	return std::string(text.substr(first, last - first + 1));
}

std::vector<IssueField> parse_issue_fields(std::string_view value) {
	std::vector<IssueField> fields;

	std::size_t start = 0;

	while (start < value.size()) {
		std::size_t comma = value.find(',', start);

		std::string field = trim(value.substr(start, comma == std::string_view::npos ? value.size() - start : comma - start));

		if (field == "number")
			fields.push_back(IssueField::Number);

		else if (field == "title")
			fields.push_back(IssueField::Title);

		else if (field == "state")
			fields.push_back(IssueField::State);

		else if (field == "author")
			fields.push_back(IssueField::Author);

		else if (field == "labels")
			fields.push_back(IssueField::Labels);

		else if (field == "parent")
			fields.push_back(IssueField::Parent);

		if (comma == std::string_view::npos)
			break;

		start = comma + 1;
	}

	return fields;
}

std::optional<Color> color_from_string(std::string_view value) {
	if (value == "default") return Color::Default;
	if (value == "black") return Color::Black;
	if (value == "red") return Color::Red;
	if (value == "green") return Color::Green;
	if (value == "yellow") return Color::Yellow;
	if (value == "blue") return Color::Blue;
	if (value == "magenta") return Color::Magenta;
	if (value == "cyan") return Color::Cyan;
	if (value == "white") return Color::White;

	return std::nullopt;
}

}

max_width			

void Config::load(const std::filesystem::path& path) {
	std::ifstream config(path);

	if (!config) return;

	std::string section;
	std::string line;
	while (std::getline(config, line)) {
		line = trim(line);
		if (line.empty()) continue;
		if (line.front() == '#' || line.front() == ';') continue;

		if (line.front() == '[' && line.back() == ']') {
			section = trim (std::string_view(line).substr(1, line.size() - 2));
			continue;
		}

		std::size_t separator = line.find('=');
		if (separator == std::string::npos) continue;

		std::string key = trim(std::string_view(line.substr(0, separator)));
		std::string value = trim(std::string_view(line.substr(separator + 1)));

		if (section == "display") {
			if (key == "name_width") name_width = std::stoul(value);
			else if (key == "path_width") path_width = std::stoul(value);
			else if (key == "remote_width") remote_width = std::stoul(value);
			else if (key == "branch_width") branch_width = std::stoul(value);
		}

		else if (section == "issues") {
			if (key == "number_width") issue_number_width = std::stoul(value);
			else if (key == "title_width") issue_title_width = std::stoul(value);
			else if (key == "author_width") issue_author_width = std::stoul(value);
			else if (key == "state_width") issue_state_width = std::stoul(value);
			else if (key == "labels_width") issue_labels_width = std::stoul(value);
			else if (key == "columns") {
				std::vector<IssueField> parsed = parse_issue_fields(value);
				if (!parsed.empty()) issue_columns = std::move(parsed);
			}
		}

		else if (section == "colors") {
			if (key == "enabled") {
				colors_enabled = value == "true";
			} else {
				std::optional<Color> color = color_from_string(value);
				if (color) {
					if (key == "issue_number")						issue_number_color			= *color;
					else if (key == "header_color")					header_color				= *color;
					else if (key == "issue_title_color")			issue_title_color			= *color;
					else if (key == "issue_open_color")				issue_open_color			= *color;
					else if (key == "issue_closed_color")			issue_closed_color			= *color;
					else if (key == "issue_author_color")			issue_author_color			= *color;
					else if (key == "issue_labels_color")			issue_labels_color			= *color;
					else if (key == "issue_parent_color")			issue_parent_color			= *color;
					else if (key == "error_color")					error_color					= *color;
					else if (key == "warning_color")				warning_color				= *color;
					else if (key == "project_name_color")			project_name_color			= *color;
					else if (key == "project_path_color")			project_path_color			= *color;
					else if (key == "project_branch_color")			project_branch_color		= *color;
					else if (key == "project_remote_color")			project_remote_color		= *color;
					else if (key == "project_type_color")			project_type_color			= *color;
					else if (key == "project_short_remote_color")	project_short_remote_color	= *color;
					else if (key == "tick_color")					tick_color					= *color;
					else if (key == "cross_color")					cross_color					= *color;
				}
			}

		}
	}
}
