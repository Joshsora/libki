#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void GidField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void GidField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t GidField::get_size() const
	{
		return 0;
	}
}
}