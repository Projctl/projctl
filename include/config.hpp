#pragma once

#include "issue.hpp"

#include <vector>
#include <filesystem>
#include "style.hpp"


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

	Style::Color header_color				= Style::Color::Cyan;
	Style::Color project_name_color			= Style::Color::BaldYellow;
	Style::Color project_path_color			= Style::Color::Magenta;
	Style::Color project_branch_color		= Style::Color::Cyan;
	Style::Color project_remote_color		= Style::Color::Blue;
	Style::Color project_type_color			= Style::Color::BaldMagenta;
	Style::Color project_short_remote_color = Style::Color::BaldBlue;
	Style::Color tick_color					= Style::Color::BaldGreen;
	Style::Color cross_color				= Style::Color::BaldRed;
	Style::Color issue_number_color			= Style::Color::BaldMagenta;
	Style::Color issue_title_color			= Style::Color::White;
	Style::Color issue_open_color			= Style::Color::BaldGreen;
	Style::Color issue_closed_color			= Style::Color::BaldRed;
	Style::Color issue_author_color			= Style::Color::Blue;
	Style::Color issue_labels_color			= Style::Color::Magenta;
	Style::Color issue_parent_color			= Style::Color::Cyan;
	Style::Color error_color				= Style::Color::BaldRed;
	Style::Color warning_color				= Style::Color::Yellow;

	std::string editor						= "nvim";

	void load(const std::filesystem::path& path);
};
