#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"

namespace ki
{
namespace dml
{
	template <>
	void UBytField::write_to(std::ostream &ostream) const
	{
		ValueBytes<UBYT> data;
		data.value = m_value;
		ostream.write(data.buff, sizeof(UBYT));
	}

	template <>
	void UBytField::read_from(std::istream &istream)
	{
		ValueBytes<UBYT> data;
		istream.read(data.buff, sizeof(UBYT));
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read UBYT value (" << m_name << ").";
			throw parse_error(oss.str());
		}
		m_value = data.value;
	}

	template <>
	size_t UBytField::get_size() const
	{
		return sizeof(BYT);
	}

	template <>
	const char* UBytField::get_type_name() const
	{
		return "UBYT";
	}
}
}
