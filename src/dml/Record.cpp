#include "ki/dml/Record.h"
#include <algorithm>

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
		for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
			delete *it;
		m_fields.clear();
		m_field_map.clear();
	}

	Record::Record(const Record& record)
	{
		for (auto it = record.fields_begin(); it != record.fields_end(); ++it)
			add_field((*it)->clone());
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

	rapidxml::xml_node<> *Record::as_xml(rapidxml::xml_document<> &doc) const
	{
		auto *node = doc.allocate_node(rapidxml::node_type::node_element, "RECORD");
		for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
			node->append_node((*it)->as_xml(doc));
		return node;
	}

	void Record::from_xml(rapidxml::xml_node<> *node)
	{
		// Make sure that we've been passed a <RECORD> element.
		const std::string node_name = node->name();
		if (node_name != "RECORD")
		{
			std::ostringstream oss;
			oss << "Expected <RECORD> node but got <" << node->name() << ">.";
			throw value_error(oss.str());
		}

		// Every child node inside a <RECORD> element is a Field.
		for (auto *field_node = node->first_node();
			field_node; field_node = field_node->next_sibling())
		{
			FieldBase *field = FieldBase::create_from_xml(field_node);
			if (has_field(field->get_name()))
			{
				// Is the old field the same type as the one created from
				// the XML data?
				FieldBase *old_field = m_field_map.at(field->get_name());
				if (field->m_type_hash == old_field->m_type_hash)
				{
					// Set the value of the old field to the value of the new
					// one.
					old_field->set_value(field);
					delete field;
				}
				else
				{
					// Since the types are different, we can't set the value
					// of the old field to the value of the new one so,
					// replace the old field with this new one instead.
					const std::ptrdiff_t index = std::find(
						m_fields.begin(), m_fields.end(), old_field) - m_fields.begin();
					m_fields[index] = field;
					m_field_map[field->get_name()] = field;
					delete old_field;
				}
			}
			else
				add_field(field);
		}
	}
}
}
