#pragma once
#include <rapidxml.hpp>
#include "ki/pclass/TypeSystem.h"

namespace ki
{
namespace serialization
{
	/**
	 * TODO: Documentation
	 */
	class XmlSerializer
	{
	public:
		explicit XmlSerializer(const pclass::TypeSystem &type_system);
		~XmlSerializer() = default;

		std::string save(pclass::PropertyClass *object);
		void load(std::unique_ptr<pclass::PropertyClass> &dest, const std::string &xml_string);

	protected:
		virtual rapidxml::xml_node<> *presave_object(const pclass::PropertyClass *object);
		virtual void preload_object(std::unique_ptr<pclass::PropertyClass> &dest, rapidxml::xml_node<> *node);

	private:
		const pclass::TypeSystem *m_type_system;
		rapidxml::xml_document<> m_document;

		void save_object(rapidxml::xml_node<> *root, const pclass::PropertyClass *object);
		void save_property(rapidxml::xml_node<> *object, const pclass::IProperty &prop);

		void load_object(std::unique_ptr<pclass::PropertyClass> &dest, rapidxml::xml_node<> *root);
		void load_property(pclass::IProperty &prop, rapidxml::xml_node<> *node);
	};
}
}
