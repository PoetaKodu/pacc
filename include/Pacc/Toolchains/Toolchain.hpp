#pragma once

#include PACC_PCH

#include <Pacc/Helpers/Json.hpp>

#include <Pacc/Helpers/Exceptions.hpp>

// TODO: move this to other file to avoid circular include
struct BuildSettings
{
	std::string configName 		= "Debug";
	std::string platformName 	= "x64";
};

struct Toolchain
{
	std::string 	prettyName;
	std::string 	version;
	
	fs::path 		mainPath;

	enum Type
	{
		MSVC,
		GNUMake,
		Unknown
	};

	virtual ~Toolchain() = default;

	virtual Type type() const;

	virtual std::optional<int> run(struct Package const & pkg_, BuildSettings settings_ = {}, int verbosityLevel_ = 0);

	static std::string_view typeName(Type type_);

	virtual std::string premakeToolchainType() const;


	virtual bool generateProjectFiles();


	virtual bool isEqual(Toolchain const& other_) const;

	virtual void serialize(json& out_) const;

	virtual bool deserialize(json const& in_);

};
