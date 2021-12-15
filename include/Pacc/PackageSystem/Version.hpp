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

	/// <summary>Determines whether two Version objects are exactly the same</summary>
	/// <param name="rhs_">The other Version object.</param>
	bool operator==(Version const& rhs_) const;

	std::strong_ordering operator<=>(Version const& rhs_) const
	{
		return std::tie(major, minor, patch) <=> std::tie(rhs_.major, rhs_.minor, rhs_.patch);
	}

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
	} type = Any;

	VersionRequirement() = default;

	VersionRequirement(Type type_, int major_, int minor_, int patch_)
		:
		version{ major_, minor_, patch_ },
		type(type_)
	{

	}

	VersionRequirement(std::string const str_)
	{
		*this = fromString(str_);
	}

	std::string toString() const;

	bool test(Version const& version_) const;

	static VersionRequirement fromString(std::string const& str_);
};
// A shorthand for VersionRequirement
using VersionReq = VersionRequirement;
