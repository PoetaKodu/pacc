#pragma once

#include PACC_PCH

struct ChildProcess
{
	using ExitCode = std::optional<int>;

	std::string 		command;

	fs::path 			workingDirectory 	= ""; // "" => current working dir
	int 				timeout 			= -1; // -1 => no timeout
	bool				printRealTime 		= false;
	bool 				storeOutput 		= true;
	ch::milliseconds 	passiveSleepStep 	= ch::milliseconds{100};

	struct {
		std::string stdOut;
		std::string stdErr;
	} out{};

	ExitCode exitCode 	= std::nullopt;
	
	ExitCode runSync();
};

