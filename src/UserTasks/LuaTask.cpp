#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>
#include <Pacc/Helpers/String.hpp>
#include <Pacc/UserTasks/LuaTask.hpp>
#include <Pacc/PackageSystem/Package.hpp>

auto LuaTaskAction::load(json const& object_) const -> UPtr<BasicEventHandlerDesc>
{
	auto desc = std::make_unique<LuaEventHandlerDesc>();

	desc->action = "lua";
	desc->moduleName = object_.value<String>("module", "");
	desc->functionName = object_.value<String>("function", "");

	return desc;
}

auto LuaTaskAction::load(String const& abbreviatedAction_) const -> UPtr<BasicEventHandlerDesc>
{
	constexpr auto SyntaxHelp = StringView(
			"Use the following syntax:\n"
			"        module.lua:functionName\n"
			"    The module part should be enclosed in quotes if the path contains the ':' character.\n"
		);

	auto desc = std::make_unique<LuaEventHandlerDesc>();

	desc->action = "lua";

	auto moduleName = String();
	auto functionName = String();

	if (abbreviatedAction_.starts_with('"'))
	{
		// Syntax:
		// "module":function

		auto endQuote = abbreviatedAction_.find('"', 1);
		if (endQuote == String::npos)
		{
			throw PaccException("Invalid Lua task action: {}", abbreviatedAction_)
				.withHelp(SyntaxHelp);
		}

		desc->moduleName	= abbreviatedAction_.substr(1, endQuote - 1);
		desc->functionName	= abbreviatedAction_.substr(endQuote + 2);
	}
	else
	{
		// Syntax:
		// module:function

		auto res = splitBy(abbreviatedAction_, ':', true);
		if (res.second.empty())
		{
			throw PaccException("Invalid Lua task action: {}", abbreviatedAction_)
				.withHelp(SyntaxHelp);
		}

		desc->moduleName	= std::move(res.first);
		desc->functionName	= std::move(res.second);
	}

	return desc;
}

auto LuaTaskAction::execute(Package& context_, BasicEventHandlerDesc const& desc_) const -> void
{
	auto const& desc = dynamic_cast<LuaEventHandlerDesc const&>(desc_);

	auto& script = useApp().requireLuaScript(context_, desc.moduleName);

	auto luaFn = script[desc.functionName];
	if (luaFn.get_type() != sol::type::function) {
		throw PaccException("Function \"{}\" not defined within the specified module.", desc.functionName);
	}

	auto result = luaFn(std::ref(context_));

	if (!result.valid()) {
		throw PaccException("Could not execute event \"{}\"!\nDetails: {}", desc.functionName, result.get<sol::error>().what());
	}
}
