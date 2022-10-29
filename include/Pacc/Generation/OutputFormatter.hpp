#pragma once

#include PACC_PCH

struct OutputFormatter
{
	String& output;
	int indent = 0;

	void writeIndent()
	{
		for(int i = 0; i < indent; ++i)
			output += '\t';
	}

	template <bool Indent = true, typename FirstArg, typename... Args>
	void write(FirstArg&& firstArg_, Args&&... args_)
	{
		if constexpr (Indent) {
			this->writeIndent();
		}
		output += fmt::format( fmt::runtime(std::forward<FirstArg>(firstArg_)), std::forward<Args>(args_)... );
	}

	void writeRaw(StringView s)
	{
		output += s;
	}
};

struct IndentScope
{
	OutputFormatter& fmt;
	IndentScope(OutputFormatter& fmt_)
		: fmt(fmt_)
	{
		++fmt.indent;
	}
	~IndentScope() {
		--fmt.indent;
	}
};
