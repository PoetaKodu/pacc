#pragma once

#include <Pacc/Toolchains/General.hpp>

struct MSVCToolchain : Toolchain
{
	static std::vector<MSVCToolchain> detect();
};