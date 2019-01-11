#include "ki/pclass/EnumType.h"
#include "ki/util/exception.h"
#include <sstream>
#include "ki/pclass/Enum.h"

namespace ki
{
namespace pclass
{
	EnumType::Element::Element(const std::string &name,
		const enum_value_t value)
	{
		m_name = name;
		m_value = value;
	}

	const std::string &EnumType::Element::get_name() const
	{
		return m_name;
	}

	enum_value_t EnumType::Element::get_value() const
	{
		return m_value;
	}

	EnumType::EnumType(const std::string &name,
		const TypeSystem &type_system)
		: Type(name, type_system)
	{
		m_kind = Kind::ENUM;
	}

	EnumType::~EnumType()
	{
		for (auto it = m_elements.begin();
			it != m_elements.end(); ++it)
		{
			delete *it;
		}
	}

	bool EnumType::has_element(const std::string& name) const
	{
		return m_element_name_lookup.find(name) != m_element_name_lookup.end();
	}

	bool EnumType::has_element(const enum_value_t value) const
	{
		return m_element_value_lookup.find(value) != m_element_value_lookup.end();
	}

	const EnumType::Element& EnumType::get_element(const std::string& name) const
	{
		const auto &it = m_element_name_lookup.find(name);
		if (it == m_element_name_lookup.end())
		{
			std::ostringstream oss;
			oss << "No element called '" << name
				<< "' exists in enum: '" << get_name() << "'.";
			throw runtime_error(oss.str());
		}

		return *it->second;
	}

	const EnumType::Element& EnumType::get_element(const enum_value_t value) const
	{
		const auto &it = m_element_value_lookup.find(value);
		if (it == m_element_value_lookup.end())
		{
			std::ostringstream oss;
			oss << "No element with value " << value
				<< " exists in enum: '" << get_name() << "'.";
			throw runtime_error(oss.str());
		}

		return *it->second;
	}

	EnumType& EnumType::add_element(const std::string &name,
		const enum_value_t value)
	{
		// Has an element already been added with this name?
		if (has_element(name))
		{
			std::ostringstream oss;
			oss << "Enum '" << get_name()
				<< "' already has an element called '"
				<< name << "'.";
			throw runtime_error(oss.str());
		}

		// Has an element already been added with this value?
		if (has_element(value))
		{
			std::ostringstream oss;
			oss << "Enum '" << get_name()
				<< "' already has an element with the value '"
				<< value << "'.";
			throw runtime_error(oss.str());
		}

		// Create the element and add it to our lookups
		auto *element = new Element(name, value);
		m_elements.push_back(element);
		m_element_name_lookup[name] = element;
		m_element_value_lookup[value] = element;
		return *this;
	}

	void EnumType::write_to(BitStream &stream, Value &value) const
	{
		// Get an Enum reference and use it to write to the stream
		value.as<Enum>().get<Enum>().write_to(stream);
	}

	Value EnumType::read_from(BitStream &stream) const
	{
		auto value = Enum(*this);
		value.read_from(stream);
		return Value::make_value<Enum>(value);
	}
}
}
