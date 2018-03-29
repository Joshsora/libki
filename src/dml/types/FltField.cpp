#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void FltField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void FltField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t FltField::get_size() const
	{
		return 0;
	}
}
}