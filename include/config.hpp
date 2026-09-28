#pragma once

#include "issue.hpp"

#include <vector>
#include <filesystem>

enum class Color {
	Default,
	Black,
	BaldBlack,
	Red,
	BaldRed,
	Green,
	BaldGreen,
	Yellow,
	BaldYellow,
	Blue,
	BaldBlue,
	Magenta,
	BaldMagenta,
	Cyan,
	BaldCyan,
	White,
	BaldWhite,
};

constexpr std::string_view color_code(Color color) {
	switch (color) {
		case Color::Default:	return "\033[39m";
		case Color::Black:		return "\033[30m";
		case Color::BaldBlack:	return "\033[1;30m";
		case Color::Red:		return "\033[31m";
		case Color::BaldRed:	return "\033[1;31m";
		case Color::Green:		return "\033[32m";
		case Color::BaldGreen:  return "\033[1;32m";
		case Color::Yellow:		return "\033[33m";
		case Color::BaldYellow:	return "\033[1;33m";
		case Color::Blue:		return "\033[34m";
		case Color::BaldBlue:	return "\033[1;34m";
		case Color::Magenta:	return "\033[35m";
		case Color::BaldMagenta:return "\033[1;35m";
		case Color::Cyan:		return "\033[36m";
		case Color::BaldCyan:	return "\033[1;36m";
		case Color::White:		return "\033[37m";
		case Color::BaldWhite:	return "\033[1;37m";
	}

	return "\033[39m";
}

constexpr std::string_view RESET = "\033[0m";
constexpr std::string_view TICK = "✓ ";
constexpr std::string_view CROSS = "✗ ";

struct Config {
	std::size_t max_width			= 300;
	std::size_t name_width			= 20;
	std::size_t path_width			= 50;
	std::size_t remote_width		= 50;
	std::size_t branch_width		= 40;
	std::size_t issue_number_width	= 8;
	std::size_t issue_title_width	= 40;
	std::size_t issue_author_width	= 20;
	std::size_t issue_state_width	= 12;
	std::size_t issue_labels_width	= 30;

	std::vector<IssueField> issue_columns { IssueField::Number, IssueField::Title, IssueField::State, IssueField::Author, IssueField::Labels };

	bool colors_enabled = true;

	Color header_color				= Color::Cyan;
	Color project_name_color		= Color::BaldYellow;
	Color project_path_color		= Color::Magenta;
	Color project_branch_color		= Color::Cyan;
	Color project_remote_color		= Color::Blue;
	Color project_type_color		= Color::BaldMagenta;
	Color project_short_remote_color= Color::BaldBlue;
	Color tick_color				= Color::BaldGreen;
	Color cross_color				= Color::BaldRed;
	Color issue_number_color		= Color::BaldMagenta;
	Color issue_title_color			= Color::White;
	Color issue_open_color			= Color::BaldGreen;
	Color issue_closed_color		= Color::BaldRed;
	Color issue_author_color		= Color::Blue;
	Color issue_labels_color		= Color::Magenta;
	Color issue_parent_color		= Color::Cyan;
	Color error_color				= Color::BaldRed;
	Color warning_color				= Color::Yellow;

	void load(const std::filesystem::path& path);
};
