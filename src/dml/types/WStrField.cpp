#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"
#include <algorithm>
#include <locale>
#include <codecvt>

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
			std::reverse(&data.buff[0], &data.buff[2]);
		ostream.write(data.buff, sizeof(USHRT));
		ostream.write((char *)m_value.c_str(), m_value.length() * sizeof(char16_t));
	}

	template <>
	void WStrField::read_from(std::istream &istream)
	{
		// Get the length
		ValueBytes<USHRT> length_data;
		istream.read(length_data.buff, sizeof(USHRT));
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read WSTR value (" << m_name << ").";
			throw parse_error(oss.str());
		}

		ValueBytes<USHRT> endianness_check;
		endianness_check.value = 0x0102;
		if (endianness_check.buff[0] == 0x01)
			std::reverse(&length_data.buff[0], &length_data.buff[2]);

		// Read the data into a buffer
		size_t length = length_data.value * sizeof(char16_t);
		char *data = new char[length + sizeof(char16_t)]{ 0 };
		istream.read(data, length);
		if (istream.fail())
		{
			std::ostringstream oss;
			oss << "Not enough data was available to read WSTR value (" << m_name << ").";
			throw parse_error(oss.str());
		}

		// Reverse each character from little endian to big endian
		// if memory is supposed to be in big endian on this PC.
		for (int i = 0; i < length; i += 2)
		{
			if (endianness_check.buff[0] == 0x01)
				std::reverse(&data[i], &data[i + 2]);
		}

		m_value = WSTR((char16_t *)data);
		delete[] data;
	}

	template <>
	size_t WStrField::get_size() const
	{
		return sizeof(USHRT) + (m_value.length() * sizeof(char16_t));
	}

	template <>
	const char* WStrField::get_type_name() const
	{
		return "WSTR";
	}

	template <>
	std::string WStrField::get_value_string() const
	{
#if _MSC_VER >= 1900
		std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
		auto p = reinterpret_cast<const int16_t *>(m_value.data());
		STR temp = convert.to_bytes(p, p + m_value.size());
#else
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		STR temp = convert.to_bytes(m_value);
#endif
		return temp;
	}

	template <>
	void WStrField::set_value_from_string(std::string value)
	{
#if _MSC_VER >= 1900
		std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
		m_value = reinterpret_cast<const char16_t *>(convert.from_bytes(value).data());
#else
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		m_value = convert.from_bytes(value);
#endif
	}
}
}
