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