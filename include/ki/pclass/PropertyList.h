#pragma once
#include <unordered_map>
#include <vector>
#include "ki/pclass/HashCalculator.h"

namespace ki
{
namespace pclass
{
	class PropertyClass;
	class IProperty;

	/**
	 * Manages property lookup on a PropertyClass instance.
	 */
	class PropertyList
	{
		friend PropertyClass;

	public:
		class iterator
		{
			friend PropertyList;

		public:
			IProperty &operator*() const;
			IProperty *operator->() const;
			iterator &operator++();
			iterator operator++(int);
			bool operator==(const iterator &that) const;
			bool operator!=(const iterator &that) const;

		private:
			PropertyList *m_list;
			int m_index;

			explicit iterator(PropertyList &list, int index = 0);
		};

		class const_iterator
		{
			friend PropertyList;

		public:
			const IProperty &operator*() const;
			const IProperty *operator->() const;
			const_iterator &operator++();
			const_iterator operator++(int);
			bool operator==(const const_iterator &that) const;
			bool operator!=(const const_iterator &that) const;

		private:
			const PropertyList *m_list;
			int m_index;

			explicit const_iterator(const PropertyList &list, int index = 0);
		};

		/**
		 * @returns The number of properties in this list.
		 */
		std::size_t get_property_count() const;

		/**
		 * @param[in] name The name of the property to search for.
		 * @returns Whether or not a property with the given name exists.
		 */
		bool has_property(const std::string &name) const;

		/**
		 * @param[in] hash The full hash of the property to search for.
		 * @returns Whether or not a property with the given hash exists.
		 */
		bool has_property(hash_t hash) const;

		/**
		 * @param[in] index The index of the property to return.
		 * @returns The property at the specified index.
		 * @throw ki::runtime_error If the index is out of bounds.
		 */
		IProperty &get_property(int index);

		/**
		 * @param[in] index The index of the property to return.
		 * @returns The property at the specified index.
		 * @throw ki::runtime_error If the specified index is out of bounds.
		 */
		const IProperty &get_property(int index) const;

		/**
		 * @param[in] name The name of the property to search for.
		 * @returns The property found with the specified name.
		 * @throw ki::runtime_error If no property exists with the specified name.
		 */
		IProperty &get_property(const std::string &name);

		/**
		 * @param[in] name The name of the property to search for.
		 * @returns The property found with the specified name.
		 * @throw ki::runtime_error If no property exists with the specified name.
		 */
		const IProperty &get_property(const std::string &name) const;

		/**
		 * @param[in] hash The full hash of the property to search for.
		 * @returns The property found with the specified hash.
		 * @throw ki::runtime_error If no property exists with the specified hash.
		 */
		IProperty &get_property(hash_t hash);

		/**
		 * @param[in] hash The full hash of the property to search for.
		 * @returns The property found with the specified hash.
		 * @throw ki::runtime_error If no property exists with the specified hash.
		 */
		const IProperty &get_property(hash_t hash) const;

		iterator begin();
		const_iterator begin() const;

		iterator end();
		const_iterator end() const;

	private:
		std::vector<IProperty *> m_properties;
		std::unordered_map<std::string, IProperty *> m_property_name_lookup;
		std::unordered_map<hash_t, IProperty *> m_property_hash_lookup;

		void add_property(IProperty &prop);
	};
}
}
