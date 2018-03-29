#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void StrField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void StrField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t StrField::get_size() const
	{
		return 0;
	}
}
}
