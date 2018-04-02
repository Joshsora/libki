#include "ki/dml/FieldBase.h"
#include "ki/dml/Field.h"

namespace ki
{
namespace dml
{
	FieldBase::FieldBase(std::string name)
	{
		m_name = name;
		m_transferable = true;
		m_type_hash = 0;
	}

	std::string FieldBase::get_name() const
	{
		return m_name;
	}

	bool FieldBase::is_transferable() const
	{
		return m_transferable;
	}

	FieldBase* FieldBase::create_from_xml(const rapidxml::xml_node<>* node)
	{
		auto *type_attr = node->first_attribute("TYPE");
		if (!type_attr)
		{
			std::ostringstream oss;
			oss << "XML Field node is missing required TYPE attribute (" << node->name() << ").";
			throw value_error(oss.str());
		}
		const std::string type = type_attr->value();

		FieldBase *field;
		if (type == "BYT")
			field = new BytField("");
		else if (type == "UBYT")
			field = new UBytField("");
		else if (type == "SHRT")
			field = new ShrtField("");
		else if (type == "USHRT")
			field = new UShrtField("");
		else if (type == "INT")
			field = new IntField("");
		else if (type == "UINT")
			field = new UIntField("");
		else if (type == "STR")
			field = new StrField("");
		else if (type == "WSTR")
			field = new WStrField("");
		else if (type == "FLT")
			field = new FltField("");
		else if (type == "DBL")
			field = new DblField("");
		else if (type == "GID")
			field = new GidField("");
		else
		{
			std::ostringstream oss;
			oss << "Unknown DML type \"" << type << "\" in XML Field node: " << node->name() << ".";
			throw value_error(oss.str());
		}

		field->from_xml(node);
		return field;
	}
}
}
