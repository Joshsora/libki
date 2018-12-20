#include "ki/serialization/JsonSerializer.h"
#include <sstream>

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

	void JsonSerializer::save_property(
		json& j, const pclass::IProperty &prop) const
	{
		for (std::size_t i = 0; i < prop.get_element_count(); ++i)
		{
			if (prop.get_type().get_kind() == pclass::Type::kind::CLASS)
			{
				auto *other_object = prop.get_object(i);
				if (prop.is_array())
					j[prop.get_name()].push_back(save_object(other_object));
				else
					j[prop.get_name()] = save_object(other_object);
			}
			else
			{
				auto value = prop.get_value(i).as<json>();
				if (prop.is_array())
					j[prop.get_name()].push_back(value.get<json>());
				else
					j[prop.get_name()] = value.get<json>();
			}
		}
	}

	void JsonSerializer::load(std::unique_ptr<pclass::PropertyClass> &dest,
		const std::string &json_string) const
	{
		try
		{			
			auto j = json::parse(json_string);
			load_object(dest, j);
		}
		catch (json::exception &e)
		{
			std::ostringstream oss;
			oss << "Failed to deserialize JSON - " << e.what();
			throw runtime_error(oss.str());
		}
	}

	bool JsonSerializer::preload_object(
		std::unique_ptr<pclass::PropertyClass>& dest, json& j) const
	{
		// If meta information is not present, assume that the type hash is null.
		if (!j.count("_pclass_meta"))
			dest = nullptr;
		else
		{
			// Use the type hash to instantiate an object
			const auto type_hash = j["_pclass_meta"].value("type_hash", NULL);
			if (type_hash != 0)
			{
				const auto &type = m_type_system->get_type(type_hash);
				dest = type.instantiate();
			}
			else
				dest = nullptr;
		}
		return dest != nullptr;
	}

	void JsonSerializer::load_object(
		std::unique_ptr<pclass::PropertyClass> &dest, json &j) const
	{
		if (!preload_object(dest, j))
			return;

		auto &property_list = dest->get_properties();
		for (auto it = property_list.begin();
			it != property_list.end(); ++it)
		{
			auto &prop = *it;
			load_property(prop, j);
		}
	}

	void JsonSerializer::load_property(
		pclass::IProperty &prop, json &j) const
	{
		if (!j.count(prop.get_name()))
		{
			std::ostringstream oss;
			oss << "JSON object missing property: '" << prop.get_name() << "' "
				<< "(type='" << prop.get_instance().get_type().get_name() << "').";
			throw runtime_error(oss.str());
		}

		auto &property_j = j[prop.get_name()];
		if (prop.is_dynamic())
			prop.set_element_count(property_j.size());

		for (std::size_t i = 0; i < prop.get_element_count(); ++i)
		{
			if (prop.get_type().get_kind() == pclass::Type::kind::CLASS)
			{
				std::unique_ptr<pclass::PropertyClass> other_object = nullptr;
				if (prop.is_array())
					load_object(other_object, property_j.at(i));
				else
					load_object(other_object, property_j);
				prop.set_object(other_object, i);
			}
			else
			{
				if (prop.is_array())
					prop.set_value(
						pclass::Value::make_reference<json>(property_j.at(i)), i
					);
				else
					prop.set_value(
						pclass::Value::make_reference<json>(property_j), i
					);
			}
		}
	}
}
}
