#include "ki/serialization/XmlSerializer.h"
#include <sstream>
#include <rapidxml_print.hpp>

namespace ki
{
namespace serialization
{
	XmlSerializer::XmlSerializer(const pclass::TypeSystem &type_system)
	{
		m_type_system = &type_system;
	}

	std::string XmlSerializer::save(pclass::PropertyClass *object)
	{
		// Create the XML document and root node
		m_document.clear();
		auto *root_node = m_document.allocate_node(
			rapidxml::node_type::node_element, "Objects");
		m_document.append_node(root_node);

		// Save the object into the root node
		save_object(root_node, object);

		// Print the XML document into the string and return it
		std::string s;
		print(std::back_inserter(s), m_document, 0);
		return s;
	}

	rapidxml::xml_node<> *XmlSerializer::presave_object(const pclass::PropertyClass *object)
	{
		if (!object)
			return nullptr;

		auto type_name = object->get_type().get_name();
		auto *object_node = m_document.allocate_node(
			rapidxml::node_type::node_element, "Class"
		);

		auto *class_attribute_value = m_document.allocate_string(type_name.data());
		auto *class_attribute = m_document.allocate_attribute(
			"Name", class_attribute_value
		);
		object_node->append_attribute(class_attribute);
		return object_node;
	}

	void XmlSerializer::save_object(rapidxml::xml_node<> *root, const pclass::PropertyClass* object)
	{
		auto *object_node = presave_object(object);
		if (!object_node)
			return;

		auto &property_list = object->get_properties();
		for (auto it = property_list.begin();
			it != property_list.end(); ++it)
		{
			auto &prop = *it;
			save_property(object_node, prop);
		}

		root->append_node(object_node);
	}

	void XmlSerializer::save_property(rapidxml::xml_node<> *object, const pclass::IProperty& prop)
	{
		for (std::size_t i = 0; i < prop.get_element_count(); ++i)
		{
			auto *property_name = m_document.allocate_string(prop.get_name().data());
			auto *property_node = m_document.allocate_node(
				rapidxml::node_element, property_name
			);

			if (prop.is_array())
			{
				std::ostringstream oss;
				oss << i;
				auto key = oss.str();
				auto *key_value = m_document.allocate_string(key.data());
				auto *key_attribute = m_document.allocate_attribute("key", key_value);
				property_node->append_attribute(key_attribute);
			}

			if (prop.get_type().get_kind() == pclass::Type::Kind::CLASS)
			{
				auto *other_object = prop.get_object(i);
				if (other_object)
					save_object(property_node, other_object);
				else
					property_node->value("0");
			}
			else
			{
				auto value = prop.get_value(i).as<std::string>().get<std::string>();
				auto *property_value = m_document.allocate_string(value.data());
				property_node->value(property_value);
			}

			object->append_node(property_node);
		}
	}

	void XmlSerializer::load(std::unique_ptr<pclass::PropertyClass> &dest, const std::string &xml_string)
	{
		m_document.clear();
		auto *c_xml_string = m_document.allocate_string(xml_string.data());
		try
		{
			m_document.parse<0>(c_xml_string);
		}
		catch (rapidxml::parse_error &e)
		{
			std::ostringstream oss;
			oss << "Failed to parse given XML string: " << e.what();
			throw runtime_error(oss.str());
		}

		// Get the <Objects> root node
		auto *root_node = m_document.first_node("Objects");
		load_object(dest, root_node);
	}

	void XmlSerializer::preload_object(std::unique_ptr<pclass::PropertyClass> &dest, rapidxml::xml_node<> *node)
	{
		auto *name_attribute = node->first_attribute("Name");
		if (!name_attribute)
			throw runtime_error("'Class' element was missing 'Name' attribute.");

		const auto type_name = std::string(
			name_attribute->value(), name_attribute->value_size());
		const auto &type = m_type_system->get_type(type_name);
		dest = type.instantiate();
	}

	void XmlSerializer::load_object(
		std::unique_ptr<pclass::PropertyClass> &dest, rapidxml::xml_node<> *root)
	{
		auto *object_node = root->first_node("Class");
		if (!object_node)
		{
			dest = nullptr;
			return;
		}

		preload_object(dest, object_node);
		auto &property_list = dest->get_properties();
		for (auto it = property_list.begin();
			it != property_list.end(); ++it)
		{
			auto &prop = *it;
			load_property(prop, object_node);
		}

		// All properties on this object have been set, let the new
		// instance react to this change
		dest->on_created();
	}

	void XmlSerializer::load_property(pclass::IProperty &prop, rapidxml::xml_node<> *node)
	{
		auto *property_name = prop.get_name().data();

		// Get the node that contains data for this property and
		// check if it exists (because it must exist at least once)
		auto *property_node = node->first_node(property_name);
		if (!property_node)
		{
			std::ostringstream oss;
			oss << "Missing parameter element: '" << prop.get_name() << "'.";
			throw runtime_error(oss.str());
		}

		// Get a list of element nodes that have values for this property
		// in the order of their "key" attribute
		std::vector<rapidxml::xml_node<> *> property_entries;
		while (property_node)
		{
			// Get the key attribute if the property is an array
			if (prop.is_array())
			{
				auto *key_attribute = property_node->first_attribute("key");
				if (!key_attribute)
				{
					std::ostringstream oss;
					oss << "Parameter element '" << prop.get_name()
						<< "' is missing 'key' attribute.";
					throw runtime_error(oss.str());
				}

				// Get the value of the key attribute
				auto key_value_str = std::string(
					key_attribute->value(), key_attribute->value_size()
				);
				std::size_t key_value;
				std::istringstream iss(key_value_str);
				iss >> key_value;

				// Store this node at the key value
				property_entries.insert(property_entries.begin() + key_value, property_node);
			}
			else
				property_entries.push_back(property_node);
			property_node = property_node->next_sibling(property_name);
		}

		// If the property is dynamic, make sure the element count has been set,
		// otherwise, make sure we have the correct number of elements
		if (prop.is_dynamic())
			prop.set_element_count(property_entries.size());
		else if (property_entries.size() != prop.get_element_count())
		{
			std::ostringstream oss;
			oss << "Expected " << prop.get_element_count() << " values for '"
				<< prop.get_name() << "' but got " << property_entries.size() << ".";
			throw runtime_error(oss.str());
		}

		// Update the value(s) of the property
		for (std::size_t i = 0; i < prop.get_element_count(); ++i)
		{
			auto *element_node = property_entries.at(i);
			if (prop.get_type().get_kind() == pclass::Type::Kind::CLASS)
			{
				std::unique_ptr<pclass::PropertyClass> other_object = nullptr;
				load_object(other_object, element_node);
				prop.set_object(other_object, i);
			}
			else
			{
				auto element_value = std::string(
					element_node->value(), element_node->value_size()
				);
				prop.set_value(
					pclass::Value::make_reference<std::string>(element_value), i
				);
			}
		}
	}
}
}
