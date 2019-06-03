#include "ki/pclass/Enum.h"

namespace ki
{
namespace pclass
{
	Enum::Enum(const Type &type, const enum_value_t value)
	{
		// Make sure the type we've been given is an enum type
		if (type.get_kind() != Type::Kind::ENUM)
			throw runtime_error("Enum constructor was supplied with a non-enum type.");

		m_type = &dynamic_cast<const EnumType &>(type);
		set_value(value);
	}

	Enum::Enum(const Type &type, const std::string &element_name)
	{
		// Make sure the type we've been given is an enum type
		if (type.get_kind() != Type::Kind::ENUM)
			throw runtime_error("Enum constructor was supplied with a non-enum type.");

		m_type = &dynamic_cast<const EnumType &>(type);
		set_value(element_name);
	}

	Enum &Enum::operator=(const Enum& that)
	{
		// Are the types the same?
		if (&get_type() != &that.get_type())
			throw runtime_error("Cannot change Enum type after it's constructed.");
		set_value(that.m_value);
		return *this;
	}

	const EnumType &Enum::get_type() const
	{
		return *m_type;
	}

	enum_value_t Enum::get_value() const
	{
		return m_value;
	}

	void Enum::set_value(const enum_value_t value)
	{
		// Make sure the given value is valid for this enum type
		if (value != 0 && !get_type().has_element(value))
		{
			std::ostringstream oss;
			oss << "Enum '" << get_type().get_name()
				<< "' has no element with value: " << value << ".";
			throw runtime_error(oss.str());
		}
		m_value = value;
	}

	void Enum::set_value(const std::string &element_name)
	{
		m_value = get_type().get_element(element_name).get_value();
	}

	void Enum::write_to(BitStream& stream, const bool is_file) const
	{
		if (is_file)
		{
			// Write the element name
			const auto &name = get_type().get_element(m_value).get_name();
			detail::primitive_type_helper<std::string>::write_to(stream, is_file, name);
		}
		else
		{
			// Write the element value
			detail::primitive_type_helper<enum_value_t>::write_to(stream, is_file, m_value);
		}
	}

	void Enum::read_from(BitStream& stream, const bool is_file)
	{
		if (is_file)
		{
			// Set the value using the element name
			const auto name = detail::primitive_type_helper<std::string>
				::read_from(stream, is_file).get<std::string>();
			set_value(name);
		}
		else
		{
			// Set the value using the element value
			const auto value = detail::primitive_type_helper<enum_value_t>
				::read_from(stream, is_file).get<enum_value_t>();
			set_value(value);
		}
	}

	Enum::operator enum_value_t() const
	{
		return get_value();
	}

	void Enum::operator=(const enum_value_t value)
	{
		set_value(value);
	}

	bool Enum::operator==(const enum_value_t &rhs) const
	{
		return m_value == rhs;
	}

	Enum::operator std::basic_string<char>() const
	{
		return get_type().get_element(m_value).get_name();
	}

	void Enum::operator=(const std::string& element_name)
	{
		set_value(element_name);
	}

	bool Enum::operator==(const std::string& rhs) const
	{
		if (!get_type().has_element(rhs))
			return false;
		return m_value == get_type().get_element(rhs).get_value();
	}

	bool Enum::operator==(const Enum &rhs) const
	{
		return &get_type() == &rhs.get_type() &&
			m_value == rhs.m_value;
	}

	bool Enum::operator!=(const Enum& rhs) const
	{
		return !(*this == rhs);
	}
}
}
