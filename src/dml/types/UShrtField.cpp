#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void UShrtField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void UShrtField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t UShrtField::get_size() const
	{
		return 0;
	}
}
}
