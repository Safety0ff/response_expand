// This software is dual-licensed:
//   This software may be redistributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//   This software may be redistributed under the new BSD license.
//   (See accompanying file LICENSE_NEWBSD)

#include <stdlib.h>
#include <string.h>

#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

// returns true if the quote is unescaped
bool applyBackslashRule(std::string& arg)
{
	std::string::reverse_iterator it;
	for (it = arg.rbegin(); it!=arg.rend() && *it=='\\'; ++it) {}
	size_t numbs = std::distance(arg.rbegin(), it);
	bool escapedquote = numbs % 2;
	size_t numescaped = numbs/2+escapedquote;
	arg.resize(arg.size()-numescaped);
	if (escapedquote)
		arg += '"';
	return !escapedquote;
}

// returns true if the end of the quote is the end of the argument
bool dealWithQuote(std::istream& is, std::string& arg)
{
	// first go back and deal with backslashes
	if (!applyBackslashRule(arg))
		return false;

	// keep appending until we find a quote terminator
	while (is.good())
	{
		char c = is.get();
		switch (c)
		{
			case '"':
				if (applyBackslashRule(arg))
					return false;
				break;
			case EOF: // new line or EOF ends quote and argument
			case '\n':
				return true;
			case '\r': // ignore carriage returns in quotes
				break;
			default:
				arg += c;
		}
	}
	return true; // EOF
}

void dealWithComment(std::istream& is)
{
	while (is.good())
	{
		char c = is.get();
		if (c == '\n' || c == '\r')
				return; // newline, carriage return and EOF end comment
	}
}

std::vector<std::string> expand(std::istream& is)
{
	std::vector<std::string> expanded;
	std::string arg;

	is >> std::ws;
	while(is.good())
	{
		char c = is.get();
		if (c == EOF)
		{
			break;
		}
		else if (std::isspace(c))
		{
			is >> std::ws;
			if (!arg.empty())
			{
				expanded.push_back(arg);
				arg.clear();
			}
		}
		else if (c == '"')
		{
			if (dealWithQuote(is, arg) && !arg.empty())
			{
				expanded.push_back(arg);
				arg.clear();
			}
		}
		else if (c == '#')
		{
				dealWithComment(is);
		}
		else
		{
			arg += c;
			while (is.good())
			{
				c = is.peek();
				if (std::isspace(c) || c == '"' || c == EOF)
					break; // N.B. comments can't be placed in the middle of an arg
				else
					arg += is.get();
			}
		}
	}
	if (!arg.empty())
		expanded.push_back(arg);
	return expanded;
}

extern "C" int response_expand(size_t *pargc, char ***ppargv)
{
	// N.B. It is possible to create an infinite loop with response arguments
	// in the original implementation, we artificially limmit re-parsing responses
	const unsigned reexpand_limit = 32;
	std::map<std::string, unsigned> response_args;

	// Compile a list of arguments, at the end convert them to the proper C string form
	std::vector<std::string> processed_args;
	std::list<std::string> unprocessed_args;

	// fill unprocessed with initial arguments
	for (size_t i = 0; i < *pargc; ++i)
	{
		unprocessed_args.push_back((*ppargv)[i]);
	}

	processed_args.reserve(*pargc);

	while (!unprocessed_args.empty())
	{
		std::string arg = unprocessed_args.front();
		unprocessed_args.pop_front();

		if (!arg.empty() && arg[0] == '@')
		{
			arg.erase(arg.begin()); // remove leading '@'
			if (arg.empty())
			{
				return 1;
			}
			else
			{
				if (response_args.find(arg) == response_args.end())
					response_args[arg] = 1;
				else if (++response_args[arg] > reexpand_limit)
					return 2; // We might be in an infinite loop

				std::vector<std::string> expanded_args;
				const char* env = getenv(arg.c_str());
				if (env)
				{
					std::string envCppStr(env);
					std::istringstream ss(envCppStr);
					expanded_args = expand(ss);
				}
				else
				{
					std::ifstream ifs(arg.c_str());
					if (ifs.good())
						expanded_args = expand(ifs);
					else
						return 3; // response not found between environment and files
				}
				unprocessed_args.insert(unprocessed_args.begin(),
										expanded_args.begin(), expanded_args.end());
			}
		}
		else if (!arg.empty())
		{
			processed_args.push_back(arg);
		}
	}

	char** pargv = (char**) malloc(sizeof(pargv) * processed_args.size());

	for (size_t i = 0; i < processed_args.size(); ++i)
	{
		pargv[i] = (char*) malloc(sizeof(pargv[i]) * (1 + processed_args[i].length()));
		strcpy(pargv[i], processed_args[i].c_str());
	}

	*pargc = processed_args.size();
	*ppargv = pargv;

	return 0;
}
