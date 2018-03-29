#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void BytField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void BytField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t BytField::get_size() const
	{
		return 0;
	}
}
}