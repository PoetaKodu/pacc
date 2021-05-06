#pragma once

#include PACC_PCH

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

	void expect(std::string_view name, json::value_t type);
	void requireType(std::string_view name, json::value_t type);
};

constexpr std::string_view jsonTypeName(json::value_t type);