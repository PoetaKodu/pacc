#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/HelperTypes.hpp>

struct Package;

/// @brief A descriptor for a basic event handler.
struct BasicEventHandlerDesc
{
	/// @brief Type of action
	/// Syntax:
	/// "<type>:<rest>"
	/// Examples:
	/// "lua:handleFetchStart"
	/// "shallow_clone:github/user/repo"
	/// "lua:printHelloWorld"
	String action;

	virtual ~BasicEventHandlerDesc() {}
};

/// @brief There are many events related to a target
/// that user can manually respond to.
/// This class is meant to be used as a base for all
/// entities that have such event handling capabilities.
struct EventHandlingEntity
{
	UMap<String, Vec< UPtr<BasicEventHandlerDesc> >> eventHandlers;
};

struct EventHandlerAction
{
	bool canLoadFromAbbreviatedString = false;

	virtual ~EventHandlerAction() {}

	virtual auto load(json const& object_) const -> UPtr<BasicEventHandlerDesc> = 0;

	/// @brief
	/// @param abbreviatedAction_
	/// @return
	virtual auto load(String const& abbreviatedAction_) const -> UPtr<BasicEventHandlerDesc> = 0;

	virtual auto execute(Package& context_, BasicEventHandlerDesc const& desc_) const -> void = 0;
};

class EventHandlerActionStorage
{
protected:
	UMap<String, UPtr<EventHandlerAction>> actions;
public:

	auto add(String const& name_, UPtr<EventHandlerAction> action_) -> void;
	auto find(String const& name_) const -> EventHandlerAction const*;
};

/// @brief Pacc app module that handles registering and finding
/// the EventHandler action types.
class PaccAppModule_EventHandlerActions
{
protected:
	EventHandlerActionStorage actions;

public:
	auto registerEventAction(String const& name_, UPtr<EventHandlerAction> action_) -> void
	{
		actions.add(name_, std::move(action_));
	}

	auto findEventAction(String const& name_) const -> EventHandlerAction const*
	{
		return actions.find(name_);
	}
};
