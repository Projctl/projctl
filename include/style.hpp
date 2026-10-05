#pragma once

#include <string_view>

namespace Style {

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

}
