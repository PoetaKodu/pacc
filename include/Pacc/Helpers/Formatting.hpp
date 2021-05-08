#pragma once

#include PACC_PCH

namespace fmt
{

void enableColors();

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

namespace fmt_args
{
	struct Styles {
		fmt::text_style Bold	= fmt::emphasis::bold;
		fmt::text_style Red		= fmt::fg(fmt::color::red);
		fmt::text_style Green	= fmt::fg(fmt::color::green);
		fmt::text_style Blue	= fmt::fg(fmt::color::blue);
		fmt::text_style Yellow	= fmt::fg(fmt::color::yellow);
	};

	inline Styles const& s() {
		static Styles instance{};
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

	DEFINE_FMT_ARG(error, 	"Error", 	s().Bold | s().Red, 	"[Error]");
	DEFINE_FMT_ARG(help, 	"Help", 	s().Bold | s().Yellow, 	"[Help]");
	DEFINE_FMT_ARG(details, "Details", 	s().Bold, 				"Details");

	#undef DEFINE_FMT_ARG
}