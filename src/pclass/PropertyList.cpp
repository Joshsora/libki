#include "ki/pclass/PropertyList.h"
#include "ki/pclass/Property.h"
#include "ki/util/exception.h"
#include <sstream>
#include <iomanip>

namespace ki
{
namespace pclass
{
	std::size_t PropertyList::get_property_count() const
	{
		return m_properties.size();
	}

	bool PropertyList::has_property(const std::string& name) const
	{
		return m_property_name_lookup.find(name)
			!= m_property_name_lookup.end();
	}

	bool PropertyList::has_property(const hash_t hash) const
	{
		return m_property_hash_lookup.find(hash)
			!= m_property_hash_lookup.end();
	}

	const PropertyBase& PropertyList::get_property(const int index) const
	{
		if (index >= 0 && index < m_properties.size())
			return *m_properties[index];

		std::ostringstream oss;
		oss << "Property index out of range. (index=" << index
			<< ", size=" << m_properties.size() << ")";
		throw runtime_error(oss.str());
	}

	const PropertyBase& PropertyList::get_property(const std::string& name) const
	{
		const auto it = m_property_name_lookup.find(name);
		if (it != m_property_name_lookup.end())
			return *it->second;

		std::ostringstream oss;
		oss << "Could not find property with name: '" << name << "'.";
		throw runtime_error(oss.str());
	}

	const PropertyBase& PropertyList::get_property(const hash_t hash) const
	{
		const auto it = m_property_hash_lookup.find(hash);
		if (it != m_property_hash_lookup.end())
			return *it->second;

		std::ostringstream oss;
		oss << "Could not find property with hash: 0x"
			<< std::hex << std::setw(8) << std::setfill('0')
			<< std::uppercase << hash << ".";
		throw runtime_error(oss.str());
	}

	PropertyList::const_iterator PropertyList::begin() const
	{
		return m_properties.cbegin();
	}

	PropertyList::const_iterator PropertyList::end() const
	{
		return m_properties.cend();
	}

	void PropertyList::add_property(PropertyBase *prop)
	{
		// Make sure a property with the same name as another isn't being added
		if (has_property(prop->get_name()))
		{
			// This pointer is going to be lost, so delete it now
			delete prop;

			std::ostringstream oss;
			oss << "A property has already been added with name: '"
				<< prop->get_name() << "'.";
			throw runtime_error(oss.str());
		}

		// Check for hash collisions
		if (has_property(prop->get_full_hash()))
		{
			const auto &other = get_property(prop->get_full_hash());

			// This pointer is going to be lost, so delete it now
			delete prop;

			std::ostringstream oss;
			oss << "Cannot add property '" << prop->get_name() << "'. "
				<< "Hash collision with property '" << other.get_name() << "'.";
			throw runtime_error(oss.str());
		}

		// Add the property to lookups
		m_properties.push_back(prop);
		m_property_name_lookup[prop->get_name()] = prop;
		m_property_hash_lookup[prop->get_full_hash()] = prop;
	}
}
}
