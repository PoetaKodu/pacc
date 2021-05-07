#pragma once

#include <Pacc/Toolchains/General.hpp>

struct GNUMakeToolchain : Toolchain
{
	static std::vector<GNUMakeToolchain> detect();

private:
	static fs::path findMake();
};