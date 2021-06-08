#pragma once

#include PACC_PCH

#include <Pacc/Toolchains/Toolchain.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

struct PaccConfig
{
	using VecOfTc = Vec< SPtr<Toolchain> >;

	VecOfTc		toolchains;
	VecOfTc 	detectedToolchains;
	size_t 		selectedToolchain;
	fs::path 	path;

	Toolchain* currentToolchain() const
	{
		if (selectedToolchain < toolchains.size())
			return toolchains[selectedToolchain].get();
		
		return nullptr;
	}

	bool ensureValidToolchains(VecOfTc & current_);

	void updateToolchains(VecOfTc current_);

	// Unchecked!
	void updateSelectedToolchain(int index_);

	bool validateDetectedToolchains(VecOfTc const& current_) const;

	static json serializeToolchains(VecOfTc const& tcs_);

	static PaccConfig loadOrCreate(fs::path const& jsonPath_);
	static PaccConfig load(fs::path const& jsonPath_);

private:

	VecOfTc readToolchains(json const& input_, std::string const &field_);
	void readSelectedToolchain(json const& input_);
};