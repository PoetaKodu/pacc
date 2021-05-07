#include PACC_PCH

#include <Pacc/Process.hpp>

///////////////////////////////////////
ChildProcess::ExitCode ChildProcess::runSync()
{
	auto prevWorkingDirectory = fs::current_path();
	if (workingDirectory != "")
		fs::current_path(workingDirectory); 
		
	proc::Process proc(command, "",
		// Handle stdout:
		[&](const char *bytes, size_t n)
		{
			if (printRealTime || storeOutput)
			{
				std::string s(bytes, n);
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
				std::string s(bytes, n);

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

	bool killed 	= false;
	int exitStatus 	= 1;
	int runTime 	= 0;
	while(!proc.try_get_exit_status(exitStatus))
	{
		if (timeout != -1)
		{
			if (runTime++ > timeout * 10)
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
		return std::nullopt;

	return exitStatus;
}
