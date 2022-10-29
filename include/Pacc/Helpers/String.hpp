#pragma once

#include PACC_PCH

#include <Pacc/Helpers/HelperTypes.hpp>


/////////////////////////////////////////
String toLower(StringView str_);


using StringPair = std::pair<String, String>;

///////////////////////////////////////////////////
StringPair splitBy(StringView str_, char delim_, bool leftAsFallback_ = true);

/////////////////////////////////////////////
bool startsWith(StringView str_, StringView prefixTest_);

/////////////////////////////////////////////
bool parseArgSwitch(StringView arg_, StringView switch_, String &value_);

/////////////////////////////////////////////////
bool compareIgnoreCase(StringView l, StringView r);

/////////////////////////////////////////////////
String replaceAll(StringView source_, StringView from_, StringView to_);

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
	bool operator() (StringView const& s1_, StringView const& s2_) const
	{
		return rg::lexicographical_compare(s1_, s2_, CompareIgnoreCase{});
	}
};

/////////////////////////////////////////////////
template <typename T>
Opt<T> convertTo(String const& str_) = delete;

/////////////////////////////////////////////////
template <typename T>
T convertToOr(String const& str_, T val={})
{
	return convertTo<T>(str_).value_or();
}

struct StringTokenIterator
{
	/////////////////////////////////////
	StringTokenIterator(StringView view_, StringView tokens_)
		: view(view_), tokens(tokens_)
	{
	}

	/////////////////////////////////
	StringTokenIterator& operator++()
	{
		if (currentPos.has_value() &&
			*currentPos == StringView::npos)
			wasInvalid = true;
		else
		{
			size_t startPos = currentPos.has_value() ? (*currentPos + 1) : 0;
			currentPos = view.find_first_of(tokens, startPos);
		}

		return *this;
	}

	/////////////////////////////////
	StringView operator*() const
	{
		size_t startPos = currentPos.has_value() ? (*currentPos + 1) : 0;
		size_t nextToken = view.find_first_of(tokens, startPos);
		if (nextToken == StringView::npos)
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
		auto result = StringTokenIterator{ view, tokens };
		result.currentPos = StringView::npos;
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

	StringView 		view;
	StringView 		tokens;
	Opt<size_t> 	currentPos 	= std::nullopt;
	bool 					wasInvalid 	= false;
};


/////////////////////////////////////////////////
// Specialization:
/////////////////////////////////////////////////

/////////////////////////////////////////////////
template <>
inline Opt<int> convertTo(String const& str_)
{
	try 		{ return std::stoi(str_); }
	catch(...) 	{ return std::nullopt; }
}
