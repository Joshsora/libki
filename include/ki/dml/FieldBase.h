#pragma once
#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <rapidxml.hpp>
#include "../util/Serializable.h"

namespace ki
{
namespace dml
{
	class Record;

	/**
	 * An abstract base class for DML fields.
	 */
	class FieldBase : public util::Serializable
	{
		friend Record;
	public:
		FieldBase(std::string name);
		virtual ~FieldBase() = default;

		std::string get_name() const;
		bool is_transferable() const;

		template <typename ValueT>
		bool is_type() const
		{
			return (typeid(ValueT).hash_code() == m_type_hash);
		}
		virtual const char *get_type_name() const = 0;

		/**
		 * Creates an XML node from this field's data.
		 * 
		 * The document is only used to allocate necessary resources, and
		 * so the returned node has not been appended to the document.
		 */
		virtual rapidxml::xml_node<> *as_xml(rapidxml::xml_document<> &doc) const = 0;

		/**
		 * Loads data from an XML Field node into this field.
		 * Example: <FieldName TYPE="STR">Value</FieldName>
		 * 
		 * If the field in the XML data does not have the same type
		 * as this field, then an exception is thrown.
		 */
		virtual void from_xml(const rapidxml::xml_node<> *node) = 0;

		/**
		 * Creates a new Field from XML data.
		 */
		static FieldBase *create_from_xml(const rapidxml::xml_node<> *node);
	protected:
		std::string m_name;
		bool m_transferable;
		size_t m_type_hash;

		/**
		 * Returns a new Field with the same name, transferability
		 * and value but with a different owner Record.
		 */
		virtual FieldBase *clone() const = 0;

		/**
		 * Copies the value of another Field into this one
		 * if the types are the same.
		 */
		virtual void set_value(FieldBase *other) = 0;
	};

	typedef std::vector<FieldBase *> FieldList;
	typedef std::map<std::string, FieldBase *> FieldNameMap;
}
}
