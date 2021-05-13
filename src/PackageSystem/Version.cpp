#include PACC_PCH


#include <Pacc/PackageSystem/Version.hpp>

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
	}

	return result;
}

////////////////////////////////////////
std::string Version::toString() const
{
	return fmt::format(FMT_COMPILE("{}.{}.{}"), major, minor, patch);
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

	result.version = Version::fromString(result.type == Exact ? str_ : str_.substr(1));

	return result;
}


////////////////////////////////////////
std::string VersionRequirement::toString() const
{
	constexpr std::string_view ReqChar[3] = { "", "~", "^" };

	return std::string(ReqChar[static_cast<int>(type)]) + version.toString();
}
