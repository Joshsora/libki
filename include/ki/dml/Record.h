#pragma once
#include "FieldBase.h"
#include "Field.h"

namespace ki
{
namespace dml
{
	/**
	 * An ordered collection of DML fields.
	 */
	class Record final : public util::Serializable
	{
	public:
		Record();
		Record(const Record &record);
		virtual ~Record();

		/**
		 * Returns true if a field of any type has the name
		 * specified.
		 */
		bool has_field(std::string name) const;

		/**
		 * Returns true if a field exists with the specified
		 * name and type.
		 */
		template <typename ValueT>
		bool has_field(std::string name) const
		{
			if (!has_field(name))
				return false;
			return m_field_map.at(name)->is_type<ValueT>();
		}

		/**
		 * Returns a previously added field with the specified name
		 * and type.
		 * 
		 * If the field was not previously added, then a nullptr is
		 * returned.
		 */
		template <typename ValueT>
		Field<ValueT> *get_field(std::string name) const
		{
			if (has_field<ValueT>(name))
				return dynamic_cast<Field<ValueT> *>(m_field_map.at(name));
			return nullptr;
		}

		/**
		 * Adds a new field to the record with the specified name, type,
		 * and transferability, and returns the newly created field.
		 * 
		 * If a field already exists with the specified name and type,
		 * then the previously added field is returned.
		 * 
		 * If a field already exists with the specified name, but the type
		 * differs, then a nullptr is returned.
		 */
		template <typename ValueT>
		Field<ValueT> *add_field(std::string name, bool transferable = true)
		{
			// Does this field already exist?
			if (has_field(name))
			{
				// Return nullptr if the type is not the same
				auto *field = m_field_map.at(name);
				if (!field->is_type<ValueT>())
					return nullptr;
				return dynamic_cast<Field<ValueT> *>(field);
			}

			// Create the field
			auto *field = new Field<ValueT>(name, *this);
			field->m_transferable = transferable;
			add_field(field);
			return field;
		}

		size_t get_field_count() const;

		FieldList::const_iterator fields_begin() const;
		FieldList::const_iterator fields_end() const;

		void write_to(std::ostream &ostream) const final;
		void read_from(std::istream &istream) final;
		size_t get_size() const final;
	private:
		FieldList m_fields;
		FieldNameMap m_field_map;

		void add_field(FieldBase *field);
	};
}
}
