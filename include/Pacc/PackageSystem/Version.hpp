#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/String.hpp>
#include <Pacc/Helpers/Exceptions.hpp>

/// <summary>
///		A version compatible with semantic versioning:
///		TODO: support metadata
///		https://semver.org/
/// </summary>
struct Version
{
	constexpr static StringView FieldNames[3] = { "major", "minor", "patch" };

	int major = 0;
	int minor = 0;
	int patch = 0;

	static Version fromString(StringView str_);
	String toString() const;

	auto operator<=>(Version const& rhs_) const = default;
};

/// <summary>
/// 	Version requirement used for dependency management.
/// </summary>
struct VersionRequirement
{
	Version version;

	enum Type
	{
		Exact,		/// Major, Minor and Patch must be exactly the same. Example: 1.2.3
		SameMinor,	/// Major and Minor must be exactly the same. Example: ~1.2.3
		SameMajor,	/// Major must be exactly the same. Example: ^1.2.3
		Any			/// Can be any version. Example: *
	} type = Any;

	VersionRequirement() = default;

	VersionRequirement(Type type_, int major_, int minor_, int patch_)
		:
		version{ major_, minor_, patch_ },
		type(type_)
	{

	}

	VersionRequirement(StringView str_)
	{
		*this = fromString(str_);
	}

	String toString() const;

	bool test(Version const& version_) const;

	static VersionRequirement fromString(StringView str_);
};

// A shorthand for VersionRequirement
using VersionReq = VersionRequirement;
