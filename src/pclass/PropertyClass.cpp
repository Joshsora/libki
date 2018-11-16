#include "ki/pclass/PropertyClass.h"

namespace ki
{
namespace pclass
{
	PropertyClass::PropertyClass(const Type &type, const TypeSystem &type_system)
	{
		m_type = &type;
		m_properties = new PropertyList();
	}

	PropertyClass::~PropertyClass()
	{
		// Delete the list of properties
		delete m_properties;
	}

	const Type& PropertyClass::get_type() const
	{
		return *m_type;
	}

	PropertyList &PropertyClass::get_properties()
	{
		return *m_properties;
	}

	const PropertyList& PropertyClass::get_properties() const
	{
		return *m_properties;
	}
}
}
