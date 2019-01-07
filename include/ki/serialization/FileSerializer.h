#pragma once
#include <memory>
#include "ki/pclass/TypeSystem.h"
#include "ki/serialization/BinarySerializer.h"

namespace ki
{
namespace serialization
{
	/**
	 * TODO: Documentation
	 */
	class FileSerializer
	{
		static constexpr const char *BINARY_HEADER = "BINd";
		static constexpr const char *JSON_HEADER = "JSON";

	public:
		explicit FileSerializer(pclass::TypeSystem &type_system);

		void save_binary(pclass::PropertyClass *object,
			BinarySerializer::flags flags, const std::string &filepath) const;
		void save_xml(pclass::PropertyClass *object, const std::string &filepath) const;
		void save_json(pclass::PropertyClass *object, const std::string &filepath) const;

		void load(std::unique_ptr<pclass::PropertyClass> &dest, const std::string &filepath) const;

	private:
		const pclass::TypeSystem *m_type_system;
	};
}
}
