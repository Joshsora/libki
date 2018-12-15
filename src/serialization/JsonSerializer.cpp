#include "ki/serialization/JsonSerializer.h"

using namespace nlohmann;

namespace ki
{
namespace serialization
{
	JsonSerializer::JsonSerializer(pclass::TypeSystem& type_system,
		const bool is_file)
	{
		m_type_system = &type_system;
		m_is_file = is_file;
	}

	std::string JsonSerializer::save(const pclass::PropertyClass* object) const
	{
		return save_object(object).dump(
			m_is_file ? FILE_INDENT_VALUE : -1,
			' ',
			true
		);
	}

	bool JsonSerializer::presave_object(json& j, const pclass::PropertyClass* object) const
	{
		// Add the object's meta information
		j["_pclass_meta"] = {
			{ "type_hash", object ? object->get_type().get_hash() : NULL }
		};
		return object != nullptr;
	}

	json JsonSerializer::save_object(const pclass::PropertyClass* object) const
	{
		json j;
		if (!presave_object(j, object))
			return j;

		// Add the object's properties
		auto &property_list = object->get_properties();
		for (auto it = property_list.begin();
			it != property_list.end(); ++it)
		{
			auto &prop = *it;
			save_property(j, prop);
		}

		return j;
	}

	void JsonSerializer::save_property(json& j,
		const pclass::IProperty &prop) const
	{
		if (prop.is_dynamic())
		{
			// Cast the property to a IDynamicProperty
			const auto &dynamic_property =
				dynamic_cast<const pclass::IDynamicProperty &>(prop);
			return save_dynamic_property(j, dynamic_property);
		}
		
		if (prop.get_type().get_kind() == pclass::Type::kind::CLASS)
		{
			auto *other_object = prop.get_object();
			j[prop.get_name()] = save_object(other_object);
		}
		else
		{
			auto value = prop.get_value().dereference<json>();
			j[prop.get_name()] = value.get<json>();
		}
	}

	void JsonSerializer::save_dynamic_property(json& j,
		const pclass::IDynamicProperty &prop) const
	{
		json property_value;

		for (auto i = 0; i < prop.get_element_count(); ++i)
		{
			if (prop.get_type().get_kind() == pclass::Type::kind::CLASS)
			{
				auto *other_object = prop.get_object(i);
				property_value.push_back(save_object(other_object));
			}
			else
			{
				auto value = prop.get_value(i).dereference<json>();
				property_value.push_back(value.get<json>());
			}
		}

		j[prop.get_name()] = property_value;
	}

	void JsonSerializer::load(std::unique_ptr<pclass::PropertyClass>& dest, 
		const std::string& json_string) const
	{
		// TODO: JSON Deserialization
		auto j = json::parse(json_string);
		dest = nullptr;
	}
}
}
