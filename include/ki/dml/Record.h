#pragma once
#include "FieldBase.h"
#include "Field.h"

namespace ki
{
namespace dml
{
	class Record final : public util::Serializable
	{
	public:
		Record();
		Record(const Record &record);
		virtual ~Record();

		bool has_field(std::string name) const;

		template <typename ValueT>
		bool has_field(std::string name) const
		{
			if (!has_field(name))
				return false;
			return m_field_map.at(name)->is_type<ValueT>();
		}

		template <typename ValueT>
		Field<ValueT> *get_field(std::string name) const
		{
			if (has_field<ValueT>(name))
				return dynamic_cast<Field<ValueT> *>(m_field_map.at(name));
			return nullptr;
		}

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
