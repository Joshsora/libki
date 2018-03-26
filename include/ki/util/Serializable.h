#pragma once
#include <istream>

namespace ki
{
namespace util
{
	class Serializable
	{
	public:
		virtual void write_to(std::ostream &ostream) const = 0;
		virtual void read_from(std::istream &istream) = 0;
		virtual size_t get_size() const = 0;
	};
}
}
