#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>

namespace ki
{
namespace dml
{
	template <>
	void WStrField::write_to(std::ostream &ostream) const
	{
		ValueBytes<USHRT> data;
		data.value = m_value.length();
		if (data.buff[0] == ((m_value.length() & 0xFF00) >> 8))
			std::reverse(&data.buff[0], &data.buff[1]);
		ostream.write(data.buff, sizeof(USHRT));
		ostream.write((char *)m_value.c_str(), m_value.length() * sizeof(char16_t));
	}

	template <>
	void WStrField::read_from(std::istream &istream)
	{
		// Get the length
		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		ValueBytes<USHRT> length_data;
		istream.read(length_data.buff, sizeof(USHRT));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&length_data.buff[0], &length_data.buff[1]);

		// Read the data into a buffer
		size_t length = length_data.value * sizeof(char16_t);
		char *data = new char[length + sizeof(char16_t)]{ 0 };
		istream.read(data, length);
		for (int i = 0; i < length; i += 2)
		{
			if (endianness_check.buff[0] == 0x01)
				std::reverse(&data[i], &data[i + 1]);
		}
		m_value = WSTR((char16_t *)data);
		delete[] data;
	}

	template <>
	size_t WStrField::get_size() const
	{
		return sizeof(USHRT) + (m_value.length() * sizeof(char16_t));
	}
}
}
