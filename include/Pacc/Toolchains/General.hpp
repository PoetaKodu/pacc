#pragma once

#include PACC_PCH

#include <Pacc/Toolchains/Toolchain.hpp>

#include <Pacc/Helpers/HelperTypes.hpp>

Vec<SPtr<Toolchain>> detectAllToolchains();
