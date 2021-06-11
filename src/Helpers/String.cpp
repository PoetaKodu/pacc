#include PACC_PCH

#include <Pacc/Helpers/String.hpp>

/////////////////////////////////////////
std::string toLower(std::string_view str_)
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


///////////////////////////////////////////////////
StringPair splitBy(std::string_view s, char c)
{
	auto pos = s.find(c);
	if (pos != std::string_view::npos)
		return StringPair{ s.substr(0, pos), s.substr(pos + 1) };
	else
		return StringPair{ s, std::string{} };
}

/////////////////////////////////////////////
bool startsWith(std::string_view str_, std::string_view prefixTest_)
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
bool parseArgSwitch(std::string_view arg_, std::string_view switch_, std::string &value_)
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

/////////////////////////////////////////////////
bool compareIgnoreCase(std::string_view l, std::string_view r)
{
	if (l.length() != r.length()) return false;

	for(std::size_t i = 0; i < l.size(); i++)
	{
		if ( std::tolower(int(l[i])) != std::tolower(int(r[i])) )
			return false;
	}

	return true;
}

/////////////////////////////////////////////////
std::string replaceAll(std::string_view source_, std::string_view from_, std::string_view to_)
{
    std::string newString;
    newString.reserve(source_.length());  // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while(std::string::npos != (findPos = source_.find(from_, lastPos)))
    {
        newString.append(source_, lastPos, findPos - lastPos);
        newString += to_;
        lastPos = findPos + from_.length();
    }

    // Care for the rest after last occurrence
    newString += source_.substr(lastPos);

    return newString;
}