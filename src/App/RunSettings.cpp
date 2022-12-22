#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/RunSettings.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

static auto addFlag(ProgramFlagMap& flags, std::initializer_list<StringView> invocations) -> ProgramFlag&
{
	assert(invocations.size() > 0 && "`addFlag` called with no invocations");

	auto flag = std::make_shared<ProgramFlag>(*invocations.begin(), "");

	for (auto& inv : invocations) {
		flags.emplace(inv, flag);
	}

	return *flag;
}

static auto setupFlagsByMainAction(ProgramFlagMap& flags, PaccMainAction mainAction)
{
	using Action = PaccMainAction;

	switch (mainAction)
	{
	case Action::Install:
	case Action::Uninstall:
	{
		addFlag(flags, { "--global", "-g" });
		break;
	}
	case Action::Logs:
	{
		addFlag(flags, { "--last" });
		break;
	}
	case Action::ListVersions:
	{
		addFlag(flags, { "--tags" });
		addFlag(flags, { "--all" });
		break;
	}
	case Action::Build:
	case Action::Generate:
	{
		addFlag(flags, { "--compile-commands", "-cc" });
		break;
	}
	}
}

auto RunSettings::wasParsed(size_t programArgIdx) const -> bool
{
	return rg::find(parsedArgs, programArgIdx) != parsedArgs.end();
}

auto RunSettings::nthActionArgument(size_t n) const -> Opt<size_t>
{
	for (size_t i = actionNameIndex + 1; i < args->size(); ++i)
	{
		if (this->wasParsed(i))
			continue;

		if (n == 0)
			return i;

		--n;
	}

	return std::nullopt;
}

auto RunSettings::fromArgs(Vec<String> const& args_) -> RunSettings
{
	auto settings = RunSettings();
	settings.args = &args_;

	settings.parsedArgs.reserve(args_.size());

	// Setup global flags
	{
		// Configure the level of verbosity
		addFlag(settings.flags, { "--verbose" });

		// Configure the number of cores to build with
		addFlag(settings.flags, { "--cores" });

		// Configure the path to Lua SDK
		addFlag(settings.flags, { "--lua-lib" });

		// Configure the Premake5 executable
		addFlag(settings.flags, { "--premake5" });

		// # Build and generation-related stuff

		// Configure the target platform, e.g. x64, x86
		addFlag(settings.flags, { "--platform", "--plat", "-p" });

		// Configure the target configuration, e.g. Debug, Release, etc.
		addFlag(settings.flags, { "--configuration", "--config", "--cfg", "-c" });

		// Configure the output (path, or other)
		addFlag(settings.flags, { "--output", "-o" });

		// Select the target
		addFlag(settings.flags, { "--target" });
	}


	for (size_t i = 1; i < args_.size(); ++i)
	{
		auto& current = args_[i];

		// Try to parse a flag
		if (current.starts_with('-'))
		{
			auto equalsPos = current.find('=');

			// it didn't require looking the next argument
			if (equalsPos != String::npos)
			{
				auto flagName = current.substr(0, equalsPos);
				auto flagValue = current.substr(equalsPos + 1);

				auto flagIt = settings.flags.find(flagName);
				if (flagIt == settings.flags.end())
				{
					fmt::print(fmt::fg(fmt::color::orange), "Unrecognized flag: {} with assigned value {}\n", flagName, flagValue);
					continue;
				}

				auto& flag = *flagIt->second;

				// Make `name` and `value` point to the actual string from the arguments
				// because they are guaranteed to be stored for the whole lifetime of the program
				flag.name = StringView(current.data(), flagName.size());
				flag.value = StringView(current.data() + flagName.size() + 1, flagValue.size());

				flag.startIndex = i;
				flag.endIndex = flag.startIndex;

				settings.parsedArgs.push_back(i);
			}
			else
			{
				auto flagIt = settings.flags.find(current);
				if (flagIt == settings.flags.end())
				{
					// if we didn't stumble upon the main action yet, then this means it has a format like:
					// pacc --someUnrecognizedFlag [other args]
					// so this has to be logged
					if (settings.mainAction == PaccMainAction::None)
					{
						fmt::print(fmt::fg(fmt::color::orange), "Unrecognized flag: {}\n", current);
					}

					// otherwise, just ignore it, it may be used by the main action
				}
				else
				{
					auto& flag = *flagIt->second;
					flag.name = current;
					flag.value = "true"; // can be overriden later
					settings.parsedArgs.push_back(i);

					if (flag.canBeMultiIndex)
					{
						// is not the last argument
						if (i + 1 == args_.size())
						{
							flag.value = "true";
							flag.startIndex = i;
							flag.endIndex = i;
						}
						else
						{
							flag.value = args_[i + 1];
							flag.startIndex = i;
							flag.endIndex = i + 1;
							settings.parsedArgs.push_back(i + 1);

							// skip the next argument
							++i;
						}

					}
				}
			}
		}
		else if (settings.mainAction == PaccMainAction::None)// it isn't a flag and main action wasn't set
		{
			settings.parsedArgs.push_back(i);

			settings.actionNameIndex = i;
			settings.mainAction = PaccMainAction::fromString(current);

			if (settings.mainAction == PaccMainAction::None)
			{
				return settings;
			}

			setupFlagsByMainAction(settings.flags, settings.mainAction);
		}

	}

	// Setup flags based on the main action
	{
		// TODO: limit previous flags based on the main action
	}

	return settings;
}

