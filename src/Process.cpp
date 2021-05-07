#include PACC_PCH

#include <Pacc/Process.hpp>

///////////////////////////////////////////////////
std::optional<int> runChildProcessSync(std::string const& command_, std::string cwd, int timeOutSecs, bool printOutput_)
{
	auto prevCwd = fs::current_path();
	if (cwd != "")
		fs::current_path(cwd); 
		
	proc::Process proc(command_, "",
		// Handle stdout:
		[printOutput_](const char *bytes, size_t n)
		{
			if (printOutput_)
			{
				std::cout << std::string(bytes, n);
				if(bytes[n - 1] != '\n')
					std::cout << std::endl;
			}
		},
		// Handle stderr:
		[printOutput_](const char *bytes, size_t n)
		{
			if (printOutput_)
			{
				std::cerr << std::string(bytes, n);
				if(bytes[n - 1] != '\n')
					std::cout << std::endl;
			}
		}
	);

	bool killed 	= false;
	int exitStatus 	= 1;
	int runTime 	= 0;
	while(!proc.try_get_exit_status(exitStatus))
	{
		if (timeOutSecs != -1)
		{
			if (runTime++ > timeOutSecs * 10)
			{
				proc.kill();
				killed = true;
				break;
			}
		}

		tt::sleep_for(ch::milliseconds{100});
	}

	if (cwd != "")
		fs::current_path(prevCwd); 

	if (killed)
		return std::nullopt;

	return exitStatus;
}

