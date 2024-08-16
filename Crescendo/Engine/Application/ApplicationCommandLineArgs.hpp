#pragma once

#include "common.hpp"
#include <unordered_map>

CS_NAMESPACE_BEGIN
{
	struct ApplicationCommandLineArgs
	{
	private:
		// key-value pairs
		std::unordered_map<std::string, std::string> args;
	public:
		ApplicationCommandLineArgs(int argc, char** argv)
		{
			// Ignore first command
			for (int i = 1; i < argc; i++)
			{
				std::string arg = argv[i];
				if (arg.find('=') != std::string::npos)
				{
					std::string key = arg.substr(0, arg.find('='));
					std::string value = arg.substr(arg.find('=') + 1);
					args[key] = value;
				}
				else
				{
					cs_std::console::warn("invalid argument: " + arg);
				}
			}
		}
		std::string GetArg(const std::string& key) const
		{
			if (HasArg(key))
			{
				return args.at(key);
			}
			else
			{
				cs_std::console::warn("argument not found: " + key);
				return "";
			}
		}
		bool HasArg(const std::string& key) const
		{
			return this->args.find(key) != this->args.end();
		}
	};
}