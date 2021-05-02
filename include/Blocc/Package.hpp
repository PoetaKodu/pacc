#pragma once

#include <string>
#include <vector>
#include <string_view>

using VecOfStr = std::vector< std::string >;

struct VecOfStrAcc
{
	VecOfStr public_;
	VecOfStr private_;
	VecOfStr interface_;
};

struct TargetBase
{
	std::string 	name;

	VecOfStr 		files;
	VecOfStrAcc 	defines;
	VecOfStrAcc 	includeFolders;
	VecOfStrAcc 	linkerFolders;
	VecOfStrAcc 	linkedLibraries;
};

struct Project : TargetBase
{
	std::string		type;
	std::string		language;
	std::string		cppStandard;
	std::string		cStandard;
};

struct Package : TargetBase
{
	std::vector<Project> projects;
};

