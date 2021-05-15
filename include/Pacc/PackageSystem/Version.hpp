#pragma once

#include PACC_PCH

#include <Pacc/Helpers/String.hpp>
#include <Pacc/Helpers/Exceptions.hpp>

/// <summary>
/// 	A version compatible with semantic versioning:
///		https://semver.org/
/// </summary>
struct Version
{
	constexpr static std::string_view FieldNames[3] = { "major", "minor", "patch" };

	int major = 0;
	int minor = 0;
	int patch = 0;

	std::string toString() const;

	static Version fromString(std::string const& str_);
};

/// <summary>
/// 	Version requirement used for dependency management.
/// </summary>
struct VersionRequirement
{
	Version version;

	enum Type
	{
		Exact,		/// Major, Minor and Patch must be exactly the same
		SameMinor,	/// Major and Minor must be exactly the same
		SameMajor,	/// Major must be exactly the same
		Any			/// Can be any version
	} type = Exact;

	VersionRequirement() = default;
	
	VersionRequirement(int major_, int minor_, int patch_, Type type_ = Exact)
		:
		version{ major_, minor_, patch_ },
		type(type_)
	{

	}

	std::string toString() const;

	static VersionRequirement fromString(std::string const& str_);
};