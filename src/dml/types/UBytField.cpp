#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"

namespace ki
{
namespace dml
{
	template <>
	void UBytField::write_to(std::ostream &ostream) const
	{
		ValueBytes<UBYT> data;
		data.value = m_value;
		ostream.write(data.buff, sizeof(UBYT));
	}

	template <>
	void UBytField::read_from(std::istream &istream)
	{
		ValueBytes<UBYT> data;
		istream.read(data.buff, sizeof(UBYT));
		m_value = data.value;
	}

	template <>
	size_t UBytField::get_size() const
	{
		return sizeof(BYT);
	}
}
}
