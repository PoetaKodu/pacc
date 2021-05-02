#pragma once

#include PACC_PCH

#include <Pacc/Main.hpp>
#include <Pacc/Package.hpp>

namespace actions
{

// help
void 			displayHelp(ProgramArgs const& args_, bool full_);
// link
void 			linkPackage(ProgramArgs const& args_);
// unlink
void 			unlinkPackage(ProgramArgs const& args_);
// generate
Package 		generate(ProgramArgs const& args_);
// build
void 			buildPackage(ProgramArgs const& args_);
// init
void 			initPackage();

}