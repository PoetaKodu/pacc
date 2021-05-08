#include PACC_PCH

#include <Pacc/Toolchains/General.hpp>

#include <Pacc/Toolchains/MSVC.hpp>
#include <Pacc/Toolchains/GNUMake.hpp>



///////////////////////////////////////////
// Private functions (forward declaration)
///////////////////////////////////////////

template <typename ToolchainType>
void detectToolchainsByType(Vec<UPtr<Toolchain>> &out_);


///////////////////////////////////////////
// Public functions:
///////////////////////////////////////////


/////////////////////////////////
Vec<UPtr<Toolchain>> detectAllToolchains()
{
	Vec<UPtr<Toolchain>> result;

	#ifdef PACC_SYSTEM_WINDOWS
		detectToolchainsByType<MSVCToolchain>(result);
	#endif

	detectToolchainsByType<GNUMakeToolchain>(result);

	return result;
}


/////////////////////////////////
template <typename ToolchainType>
void detectToolchainsByType(Vec<UPtr<Toolchain>> &out_)
{
	auto tcs = ToolchainType::detect();
	for(auto& tc : tcs)
		out_.push_back( std::make_unique<ToolchainType>( std::move(tc) ) );	
}