#include PACC_PCH

#include <Pacc/Helpers/String.hpp>

/////////////////////////////////////////
String toLower(StringView str_)
{
	String lower(str_);
	// TODO: this won't work on unicode characters.
	// Use ICU later.
	rg::transform(lower, lower.begin(),
			[](unsigned char c) {
				return std::tolower(c);
			}
		);

	return lower;
}


///////////////////////////////////////////////////
StringPair splitBy(StringView str_, char delim_, bool leftAsFallback_)
{
	auto pos = str_.find(delim_);
	if (pos != StringView::npos)
		return StringPair{ str_.substr(0, pos), str_.substr(pos + 1) };
	else
	{
		if (leftAsFallback_)
			return StringPair{ str_, "" };
		else
			return StringPair{ "", str_ };
	}
}

/////////////////////////////////////////////
bool startsWith(StringView str_, StringView prefixTest_)
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
bool parseArgSwitch(StringView arg_, StringView switch_, String &value_)
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
bool compareIgnoreCase(StringView l, StringView r)
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
String replaceAll(StringView source_, StringView from_, StringView to_)
{
    String newString;
    newString.reserve(source_.length());  // avoids a few memory allocations

    String::size_type lastPos = 0;
    String::size_type findPos;

    while(String::npos != (findPos = source_.find(from_, lastPos)))
    {
        newString.append(source_, lastPos, findPos - lastPos);
        newString += to_;
        lastPos = findPos + from_.length();
    }

    // Care for the rest after last occurrence
    newString += source_.substr(lastPos);

    return newString;
}
