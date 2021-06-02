#include PACC_PCH

#include <Pacc/App/App.hpp>
#include <Pacc/Helpers/Exceptions.hpp>

///////////////////////////////////////////////////
void PaccApp::toolchains()
{
	auto const &tcs = cfg.detectedToolchains;

	if (args.size() >= 3)
	{
		int tcIdx = -1;

		try {
			tcIdx = std::stoi(std::string(args[2]));
		}
		catch(...) {}

		if (tcIdx < 0 || tcIdx >= int(tcs.size()))
		{
			throw PaccException("Invalid toolchain id \"{:.10}\"", args[2])
				.withHelp("Use \"pacc tc\" to list available toolchains.");
		}

		fmt::print("Changed selected toolchain to {} (\"{}\", version \"{}\")",
				tcIdx,
				tcs[tcIdx]->prettyName,	
				tcs[tcIdx]->version	
			);

		cfg.updateSelectedToolchain(tcIdx);
	}
	else
	{
		// Display toolchains
		std::cout << "TOOLCHAINS:\n";
			
		if (!tcs.empty())
		{
			using fmt::fg, fmt::color;
			using namespace fmt::literals;

			auto const& style = fmt_args::s();

			size_t maxNameLen = 20;
			for(auto& tc : tcs)
				maxNameLen = std::max(maxNameLen, tc->prettyName.length());
			fmt::print("    ID{0:4}{Name}{0:{NameLen}}{Version}\n{0:-^{NumDashes}}\n",
					"",
					FMT_INLINE_ARG("Name", 		fg(color::lime_green), "Name"),
					FMT_INLINE_ARG("Version", 	fg(color::aqua), "Version"),

					"NameLen"_a 	= maxNameLen,
					"NumDashes"_a 	= maxNameLen + 20 + 4
				);

			// TODO: add user configuration with specified default toolchain.
			int idx = 0;
			for (auto& tc : tcs)
			{
				bool selected = (idx == cfg.selectedToolchain);
				auto style = selected ? fmt::emphasis::bold : fmt::text_style{};
				fmt::print(style, "{:>6}    {:{NameLen}}    {:10}\n",
						fmt::format("{} #{}", selected ? '>' : ' ', idx),
						tc->prettyName,
						tc->version,

						"NameLen"_a = maxNameLen
					);
				idx++;
			}
		}
		else
		{
			fmt::print("\tNo toolchains detected :(\n");
		}
	}
}