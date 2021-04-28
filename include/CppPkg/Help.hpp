#pragma once

#include <string_view>
#include <tuple>

namespace help
{

using ActionInfo = std::pair< std::string_view, std::string_view >;

constexpr ActionInfo actions[] = {
	{ "init", 		"creates a local package inside specified folder (default = current)" },
	{ "build", 		"builds current package" },
	{ "run", 		"runs packages's startup project" },
	{ "install",	"installs package artifacts" }
};

}