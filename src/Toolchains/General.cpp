#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Toolchains/General.hpp>

#include <Pacc/Toolchains/MSVC.hpp>
#include <Pacc/Toolchains/GNUMake.hpp>



///////////////////////////////////////////
// Private functions (forward declaration)
///////////////////////////////////////////

template <typename ToolchainType>
void detectToolchainsByType(Vec<SPtr<Toolchain>> &out_);


///////////////////////////////////////////
// Public functions:
///////////////////////////////////////////


/////////////////////////////////
Vec<SPtr<Toolchain>> detectAllToolchains()
{
	Vec<SPtr<Toolchain>> result;

	#ifdef PACC_SYSTEM_WINDOWS
		detectToolchainsByType<MSVCToolchain>(result);
	#endif

	detectToolchainsByType<GNUMakeToolchain>(result);

	return result;
}


/////////////////////////////////
template <typename ToolchainType>
void detectToolchainsByType(Vec<SPtr<Toolchain>> &out_)
{
	auto tcs = ToolchainType::detect();
	for(auto& tc : tcs)
		out_.push_back( std::make_shared<ToolchainType>( std::move(tc) ) );
}
