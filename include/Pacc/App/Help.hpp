#pragma once

#include PACC_PCH

namespace help
{

using ActionInfo = std::pair< std::string_view, std::string_view >;

constexpr ActionInfo actions[] = {
	{ "init", 			"creates a local package inside specified folder (default = current)" },
	{ "generate", 		"generates Premake5 files for current package" },
	{ "build", 			"builds current package" },
	{ "link", 			"links specified package to user's environment" },
	{ "unlink", 		"unlinks specified package from user's environment" },
	{ "toolchains", 	"manages used toolchains (list, detect, configure, etc.)" },
	{ "run", 			"runs packages's startup project" },
	{ "log", 			"list latest build logs or print last log's content (--last)" },
	{ "list-versions",	"lists available versions of remote package" },
	{ "version", 		"displays pacc version" },
	{ "help", 			"displays this help message" },
	{ "install",		"installs package artifacts" },
	{ "uninstall",		"uninstalls package artifacts" }
};

constexpr std::string_view DependencySyntax =
	"    - \"RepoName\" for package from official repository (https://github.com/pacc-repo)\n"
	"    - \"github:UserName/RepoName\" for package from GitHub repository\n"
	"    - \"gitlab:UserName/RepoName\" for package from GitLab repository";

}