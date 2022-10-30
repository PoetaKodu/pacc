#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/HelperTypes.hpp>

template <typename T>
struct BasicJsonView
{
	T & root;

	// Constructor
	BasicJsonView(T & root_)
		: root{ root_ }
	{
	}
};

struct JsonView
	: BasicJsonView<json const>
{
	using BasicJsonView::BasicJsonView;

	String stringFieldOr(StringView subfieldName_, StringView alt_) const;
	void expect(StringView name, json::value_t type) const;
	void requireType(StringView name, json::value_t type) const;
};

constexpr StringView jsonTypeName(json::value_t type);
