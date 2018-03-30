#include "ki/dml/Record.h"

namespace ki
{
namespace dml
{
	Record::Record()
	{
		m_fields = FieldList();
		m_field_map = FieldNameMap();
	}

	Record::~Record()
	{
		m_fields.clear();
		m_field_map.clear();
		for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
			delete *it;
	}

	Record::Record(const Record& record)
	{
		for (auto it = record.fields_begin(); it != record.fields_end(); ++it)
			add_field((*it)->clone(*this));
	}

	bool Record::has_field(std::string name) const
	{
		return m_field_map.count(name);
	}

	size_t Record::get_field_count() const
	{
		return m_fields.size();
	}

	FieldList::const_iterator Record::fields_begin() const
	{
		return m_fields.begin();
	}

	FieldList::const_iterator Record::fields_end() const
	{
		return m_fields.end();
	}

	void Record::write_to(std::ostream &ostream) const
	{
		for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
		{
			if ((*it)->is_transferable())
				(*it)->write_to(ostream);
		}
	}

	void Record::read_from(std::istream &istream)
	{
		for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
		{
			if ((*it)->is_transferable())
				(*it)->read_from(istream);
		}
	}

	size_t Record::get_size() const
	{
		size_t size = 0;
		for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
		{
			if ((*it)->is_transferable())
				size += (*it)->get_size();
		}
		return size;
	}

	void Record::add_field(FieldBase* field)
	{
		m_fields.push_back(field);
		m_field_map.insert({ field->get_name(), field });
	}
}
}
