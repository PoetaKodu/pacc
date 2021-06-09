#include PACC_PCH

#include <Pacc/Helpers/Json.hpp>
#include <Pacc/Helpers/Exceptions.hpp>

//////////////////////////////////////////////////
std::string JsonView::stringFieldOr(std::string_view subfieldName_, std::string_view alt_) const
{
	auto it = root.find(subfieldName_);
	if (it != root.end())
	{
		if (it->type() == json::value_t::string)
			return it->get<std::string>();
	}

	return std::string(alt_);
}


//////////////////////////////////////////////////
void JsonView::expect(std::string_view name, json::value_t type) const
{
	using namespace fmt;

	constexpr std::string_view WrongTypeMsg =
		"field \"{}\" expected to be of type \"{}\", but \"{}\" given instead";
	constexpr std::string_view NoFieldMsg =
		"field \"{}\" expected to be of type \"{}\" does not exist";

	auto it = root.find(name);

	if (it != root.end())
	{
		if (it->type() != type)
			throw PaccException(WrongTypeMsg, name, jsonTypeName(type), it->type_name());
	}
	else
		throw PaccException(NoFieldMsg, name, jsonTypeName(type));
}

//////////////////////////////////////////////////
void JsonView::requireType(std::string_view name, json::value_t type) const
{
	using namespace fmt;

	constexpr std::string_view WrongTypeMsg =
		"field \"{}\" expected to be of type \"{}\", but \"{}\" given instead";

	if (root.type() != type)
		throw PaccException(WrongTypeMsg, name, jsonTypeName(type), root.type_name());
}

//////////////////////////////////////////////////
constexpr std::string_view jsonTypeName(json::value_t type)
{
	switch(type)
	{
		case json::value_t::null:
			return "null";
		case json::value_t::object:
			return "object";
		case json::value_t::array:
			return "array";
		case json::value_t::string:
			return "string";
		case json::value_t::boolean:
			return "boolean";
		case json::value_t::binary:
			return "binary";
		case json::value_t::discarded:
			return "discarded";
		default:
			return "number";
	}
}