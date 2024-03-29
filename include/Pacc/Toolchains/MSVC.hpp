#pragma once

#include <Pacc/Toolchains/Toolchain.hpp>
struct MSVCToolchain : Toolchain
{
	virtual Type type() const
	{
		return MSVC;
	}

	enum class LineVersion
	{
		VS2022 	= 2022,
		VS2019 	= 2019,
		VS2017 	= 2017,
		VS2015 	= 2015,
		VS2013 	= 2013,
		Unknown = 0
	};
	LineVersion lineVersion;

	// say something polish
	virtual Opt<int> run(struct Package const& pkg_, BuildSettings settings_ = {}, int verbosityLevel_ = 0) override;

	virtual void serialize(json& out_) const override;

	virtual bool deserialize(json const& in_) override;

	virtual String premakeToolchainType() const override;

	static Vec<MSVCToolchain> detect();

private:

	/// <summary>
	/// 	Visual Studio (and Premake5) has a special case, where
	/// 	x86 platform for native C++ projects has to be named "Win32", instead of x86.
	/// 	Link: https://github.com/premake/premake-core/blob/65deb619f8d5579487def157d7c369e5c6d18864/modules/vstudio/vstudio.lua#L274
	/// </summary>
	static String handleWin32SpecialCase(String const& platformName_);
	static LineVersion parseLineVersion(String const& lvStr_);
};
