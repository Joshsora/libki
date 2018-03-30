#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"

namespace ki
{
namespace dml
{
	template <>
	void IntField::write_to(std::ostream &ostream) const
	{
		ValueBytes<INT> data = { m_value };
		if (data.buff[0] == ((m_value & 0xFF000000) >> 24))
			std::reverse(&data.buff[0], &data.buff[3]);
		ostream.write(data.buff, sizeof(INT));
	}

	template <>
	void IntField::read_from(std::istream &istream)
	{
		const ValueBytes<USHRT> endianness_check = { 0x0102 };
		ValueBytes<INT> data;
		istream.read(data.buff, sizeof(INT));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[3]);
		m_value = data.value;
	}

	template <>
	size_t IntField::get_size() const
	{
		return sizeof(INT);
	}
}
}
