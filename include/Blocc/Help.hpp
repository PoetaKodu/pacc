#pragma once

#include <string_view>
#include <tuple>

namespace help
{

using ActionInfo = std::pair< std::string_view, std::string_view >;

constexpr ActionInfo actions[] = {
	{ "init", 		"creates a local package inside specified folder (default = current)" },
	{ "build", 		"builds current package" },
	{ "link", 		"links specified package to user's environment" },
	{ "unlink", 	"unlinks specified package from user's environment" },
	{ "run", 		"runs packages's startup project" },
	{ "install",	"installs package artifacts" }
};

}