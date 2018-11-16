#pragma once
#include <stdexcept>

namespace ki
{
	class runtime_error : public std::runtime_error
	{
	public:
		explicit runtime_error(const std::string &message)
			: std::runtime_error(message) {}
	};
}