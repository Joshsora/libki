#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>

namespace ki
{
namespace dml
{
	template <>
	void FltField::write_to(std::ostream &ostream) const
	{
		ValueBytes<FLT> data;
		data.value = m_value;

		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[4]);
		
		ostream.write(data.buff, sizeof(FLT));
	}

	template <>
	void FltField::read_from(std::istream &istream)
	{
		ValueBytes<FLT> data;
		istream.read(data.buff, sizeof(FLT));
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read FLT value (" << m_name << ").";
			throw parse_error(oss.str());
		}

		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[4]);
		m_value = data.value;
	}

	template <>
	size_t FltField::get_size() const
	{
		return sizeof(FLT);
	}

	template <>
	const char* FltField::get_type_name() const
	{
		return "FLT";
	}
}
}
