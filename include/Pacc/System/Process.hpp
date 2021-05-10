#pragma once

#include PACC_PCH

struct ChildProcess
{
	using ExitCode 	= std::optional<int>;
	using Timeout 	= std::optional<ch::milliseconds>;

	std::string 		command;

	fs::path 			workingDirectory 	= ""; // "" => current working dir
	Timeout				timeout 			= std::nullopt;
	bool				printRealTime 		= false;
	bool 				storeOutput 		= true;
	ch::milliseconds 	passiveSleepStep 	= ch::milliseconds{10};

	struct {
		std::string stdOut;
		std::string stdErr;
	} out{};

	ExitCode exitCode 	= std::nullopt;
	
	ExitCode runSync();
};

