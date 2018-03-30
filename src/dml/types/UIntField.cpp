#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>

namespace ki
{
namespace dml
{
	template <>
	void UIntField::write_to(std::ostream &ostream) const
	{
		ValueBytes<UINT> data;
		data.value = m_value;
		if (data.buff[0] == ((m_value & 0xFF000000) >> 24))
			std::reverse(&data.buff[0], &data.buff[3]);
		ostream.write(data.buff, sizeof(UINT));
	}

	template <>
	void UIntField::read_from(std::istream &istream)
	{
		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		ValueBytes<UINT> data;
		istream.read(data.buff, sizeof(UINT));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[3]);
		m_value = data.value;
	}

	template <>
	size_t UIntField::get_size() const
	{
		return sizeof(UINT);
	}
}
}
