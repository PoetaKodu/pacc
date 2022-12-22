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
	String noteMessage;

public:
	using Super = std::runtime_error;

	template <typename... TArgs>
	explicit PaccException(fmt::format_string<TArgs...> fmt_, TArgs &&... args)
		:
		// TODO: Format may cause exception
		Super(fmt::format(fmt_, std::forward<TArgs>(args)...).c_str())
	{
	}

	template <typename... TArgs>
	auto withHelp(fmt::format_string<TArgs...> fmt_, TArgs &&... args) -> PaccException&
	{
		// TODO: Format may cause exception
		helpMessage = fmt::format(fmt_, std::forward<TArgs>(args)...);
		return *this;
	}

	template <typename... TArgs>
	auto withNote(fmt::format_string<TArgs...> fmt_, TArgs &&... args) -> PaccException&
	{
		// TODO: Format may cause exception
		noteMessage = fmt::format(fmt_, std::forward<TArgs>(args)...);
		return *this;
	}

	String const& help() const { return helpMessage; }
	String const& note() const { return noteMessage; }
};



//////////////////////////////////////////////////
void dumpException(std::exception const& exc_);

//////////////////////////////////////////////////
void dumpException(PaccException const& exc_);
