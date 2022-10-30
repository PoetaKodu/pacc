#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/HelperTypes.hpp>

struct ChildProcess
{
	using ExitCode 	= Opt<int>;
	using Timeout 	= Opt<ch::milliseconds>;

	String 		command;

	fs::path 			workingDirectory 	= ""; // "" => current working dir
	Timeout				timeout 			= std::nullopt;
	bool				printRealTime 		= false;
	bool 				storeOutput 		= true;
	ch::milliseconds 	passiveSleepStep 	= ch::milliseconds{10};

	struct {
		String stdOut;
		String stdErr;
	} out{};

	ExitCode exitCode 	= std::nullopt;

	ExitCode runSync();
};

