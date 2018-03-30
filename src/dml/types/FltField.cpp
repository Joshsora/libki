#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"

namespace ki
{
namespace dml
{
	template <>
	void FltField::write_to(std::ostream &ostream) const
	{
		const ValueBytes<USHRT> endianness_check = { 0x0102 };
		ValueBytes<FLT> data = { m_value };
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[3]);
		ostream.write(data.buff, sizeof(FLT));
	}

	template <>
	void FltField::read_from(std::istream &istream)
	{
		const ValueBytes<USHRT> endianness_check = { 0x0102 };
		ValueBytes<FLT> data;
		istream.read(data.buff, sizeof(FLT));
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&data.buff[0], &data.buff[3]);
		m_value = data.value;
	}

	template <>
	size_t FltField::get_size() const
	{
		return sizeof(FLT);
	}
}
}
