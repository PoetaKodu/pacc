#pragma once

#include PACC_PCH

#include <Pacc/Helpers/HelperTypes.hpp>

struct Package;

namespace viz
{

auto generateGraphContent(Package const& root_) -> String;


}
