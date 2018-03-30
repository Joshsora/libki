#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>

namespace ki
{
namespace dml
{
	template <>
	void DblField::write_to(std::ostream &ostream) const
	{
		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		ValueBytes<DBL> data;
		data.value = m_value;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[8]);
		ostream.write(data.buff, sizeof(DBL));
	}

	template <>
	void DblField::read_from(std::istream &istream)
	{
		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		ValueBytes<DBL> data;
		istream.read(data.buff, sizeof(DBL));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[8]);
		m_value = data.value;
	}

	template <>
	size_t DblField::get_size() const
	{
		return sizeof(DBL);
	}
}
}
