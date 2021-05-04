#include PACC_PCH

#include <Pacc/Filesystem.hpp>

namespace fsx
{


//////////////////////////////////////
fs::path fwd(fs::path p_)
{
	// On windows replace backslashes with slashes 
	#ifdef PACC_SYSTEM_WINDOWS
		std::wstring w = p_.wstring();
		std::replace(w.begin(), w.end(), L'\\', L'/');
		p_ = w;
	#endif

	return p_;
}

}