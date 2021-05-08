#pragma once

#include PACC_PCH

#include <Pacc/Toolchains/Toolchain.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

struct PaccConfig
{
	using VecOfTc = Vec< UPtr<Toolchain> >;

	VecOfTc 	detectedToolchains;
	size_t 		selectedToolchain;
	fs::path 	path;

	bool ensureValidToolchains(VecOfTc & current_);

	void updateToolchains(VecOfTc current_);

	bool validateDetectedToolchains(VecOfTc const& current_) const;

	static json serializeToolchains(VecOfTc const& tcs_);

	static PaccConfig loadOrCreate(fs::path const& jsonPath_);
	static PaccConfig load(fs::path const& jsonPath_);

private:
	void readDetectedToolchains(json const& input_);
};