#pragma once

#include PACC_PCH

#include <Pacc/Helpers/Formatting.hpp>

struct PaccException
	: std::exception
{
	// TODO: this is not professional
	// String itself may cause exception
	std::string helpMessage;

public:
	using Super = std::exception;

	// Add super-class constructors
	using Super::Super;

	template <typename TFormat, typename... TArgs>
	PaccException(TFormat && fmt_, TArgs &&... args)
		:
		// TODO: Format may cause exception
		Super(fmt::format(
				std::forward<TFormat>(fmt_),
				std::forward<TArgs>(args)...
			).c_str())
	{
	}

	template <typename TFormat, typename... TArgs>
	PaccException const& withHelp(TFormat && fmt_, TArgs &&... args)
	{
		// TODO: Format may cause exception
		helpMessage = fmt::format(
				std::forward<TFormat>(fmt_),
				std::forward<TArgs>(args)...
			);
		return *this;
	}

	std::string const& help() const { return helpMessage; }
};



//////////////////////////////////////////////////
void dumpException(std::exception const& exc_);

//////////////////////////////////////////////////
void dumpException(PaccException const& exc_);