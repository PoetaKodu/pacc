#include PACC_PCH


#include <Pacc/PackageSystem/Version.hpp>

////////////////////////////////////////
bool Version::operator==(Version const& rhs_) const
{
	return (major == rhs_.major &&
			minor == rhs_.minor &&
			patch == rhs_.patch);
}

////////////////////////////////////////
Version Version::fromString(std::string const& str_)
{
	if (str_.empty())
		throw PaccException("could not parse version (empty string)");

	Version result;
	int* fields[3] = { &result.major, &result.minor, &result.patch };

	size_t searchStart = 0;
	for (int i = 0; i < 3; ++i)
	{
		size_t dotPos = str_.find('.', searchStart);
		bool dotFound = (dotPos != std::string::npos);

		auto optField = convertTo<int>(
				dotFound ?
				str_.substr(searchStart, dotPos - searchStart) :
				str_.substr(searchStart)
			);

		if (!optField.has_value())
			throw PaccException("could not parse version (invalid {} field)", FieldNames[i]);

		*(fields[i]) = optField.value();

		if (!dotFound)
			break;
		else
			searchStart = dotPos + 1;
	}

	return result;
}

////////////////////////////////////////
std::string Version::toString() const
{
	return fmt::format(FMT_COMPILE("{}.{}.{}"), major, minor, patch);
}

////////////////////////////////////////
bool VersionRequirement::test(Version const& version_) const
{
	switch(type)
	{
	case Exact: 	return (version == version_);
	case SameMinor: return (!(version_ < version) && version.major == version_.major && version.minor == version_.minor);
	case SameMajor: return (!(version_ < version) && version.major == version_.major);
	case Any: 		return true;
	default: 		return false;
	}
}

////////////////////////////////////////
VersionRequirement VersionRequirement::fromString(std::string const& str_)
{	
	if (str_.empty())
		throw PaccException("could not parse version requirement (empty string)");
	
	VersionRequirement result;

	if (str_[0] == '~')
		result.type = SameMinor;
	else if (str_[0] == '^')
		result.type = SameMajor;
	else if (str_[0] == '*')
	{
		result.type = Any;
		return result;
	}
	else
		result.type = Exact;

	result.version = Version::fromString(result.type == Exact ? str_ : str_.substr(1));

	return result;
}


////////////////////////////////////////
std::string VersionRequirement::toString() const
{
	constexpr std::string_view ReqChar[3] = { "", "~", "^" };

	return std::string(ReqChar[static_cast<int>(type)]) + version.toString();
}
