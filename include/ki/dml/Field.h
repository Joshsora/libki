#pragma once
#include "FieldBase.h"
#include "types.h"
#include "exception.h"
#include <sstream>
#include <stdexcept>

namespace ki
{
namespace dml
{
	template <typename ValueT>
	class Field final : public FieldBase
	{
		friend Record;
		friend FieldBase;
	public:
		virtual ~Field() = default;

		ValueT get_value() const
		{
			return m_value;
		}

		void set_value(ValueT value)
		{
			m_value = value;
		}

		const char *get_type_name() const final;

		void write_to(std::ostream &ostream) const final;
		void read_from(std::istream &istream) final;
		size_t get_size() const final;


		/**
		* Creates an XML node from this field's data.
		*
		* The document is only used to allocate necessary resources, and
		* so the returned node has not been appended to the document.
		*/
		rapidxml::xml_node<> *as_xml(rapidxml::xml_document<> &doc) const final
		{
			// Create the node:
			// Copy our current name and value into buffers that are
			// lifetime-dependant on the xml_document, rather than this Field.
			char *name_buffer = doc.allocate_string(m_name.c_str(), 0);
			char *value_buffer = doc.allocate_string(get_value_string().c_str(), 0);
			auto *node = doc.allocate_node(
				rapidxml::node_type::node_element, name_buffer, value_buffer);

			// Create the TYPE attribute
			char *type_attr_value_buffer = doc.allocate_string(get_type_name(), 0);
			auto *type_attr = doc.allocate_attribute("TYPE", type_attr_value_buffer);
			node->append_attribute(type_attr);

			// If we're not transferable, set NOXFER to TRUE
			// NOXFER defaults to FALSE, so we don't need to write it otherwise.
			if (!m_transferable)
			{
				auto *noxfer_attr = doc.allocate_attribute("NOXFER", "TRUE");
				node->append_attribute(noxfer_attr);
			}

			return node;
		}

		/**
		* Loads data from an XML Field node into this field.
		* Example: <FieldName TYPE="STR">Value</FieldName>
		*
		* If the field in the XML data does not have the same type
		* as this field, then a value_error is thrown.
		*/
		void from_xml(const rapidxml::xml_node<> *node) final
		{
			// Use the name of the node as the field name and,
			// default transferable to TRUE.
			m_name = node->name();
			m_transferable = true;

			for (auto *attr = node->first_attribute();
				attr; attr = attr->next_attribute())
			{
				const std::string name = attr->name();
				if (name == "TYPE")
				{
					const std::string value = attr->value();
					if (value != get_type_name())
					{
						std::ostringstream oss;
						oss << "XML Field node has incorrect TYPE attribute value. ";
						oss << "(value=\"" << value << "\", expected=\"" << get_type_name() << "\". ";
						throw value_error(oss.str());
					}
				}
				else if (name == "NOXFER")
				{
					const std::string value = attr->value();
					m_transferable = value != "TRUE";
				}
				else
				{
					std::ostringstream oss;
					oss << "XML Field node has unknown attribute \"" << name << "\".";
					throw value_error(oss.str());
				}
			}

			const std::string value = node->value();
			if (!value.empty())
				set_value_from_string(value);
		}
	protected:
		Field(std::string name, const Record &record)
			: FieldBase(name, record)
		{
			m_type_hash = typeid(ValueT).hash_code();
			m_value = ValueT();
		}
	private:
		ValueT m_value;

		/**
		* Returns a new Field with the same name, transferability,
		* type, and value but with a different owner Record.
		*/
		Field<ValueT> *clone(const Record &record) const final
		{
			auto *clone = new Field<ValueT>(m_name, record);
			clone->m_transferable = true;
			clone->m_value = m_value;
			return clone;
		}

		/**
		* Copies the value of another Field into this one
		* if the types are the same.
		*/
		void set_value(FieldBase *other) final
		{
			if (other->is_type<ValueT>())
			{
				auto *real_other = dynamic_cast<Field<ValueT> *>(other);
				set_value(real_other->get_value());
			}
			else
			{
				std::ostringstream oss;
				oss << "Tried to copy value from " <<
					other->get_type_name() << " field to " <<
					get_type_name() << " field.";
				throw value_error(oss.str());
			}
		}

		std::string get_value_string() const;
		void set_value_from_string(std::string value);
	};

	typedef Field<BYT> BytField;
	typedef Field<UBYT> UBytField;
	typedef Field<SHRT> ShrtField;
	typedef Field<USHRT> UShrtField;
	typedef Field<INT> IntField;
	typedef Field<UINT> UIntField;
	typedef Field<STR> StrField;
	typedef Field<WSTR> WStrField;
	typedef Field<FLT> FltField;
	typedef Field<DBL> DblField;
	typedef Field<GID> GidField;

	template <typename ValueT>
	std::string Field<ValueT>::get_value_string() const
	{
		std::ostringstream oss;
		oss << m_value;
		return oss.str();
	}

	template <typename ValueT>
	void Field<ValueT>::set_value_from_string(const std::string value)
	{
		std::istringstream iss(value);
		iss >> m_value;
	}

	template <>
	std::string StrField::get_value_string() const;

	template <>
	void StrField::set_value_from_string(std::string value);

	template <>
	std::string WStrField::get_value_string() const;

	template <>
	void WStrField::set_value_from_string(std::string value);
}
}
