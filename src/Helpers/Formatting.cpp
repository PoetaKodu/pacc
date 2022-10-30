#include "include/Pacc/PaccPCH.hpp"

#ifdef PACC_SYSTEM_WINDOWS
#include <Windows.h>
#endif

namespace fmt
{

/////////////////////////////////
void enableColors()
{
	#ifdef PACC_SYSTEM_WINDOWS
	DWORD consoleMode;
	HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleMode(outputHandle, &consoleMode))
	{
		SetConsoleMode(outputHandle, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
	#endif
}

}
