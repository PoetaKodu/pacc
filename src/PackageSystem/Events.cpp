#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/PackageSystem/Events.hpp>

auto EventHandlerActionStorage::add(String const& name_, UPtr<EventHandlerAction> action_) -> void
{
	actions[name_] = std::move(action_);
}

auto EventHandlerActionStorage::find(String const& name_) const -> EventHandlerAction const*
{
	auto it = actions.find(name_);
	if (it == actions.end())
		return nullptr;

	return it->second.get();
}

