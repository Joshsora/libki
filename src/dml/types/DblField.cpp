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
		ValueBytes<DBL> data;
		data.value = m_value;

		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[8]);

		ostream.write(data.buff, sizeof(DBL));
	}

	template <>
	void DblField::read_from(std::istream &istream)
	{
		ValueBytes<DBL> data;
		istream.read(data.buff, sizeof(DBL));
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read DBL value (" << m_name << ").";
			throw parse_error(oss.str());
		}

		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[8]);
		
		m_value = data.value;
	}

	template <>
	size_t DblField::get_size() const
	{
		return sizeof(DBL);
	}

	template <>
	const char* DblField::get_type_name() const
	{
		return "DBL";
	}
}
}
