#pragma once

#include <Pacc/Toolchains/Toolchain.hpp>

struct MSVCToolchain : Toolchain
{
	virtual Type type() const { return MSVC; }

	virtual std::optional<int> run(struct Package const& pkg_) override;

	// TODO: determine using `version`
	virtual std::string premakeToolchainType() const { return "vs2019"; }

	static std::vector<MSVCToolchain> detect();
};