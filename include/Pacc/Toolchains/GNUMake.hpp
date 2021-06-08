#pragma once

#include <Pacc/Toolchains/Toolchain.hpp>

struct GNUMakeToolchain : Toolchain
{
	std::string cppCompilerName	= "g++";
	std::string cCompilerName 	= "gcc";

	virtual Type type() const { return GNUMake; }

	virtual bool isEqual(Toolchain const& other_) const override;

	virtual void serialize(json& out_) const override;

	virtual bool deserialize(json const& in_) override;

	// TODO: Choose between "gmake" and "gmake2"
	virtual std::string premakeToolchainType() const { return "gmake2"; }

	virtual std::optional<int> run(Package const & pkg_, BuildSettings settings_ = {}, int verbosityLevel_ = 0) override;

	static std::vector<GNUMakeToolchain> detect();
};