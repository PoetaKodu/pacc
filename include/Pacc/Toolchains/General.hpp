#pragma once

#include PACC_PCH

struct Toolchain
{
	std::string 	prettyName;
	std::string 	version;
	
	fs::path 		mainPath;
};