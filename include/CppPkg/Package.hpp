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
	VecOfStrAcc 	includeFolders;
	VecOfStrAcc 	linkerFolders;
	VecOfStrAcc 	linkedLibraries;
};

struct Project : TargetBase
{
	
};

struct Package : TargetBase
{
	std::vector<Project> projects;
};

