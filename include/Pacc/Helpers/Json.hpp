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

	std::string stringFieldOr(std::string_view subfieldName_, std::string_view alt_) const;
	void expect(std::string_view name, json::value_t type) const;
	void requireType(std::string_view name, json::value_t type) const;
};

constexpr std::string_view jsonTypeName(json::value_t type);