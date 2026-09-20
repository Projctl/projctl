#pragma once

#include <filesystem>
#include <string>

struct ProjectContent
{
	std::filesystem::path path;
	std::string display();
};
