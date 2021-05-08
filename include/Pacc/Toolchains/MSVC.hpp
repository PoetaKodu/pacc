#pragma once

#include <Pacc/Toolchains/Toolchain.hpp>

struct MSVCToolchain : Toolchain
{
	virtual Type type() const { return MSVC; }

	static std::vector<MSVCToolchain> detect();
};