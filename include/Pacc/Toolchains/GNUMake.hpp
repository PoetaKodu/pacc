#pragma once

#include <Pacc/Toolchains/Toolchain.hpp>

struct GNUMakeToolchain : Toolchain
{
	virtual Type type() const { return GNUMake; }

	static std::vector<GNUMakeToolchain> detect();

private:
	static fs::path findMake();
};