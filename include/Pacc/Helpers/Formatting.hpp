#pragma once

#include PACC_PCH

namespace fmt
{

template <typename TFormat, typename... TArgs>
void printToStream(std::ostream& stream_, TFormat && fmt_, TArgs &&... args)
{
	stream_ << format(
			std::forward<TFormat>(fmt_),
			std::forward<TArgs>(args)...
		);
}

template <typename TFormat, typename... TArgs>
void printErr(TFormat && fmt_, TArgs &&... args)
{
	printToStream(std::cerr,
			std::forward<TFormat>(fmt_),
			std::forward<TArgs>(args)...
		);
}

template <typename TFormat, typename... TArgs>
void printLog(TFormat && fmt_, TArgs &&... args)
{
	printToStream(std::clog,
			std::forward<TFormat>(fmt_),
			std::forward<TArgs>(args)...
		);
}

}
