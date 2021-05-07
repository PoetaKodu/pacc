#pragma once

#include PACC_PCH

std::optional<int> 	runChildProcessSync(
		std::string const& command_,
		std::string cwd = "",
		int timeOutSecs = -1,
		bool printOutput_ = false
	);

