#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>

namespace ki
{
namespace dml
{
	template <>
	void UShrtField::write_to(std::ostream &ostream) const
	{
		ValueBytes<USHRT> data;
		data.value = m_value;
		if (data.buff[0] == ((m_value & 0xFF00) >> 8))
			std::reverse(&data.buff[0], &data.buff[2]);
		ostream.write(data.buff, sizeof(USHRT));
	}

	template <>
	void UShrtField::read_from(std::istream &istream)
	{
		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		ValueBytes<USHRT> data;
		istream.read(data.buff, sizeof(USHRT));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[2]);
		m_value = data.value;
	}

	template <>
	size_t UShrtField::get_size() const
	{
		return sizeof(USHRT);
	}
}
}
