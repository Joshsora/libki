#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>

namespace ki
{
namespace dml
{
	template <>
	void ShrtField::write_to(std::ostream &ostream) const
	{
		ValueBytes<SHRT> data;
		data.value = m_value;
		if (data.buff[0] == ((m_value & 0xFF00) >> 8))
			std::reverse(&data.buff[0], &data.buff[2]);
		ostream.write(data.buff, sizeof(SHRT));
	}

	template <>
	void ShrtField::read_from(std::istream &istream)
	{
		ValueBytes<SHRT> data;
		istream.read(data.buff, sizeof(SHRT));
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read SHRT value (" << m_name << ").";
			throw parse_error(oss.str());
		}

		ValueBytes<SHRT> endianness_check;
		endianness_check.value = 0x0102;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[2]);

		m_value = data.value;
	}

	template <>
	size_t ShrtField::get_size() const
	{
		return sizeof(USHRT);
	}

	template <>
	const char* ShrtField::get_type_name() const
	{
		return "SHRT";
	}
}
}
