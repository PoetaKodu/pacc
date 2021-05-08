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
		VS2019 	= 2019,
		VS2017 	= 2017,
		VS2015 	= 2015,
		VS2013 	= 2013,
		Unknown = 0
	};
	LineVersion lineVersion;


	virtual std::optional<int> run(struct Package const& pkg_) override;

	virtual void serialize(json& out_) const override;

	virtual bool deserialize(json const& in_) override;

	virtual std::string premakeToolchainType() const override;

	static std::vector<MSVCToolchain> detect();

private:
	static LineVersion parseLineVersion(std::string const& lvStr_);
};