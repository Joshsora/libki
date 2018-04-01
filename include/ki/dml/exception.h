#pragma once
#include <stdexcept>

namespace ki
{
namespace dml
{
	class runtime_error : public std::runtime_error
	{
	public:
		runtime_error(std::string message) : std::runtime_error(message) { }
	};

	class parse_error : public runtime_error
	{
	public:
		parse_error(std::string message) : runtime_error(message) { }
	};

	class value_error : public runtime_error
	{
	public:
		value_error(std::string message) : runtime_error(message) { }
	};
}
}