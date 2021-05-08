#pragma once

#include PACC_PCH

#include <Pacc/Helpers/Json.hpp>

#include <Pacc/Helpers/Exceptions.hpp>

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

	static std::string_view typeName(Type type_)
	{
		switch(type_)
		{
			case MSVC: 		return "msvc";
			case GNUMake: 	return "gnumake";
			default: 		return "unknown";
		}
	}

	virtual Type type() const { return Unknown; }

	virtual bool isEqual(Toolchain const& other_) const
	{
		if (other_.type() != this->type())
			return false;

		return (
				prettyName 	== other_.prettyName &&
				version 	== other_.version &&
				mainPath 	== other_.mainPath
			);
	}

	virtual void serialize(json& out_) const
	{
		out_["prettyName"] 	= prettyName;
		out_["version"] 	= version;
		out_["mainPath"] 	= mainPath.u8string();
		out_["type"] 		= typeName(this->type());
	}

	virtual bool deserialize(json const& in_)
	{
		using JV = JsonView;
		JsonView view{in_};

		prettyName 	= view.stringFieldOr("prettyName", 	"Unknown");
		version 	= view.stringFieldOr("version", 	"?.?.?");
		mainPath 	= view.stringFieldOr("mainPath", 	"");

		if (mainPath == "")
			return false;

		return true;
	}

	virtual ~Toolchain() = default;
};
