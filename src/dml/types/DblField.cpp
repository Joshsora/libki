#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void DblField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void DblField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t DblField::get_size() const
	{
		return 0;
	}
}
}