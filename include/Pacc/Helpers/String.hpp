#pragma once

#include PACC_PCH


/////////////////////////////////////////
std::string toLower(std::string_view str_);


using StringPair = std::pair<std::string, std::string>;

///////////////////////////////////////////////////
StringPair splitBy(std::string_view s, char c);

/////////////////////////////////////////////
bool startsWith(std::string_view str_, std::string_view prefixTest_);

/////////////////////////////////////////////
bool parseArgSwitch(std::string_view arg_, std::string_view switch_, std::string &value_);

/////////////////////////////////////////////////
bool compareIgnoreCase(std::string_view l, std::string_view r);

/////////////////////////////////////////////////
template <typename T>
std::optional<T> convertTo(std::string const& str_) = delete;

/////////////////////////////////////////////////
template <typename T>
T convertToOr(std::string const& str_, T val={})
{
	return convertTo<T>(str_).value_or();
}


/////////////////////////////////////////////////
// Specialization:
/////////////////////////////////////////////////

/////////////////////////////////////////////////
template <>
inline std::optional<int> convertTo(std::string const& str_)
{
	try 		{ return std::stoi(str_); }
	catch(...) 	{ return std::nullopt; }
}



