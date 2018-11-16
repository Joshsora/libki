#pragma once
#include <type_traits>
#include "ki/pclass/types/Type.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class EnumOption
	{
	private:
		std::string m_name;
		uint32_t m_value;
	};

	/**
	* TODO: Documentation
	*/
	template <typename EnumT>
	class EnumType : public Type
	{
		// Ensure that EnumT is an enum
		static_assert(std::is_enum<EnumT>::value, "EnumT must be an enum!");

	public:
		EnumType(std::string name, hash_t hash, TypeSystem *type_system);
	};
}
}