#pragma once

#include <Pacc/PaccPCH.hpp>

namespace fmt
{

void enableColors();

template <typename TFormat, typename... TArgs>
void printToStream(std::ostream& stream_, TFormat && fmt_, TArgs &&... args)
{
	stream_ << fmt::format(
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

namespace fmt_args
{
	struct Styles {
		fmt::text_style Bold	= fmt::emphasis::bold;
		fmt::text_style Red		= fmt::fg(fmt::color::red);
		fmt::text_style Green	= fmt::fg(fmt::color::green);
		fmt::text_style Blue	= fmt::fg(fmt::color::blue);
		fmt::text_style Yellow	= fmt::fg(fmt::color::yellow);

		fmt::text_style ErrorMessage	= Bold | fmt::bg(fmt::color::dark_red);
		fmt::text_style HelpMessage		= Bold | fmt::fg(fmt::color::white) | fmt::bg(fmt::rgb(0x8500a6));
		fmt::text_style NoteMessage		= Bold | fmt::fg(fmt::color::white) | fmt::bg(fmt::rgb(0x0556c6));
	};

	inline Styles const& s() {
		static auto instance = Styles{};
		return instance;
	}

	#define DEFINE_FMT_ARG(funcName, argName, style, content) \
		inline auto funcName() \
		{ \
			static auto Val = fmt::format(style, content); \
			return fmt::arg(argName, Val); \
		}

	#define FMT_INLINE_ARG(argName, style, content) \
		([&]{ \
			static auto Val = fmt::format(style, content); \
			return fmt::arg(argName, Val); \
		})()

	DEFINE_FMT_ARG(error, 	"Error", 	s().ErrorMessage, 	"/  Error  /");
	DEFINE_FMT_ARG(help, 	"Help", 	s().HelpMessage, 	"/  Help   /");
	DEFINE_FMT_ARG(note, 	"Note", 	s().NoteMessage, 	"/  Note   /");
	DEFINE_FMT_ARG(details, "Details", 	s().Bold, 			"Details");

	#undef DEFINE_FMT_ARG
}
