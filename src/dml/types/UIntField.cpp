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
			std::reverse(&data.buff[0], &data.buff[4]);
		ostream.write(data.buff, sizeof(UINT));
	}

	template <>
	void UIntField::read_from(std::istream &istream)
	{
		ValueBytes<UINT> data;
		istream.read(data.buff, sizeof(UINT));
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read UINT value (" << m_name << ").";
			throw parse_error(oss.str());
		}

		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[4]);

		m_value = data.value;
	}

	template <>
	size_t UIntField::get_size() const
	{
		return sizeof(UINT);
	}

	template <>
	const char* UIntField::get_type_name() const
	{
		return "UINT";
	}
}
}
