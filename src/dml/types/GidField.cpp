#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>

namespace ki
{
namespace dml
{
	template <>
	void GidField::write_to(std::ostream &ostream) const
	{
		ValueBytes<GID> data;
		data.value = m_value;
		if (data.buff[0] == ((m_value & 0xFF00000000000000) >> 56))
			std::reverse(&data.buff[0], &data.buff[8]);
		ostream.write(data.buff, sizeof(GID));
	}

	template <>
	void GidField::read_from(std::istream &istream)
	{
		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		ValueBytes<GID> data;
		istream.read(data.buff, sizeof(GID));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[8]);
		m_value = data.value;
	}

	template <>
	size_t GidField::get_size() const
	{
		return sizeof(GID);
	}

	template <>
	const char* GidField::get_type_name() const
	{
		return "GID";
	}
}
}
