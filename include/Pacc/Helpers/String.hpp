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

struct IgnoreCaseLess
{
	// case-independent (ci) compare_less binary function
	struct CompareIgnoreCase
	{
		bool operator() (unsigned char c1_, unsigned char c2_) const
		{
			return tolower(c1_) < tolower(c2_); 
		}
	};
	bool operator() (std::string_view const& s1_, std::string_view const& s2_) const
	{
		return std::lexicographical_compare(s1_.begin(), s1_.end(), s2_.begin(), s2_.end(), CompareIgnoreCase{});
	}
};

/////////////////////////////////////////////////
template <typename T>
std::optional<T> convertTo(std::string const& str_) = delete;

/////////////////////////////////////////////////
template <typename T>
T convertToOr(std::string const& str_, T val={})
{
	return convertTo<T>(str_).value_or();
}

struct StringTokenIterator
{
	/////////////////////////////////////
	StringTokenIterator(std::string_view view_, std::string_view tokens_)
		: view(view_), tokens(tokens_)
	{
	}

	/////////////////////////////////
	StringTokenIterator& operator++()
	{
		if (currentPos.has_value() &&
			*currentPos == std::string_view::npos)
			wasInvalid = true;
		else
		{
			size_t startPos = currentPos.has_value() ? (*currentPos + 1) : 0;
			currentPos = view.find_first_of(tokens, startPos);
		}

		return *this;
	}

	/////////////////////////////////
	std::string_view operator*() const
	{
		size_t startPos = currentPos.has_value() ? (*currentPos + 1) : 0;
		size_t nextToken = view.find_first_of(tokens, startPos);
		if (nextToken == std::string_view::npos)
			return view.substr(startPos);

		return view.substr(startPos, nextToken - startPos);		
	}

	/////////////////////////////////
	StringTokenIterator begin() const
	{
		return { view, tokens };
	}

	/////////////////////////////////
	StringTokenIterator end() const
	{
		StringTokenIterator result{ view, tokens };
		result.currentPos = std::string_view::npos;
		result.wasInvalid = true;
		return result;
	}

	bool operator!=(StringTokenIterator const& rhs) const
	{
		return
			view 		!= rhs.view ||
			tokens 		!= rhs.tokens ||
			currentPos 	!= rhs.currentPos ||
			wasInvalid 	!= rhs.wasInvalid;
	}

	std::string_view 		view;
	std::string_view 		tokens;
	std::optional<size_t> 	currentPos 	= std::nullopt;
	bool 					wasInvalid 	= false;
};


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
