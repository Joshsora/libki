#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void IntField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void IntField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t IntField::get_size() const
	{
		return 0;
	}
}
}
