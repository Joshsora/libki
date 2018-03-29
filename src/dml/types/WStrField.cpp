#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void WStrField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void WStrField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t WStrField::get_size() const
	{
		return 0;
	}
}
}
