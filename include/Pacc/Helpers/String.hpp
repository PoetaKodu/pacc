#pragma once

#include PACC_PCH


/////////////////////////////////////////
inline std::string to_lower(std::string_view str_)
{
	std::string lower(str_);
	// TODO: this won't work on unicode characters.
	// Use ICU later.
	std::transform(lower.begin(), lower.end(), lower.begin(),
			[](unsigned char c) {
				return std::tolower(c);
			}
		);

	return lower;
}

/////////////////////////////////////////////
inline bool startsWith(std::string_view str_, std::string_view prefixTest_)
{
	if (str_.length() < prefixTest_.length())
		return false;

	for(size_t i = 0; i < prefixTest_.length(); ++i)
	{
		if (str_[i] != prefixTest_[i])
			return false;
	}
	
	return true;
}

/////////////////////////////////////////////
inline bool parseArgSwitch(std::string_view arg_, std::string_view switch_, std::string &value_)
{
	// Syntax:
	// --arg=val
	// --arg=<at least one character>
	if (arg_.length() <= switch_.length() + 1)
		return false;

	if (!startsWith(arg_, switch_))
		return false;
	
	if (arg_[switch_.length()] != '=')
		return false;

	value_ = arg_.substr(switch_.length()+1);
	return true;
}