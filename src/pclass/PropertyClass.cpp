#include "ki/pclass/PropertyClass.h"

namespace ki
{
namespace pclass
{
	PropertyClass::PropertyClass(const Type &type, const TypeSystem &type_system)
	{
		m_type = &type;
	}

	PropertyClass::PropertyClass(const PropertyClass &that)
	{
		m_type = that.m_type;
		m_properties = PropertyList();
	}

	PropertyClass &PropertyClass::operator=(const PropertyClass &that)
	{
		m_type = that.m_type;
		m_properties = PropertyList();
		return *this;
	}

	const Type& PropertyClass::get_type() const
	{
		return *m_type;
	}

	PropertyList &PropertyClass::get_properties()
	{
		return m_properties;
	}

	const PropertyList& PropertyClass::get_properties() const
	{
		return m_properties;
	}

	void PropertyClass::add_property(IProperty &prop)
	{
		m_properties.add_property(prop);
	}
}
}
