#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"

namespace ki
{
namespace dml
{
	template <>
	void ShrtField::write_to(std::ostream &ostream) const
	{
		ValueBytes<SHRT> data = { m_value };
		if (data.buff[0] == ((m_value & 0xFF00) >> 8))
			std::reverse(&data.buff[0], &data.buff[1]);
		ostream.write(data.buff, sizeof(SHRT));
	}

	template <>
	void ShrtField::read_from(std::istream &istream)
	{
		const ValueBytes<SHRT> endianness_check = { 0x0102 };
		ValueBytes<SHRT> data;
		istream.read(data.buff, sizeof(SHRT));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[1]);
		m_value = data.value;
	}

	template <>
	size_t ShrtField::get_size() const
	{
		return sizeof(USHRT);
	}
}
}
