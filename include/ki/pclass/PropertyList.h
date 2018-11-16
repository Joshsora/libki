#pragma once
#include <map>
#include <vector>
#include "ki/pclass/HashCalculator.h"

namespace ki
{
namespace pclass
{
	class PropertyBase;

	/**
	 * TODO: Documentation
	 */
	class PropertyList
	{
		friend PropertyBase;

	public:
		using const_iterator = std::vector<PropertyBase *>::const_iterator;

		std::size_t get_property_count() const;

		bool has_property(const std::string &name) const;
		bool has_property(hash_t hash) const;

		const PropertyBase &get_property(int index) const;
		const PropertyBase &get_property(const std::string &name) const;
		const PropertyBase &get_property(hash_t hash) const;

		const_iterator begin() const;
		const_iterator end() const;

	protected:
		void add_property(PropertyBase *prop);

	private:
		std::vector<PropertyBase *> m_properties;
		std::map<std::string, PropertyBase *> m_property_name_lookup;
		std::map<hash_t, PropertyBase *> m_property_hash_lookup;
	};
}
}
