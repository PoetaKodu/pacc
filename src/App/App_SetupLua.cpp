#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>
#include <Pacc/PackageSystem/Version.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/Helpers/Lua.hpp>

//////////////////////////////////////////////////
void PaccApp::setupLua()
{
	lua = freshLuaInstance();
}
