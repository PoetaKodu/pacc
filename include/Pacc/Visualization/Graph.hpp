#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/HelperTypes.hpp>

struct Package;

namespace viz
{

auto generateGraphContent(Package const& root_) -> String;


}
