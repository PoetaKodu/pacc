#include "include/Pacc/PaccPCH.hpp"


#include <Pacc/System/Process.hpp>

///////////////////////////////////////
ChildProcess::ExitCode ChildProcess::runSync()
{
	auto prevWorkingDirectory = fs::current_path();
	if (workingDirectory != "")
		fs::current_path(workingDirectory);

	// TODO: remove this hack, use UNICODE!!!
	#ifdef PACC_SYSTEM_WINDOWS
		std::wstring 	theCommand(command.begin(), command.end());
		std::wstring 	env = L"";
	#else
		String const& theCommand = this->command;
		String 	env = "";
	#endif

	proc::Process proc(theCommand, env,
		// Handle stdout:
		[&](const char *bytes, size_t n)
		{
			if (printRealTime || storeOutput)
			{
				auto s = String(bytes, n);
				if (printRealTime)
				{
					std::cout << s;
					if(bytes[n - 1] != '\n')
						std::cout << std::endl;
				}
				if (storeOutput)
					out.stdOut += std::move(s);
			}
		},
		// Handle stderr:
		[&](const char *bytes, size_t n)
		{
			if (printRealTime || storeOutput)
			{
				auto s = String(bytes, n);

				if (printRealTime)
				{
					std::cerr << s;
					if(bytes[n - 1] != '\n')
						std::cerr << std::endl;
				}

				if (storeOutput)
					out.stdErr += std::move(s);
			}
		}
	);

	auto killed		= false;
	auto exitStatus	= 1;
	auto startTime	= ch::steady_clock::now();

	while(!proc.try_get_exit_status(exitStatus))
	{
		if (timeout.has_value())
		{
			if (ch::steady_clock::now() > startTime + timeout.value())
			{
				proc.kill();
				killed = true;
				break;
			}
		}

		tt::sleep_for(passiveSleepStep);
	}

	if (workingDirectory != "")
		fs::current_path(prevWorkingDirectory);

	if (killed)
	{
		return std::nullopt;
	}

	exitCode = exitStatus;
	return exitStatus;
}
