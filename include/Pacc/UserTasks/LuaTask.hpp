#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/PackageSystem/Events.hpp>

/// @brief A descriptor for an event handler that is a Lua function.
struct LuaEventHandlerDesc
	: BasicEventHandlerDesc
{
	String moduleName;
	String functionName;
};


struct LuaTaskAction
	: EventHandlerAction
{
	LuaTaskAction() {
		canLoadFromAbbreviatedString = true;
	}

	auto load(json const& object_) const -> UPtr<BasicEventHandlerDesc> override;

	auto load(String const& abbreviatedAction_) const -> UPtr<BasicEventHandlerDesc> override;

	auto execute(Package& context_, BasicEventHandlerDesc const& desc_) const -> void override;
};
