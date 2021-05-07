#pragma once

#include PACC_PCH

namespace help
{

using ActionInfo = std::pair< std::string_view, std::string_view >;

constexpr ActionInfo actions[] = {
	{ "init", 		"creates a local package inside specified folder (default = current)" },
	{ "generate", 	"generates Premake5 files for current package" },
	{ "build", 		"builds current package" },
	{ "link", 		"links specified package to user's environment" },
	{ "unlink", 	"unlinks specified package from user's environment" },
	{ "toolchains", "manages used toolchains (list, detect, configure, etc.)" },
	{ "run", 		"runs packages's startup project" },
	{ "install",	"installs package artifacts" }
};

}