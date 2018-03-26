#pragma once
#include "FieldBase.h"
#include "Field.h"

namespace ki
{
namespace dml
{
	class Record : public util::Serializable
	{
	public:
		Record();
		Record(const Record &record);

		bool has_field(std::string name) const;

		template <typename ValueT>
		bool has_field(std::string name) const
		{
			if (!has_field(name))
				return false;
			return m_field_map[name]->is_type<ValueT>();
		}

		template <typename ValueT>
		Field<ValueT> &get_field(std::string name) const
		{
			if (has_field<ValueT>(name))
				return dynamic_cast<Field<ValueT> *>(m_fields[name]);
			return nullptr;
		}

		template <typename ValueT>
		Field<ValueT> &add_field(std::string name, bool transferable = true)
		{
			if (has_field<ValueT>(name))
				return nullptr;
			Field<ValueT> *field = new Field<ValueT>(name, *this);
			auto *base = static_cast<FieldBase *>(field);
			m_fields.push_back(base);
			m_field_map.insert({ base->get_name(), base });
			return field;
		}

		size_t get_field_count() const;

		FieldList::iterator fields_begin() const;
		FieldList::iterator fields_end() const;

		void write_to(std::ostream &ostream) const;
		void read_from(std::istream &istream);
		size_t get_size() const;
	private:
		FieldList m_fields;
		FieldNameMap m_field_map;
	};
}
}
