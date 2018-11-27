#pragma once
#include <map>
#include <vector>
#include "ki/pclass/HashCalculator.h"

namespace ki
{
namespace pclass
{
	class PropertyClass;
	class PropertyBase;

	/**
	 * TODO: Documentation
	 */
	class PropertyList
	{
		friend PropertyClass;

	public:
		/**
		 * TODO: Documentation
		 */
		class iterator
		{
			friend PropertyList;

		public:
			PropertyBase &operator*() const;
			PropertyBase *operator->() const;
			iterator &operator++();
			iterator operator++(int);
			bool operator==(const iterator &that) const;
			bool operator!=(const iterator &that) const;

		private:
			PropertyList *m_list;
			int m_index;

			explicit iterator(PropertyList &list, const int index = 0);
		};

		/**
		 * TODO: Documentation
		 */
		class const_iterator
		{
			friend PropertyList;

		public:
			const PropertyBase &operator*() const;
			const PropertyBase *operator->() const;
			const_iterator &operator++();
			const_iterator operator++(int);
			bool operator==(const const_iterator &that) const;
			bool operator!=(const const_iterator &that) const;

		private:
			const PropertyList *m_list;
			int m_index;

			explicit const_iterator(const PropertyList &list, const int index = 0);
		};

		std::size_t get_property_count() const;

		bool has_property(const std::string &name) const;
		bool has_property(hash_t hash) const;

		PropertyBase &get_property(int index);
		const PropertyBase &get_property(int index) const;

		PropertyBase &get_property(const std::string &name);
		const PropertyBase &get_property(const std::string &name) const;

		PropertyBase &get_property(hash_t hash);
		const PropertyBase &get_property(hash_t hash) const;

		iterator begin();
		const_iterator begin() const;

		iterator end();
		const_iterator end() const;

	protected:
		void add_property(PropertyBase &prop);

	private:
		std::vector<PropertyBase *> m_properties;
		std::map<std::string, PropertyBase *> m_property_name_lookup;
		std::map<hash_t, PropertyBase *> m_property_hash_lookup;
	};
}
}
