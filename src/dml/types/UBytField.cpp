#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	template <>
	void UBytField::write_to(std::ostream &ostream) const
	{

	}

	template <>
	void UBytField::read_from(std::istream &istream)
	{

	}

	template <>
	size_t UBytField::get_size() const
	{
		return 0;
	}
}
}
