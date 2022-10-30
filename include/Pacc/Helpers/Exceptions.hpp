#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/Formatting.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

struct PaccException
	: std::runtime_error
{
	// TODO: this is not professional
	// String itself may cause exception
	String helpMessage;

public:
	using Super = std::runtime_error;

	template <typename TFormat, typename... TArgs>
	explicit PaccException(TFormat && fmt_, TArgs &&... args)
		:
		// TODO: Format may cause exception
		Super(fmt::format(
				fmt::runtime(std::forward<TFormat>(fmt_)),
				std::forward<TArgs>(args)...
			).c_str())
	{
	}

	template <typename TFormat, typename... TArgs>
	PaccException const& withHelp(TFormat && fmt_, TArgs &&... args)
	{
		// TODO: Format may cause exception
		helpMessage = fmt::format(
				fmt::runtime(std::forward<TFormat>(fmt_)),
				std::forward<TArgs>(args)...
			);
		return *this;
	}

	String const& help() const { return helpMessage; }
};



//////////////////////////////////////////////////
void dumpException(std::exception const& exc_);

//////////////////////////////////////////////////
void dumpException(PaccException const& exc_);
