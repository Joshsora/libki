#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void UIntField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void UIntField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t UIntField::get_size() const
	{
		return 0;
	}
}
}
