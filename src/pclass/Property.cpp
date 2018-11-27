#include "ki/pclass/Property.h"
#include "ki/pclass/PropertyClass.h"
#include "ki/pclass/TypeSystem.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	PropertyBase::PropertyBase(PropertyClass &object,
		const std::string &name, const Type &type)
	{
		m_instance = &object;
		m_name = name;
		m_name_hash = type
			.get_type_system()
			.get_hash_calculator()
			.calculate_property_hash(name);
		m_full_hash = m_name_hash + type.get_hash();
		m_type = &type;

		// Add this property to the object's property list
		object.add_property(*this);
	}

	PropertyBase::PropertyBase(PropertyClass &object, 
		const PropertyBase &that)
	{
		m_instance = &object;
		m_name = that.m_name;
		m_name_hash = that.m_name_hash;
		m_full_hash = that.m_full_hash;
		m_type = that.m_type;

		// Add this property to the object's property list
		object.add_property(*this);
	}

	const PropertyClass &PropertyBase::get_instance() const
	{
		return *m_instance;
	}

	std::string PropertyBase::get_name() const
	{
		return m_name;
	}

	hash_t PropertyBase::get_name_hash() const
	{
		return m_name_hash;
	}

	hash_t PropertyBase::get_full_hash() const
	{
		return m_full_hash;
	}

	const Type &PropertyBase::get_type() const
	{
		return *m_type;
	}

	bool PropertyBase::is_pointer() const
	{
		return false;
	}

	bool PropertyBase::is_dynamic() const
	{
		return false;
	}

	DynamicPropertyBase::DynamicPropertyBase(PropertyClass &object,
		const std::string& name, const Type& type)
		: PropertyBase(object, name, type)
	{}

	DynamicPropertyBase::DynamicPropertyBase(PropertyClass &object,
		const DynamicPropertyBase &that)
		: PropertyBase(object, that)
	{}

	bool DynamicPropertyBase::is_dynamic() const
	{
		return true;
	}

	Value DynamicPropertyBase::get_value() const
	{
		// The caller must specify an index
		throw runtime_error("Called get_value() on a dynamic property. Use get_value(index) instead.");
	}

	const PropertyClass *DynamicPropertyBase::get_object() const
	{
		// The caller must specify an index
		throw runtime_error("Called get_object() on a dynamic property. Use get_object(index) instead.");
	}

	void DynamicPropertyBase::set_object(PropertyClass *object)
	{
		// The caller must specify an index
		throw runtime_error("Called set_object() on a dynamic property. Use set_object(index) instead.");
	}

	void DynamicPropertyBase::write_value_to(BitStream &stream) const
	{
		// The caller must specify an index
		throw runtime_error("Called write_value_to() on a dynamic property. Use write_value_to(index) instead.");
	}

	void DynamicPropertyBase::read_value_from(BitStream &stream)
	{
		// The caller must specify an index
		throw runtime_error("Called read_value_from() on a dynamic property. Use read_value_from(index) instead.");
	}
}
}
