#pragma once

#include <string>
#include <vector>
#include <optional>

enum class RequestField {
	Number,
	Title,
	State,
	Author,
	Labels,
	Created,
	Updated,
	Parent
};

struct Request {
	int number;
	std::string title;
	std::string state;
	std::string author;
	std::string created;
	std::string updated;
	std::vector<std::string> labels;
	std::optional<std::string> parent;
};
