#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"

namespace ki
{
namespace dml
{
	template <>
	void BytField::write_to(std::ostream &ostream) const
	{
		ValueBytes<BYT> data;
		data.value = m_value;
		ostream.write(data.buff, sizeof(BYT));
	}

	template <>
	void BytField::read_from(std::istream &istream)
	{
		ValueBytes<BYT> data;
		istream.read(data.buff, sizeof(BYT));
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read BYT value (" << m_name << ").";
			throw parse_error(oss.str());
		}
		m_value = data.value;
	}

	template <>
	size_t BytField::get_size() const
	{
		return sizeof(BYT);
	}

	template <>
	const char* BytField::get_type_name() const
	{
		return "BYT";
	}

	template <>
	std::string BytField::get_value_string() const
	{
		std::ostringstream oss;
		oss << (int16_t)m_value;
		return oss.str();
	}

	template <>
	void BytField::set_value_from_string(std::string value)
	{
		std::istringstream iss(value);
		int16_t temp = 0;
		iss >> temp;
		m_value = temp & 0xFF;
	}
}
}