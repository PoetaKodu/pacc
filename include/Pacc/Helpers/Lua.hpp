#pragma once

#include <Pacc/PaccPCH.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

struct Package;

struct LuaScriptContext {
	Package const* package;
	Path scriptPath;
};

template <>
struct std::hash<LuaScriptContext> {
	auto operator()(LuaScriptContext const& ctx) const -> std::size_t {
		return std::hash<Package const*>()(ctx.package) ^ std::hash<String>()(ctx.scriptPath.string());
	}

};
inline auto operator==(LuaScriptContext const& lhs, LuaScriptContext const& rhs) -> bool {
	return std::tie(lhs.package, lhs.scriptPath) == std::tie(rhs.package, rhs.scriptPath);
}

// Helper function because the following code doesn't work somehow:
// sol::error( loadResult ).what()
// it required you to create a named variable. /shrug
auto getError(sol::load_result const& res) -> sol::error;

// Helper function because the following code doesn't work somehow:
// sol::error( functionResult ).what()
// it required you to create a named variable. /shrug
auto getError(sol::protected_function_result const& res) -> sol::error;

auto freshLuaInstance() -> sol::state;
