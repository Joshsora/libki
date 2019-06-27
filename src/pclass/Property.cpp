#include "ki/pclass/Property.h"
#include "ki/pclass/PropertyClass.h"
#include "ki/pclass/TypeSystem.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	IProperty::IProperty(PropertyClass &object,
		const std::string &name, const Type &type, IProperty::flags flags)
	{
		m_instance = &object;
		m_name = name;
		m_name_hash = type
			.get_type_system()
			.get_hash_calculator()
			.calculate_property_hash(name);
		m_full_hash = m_name_hash + type.get_hash();
		m_type = &type;
		m_flags = flags;

		// Add this property to the object's property list
		object.add_property(*this);
	}

	IProperty::IProperty(PropertyClass &object, 
		const IProperty &that)
	{
		m_instance = &object;
		m_name = that.m_name;
		m_name_hash = that.m_name_hash;
		m_full_hash = that.m_full_hash;
		m_type = that.m_type;
		m_flags = that.m_flags;

		// Add this property to the object's property list
		object.add_property(*this);
	}

	const PropertyClass &IProperty::get_instance() const
	{
		return *m_instance;
	}

	const std::string &IProperty::get_name() const
	{
		return m_name;
	}

	hash_t IProperty::get_name_hash() const
	{
		return m_name_hash;
	}

	hash_t IProperty::get_full_hash() const
	{
		return m_full_hash;
	}

	const Type &IProperty::get_type() const
	{
		return *m_type;
	}

	IProperty::flags IProperty::get_flags() const
	{
		return m_flags;
	}

	bool IProperty::is_pointer() const
	{
		return false;
	}

	bool IProperty::is_dynamic() const
	{
		return false;
	}

	bool IProperty::is_array() const
	{
		return false;
	}

	std::size_t IProperty::get_element_count() const
	{
		return 0;
	}

	void IProperty::write_value_to(BitStream &stream, const bool is_file, const std::size_t index) const
	{
		if (index < 0 || index >= get_element_count())
			throw runtime_error("Index out of bounds.");
		auto ref_value = get_value(index);
		get_type().write_to(stream, is_file, ref_value);
	}

	void IProperty::read_value_from(BitStream &stream, const bool is_file, const std::size_t index)
	{
		if (index < 0 || index >= get_element_count())
			throw runtime_error("Index out of bounds.");
		set_value(get_type().read_from(stream, is_file), index);
	}
}
}
