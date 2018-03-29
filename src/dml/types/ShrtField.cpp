#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void ShrtField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void ShrtField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t ShrtField::get_size() const
	{
		return 0;
	}
}
}
