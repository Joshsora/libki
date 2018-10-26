#pragma once
#include <vector>
#include <map>
#include "Type.h"
#include "PrimitiveType.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class TypeSystem
	{
	public:
		/**
		 * @return A singleton instance of TypeSystem with all C++ primitives defined.
		 */
		static TypeSystem &get_singleton();

		explicit TypeSystem(HashCalculator *hash_calculator);
		~TypeSystem();

		void set_hash_calculator(HashCalculator *hash_calculator);

		Type &get_type(const std::string &name) const;
		Type &get_type(hash_t hash) const;

		template <typename ValueT>
		Type &define_primitive(const std::string &name)
		{
			auto hash = m_hash_calculator->calculate_type_hash(name);
			auto *type = new PrimitiveType<ValueT>(name, hash);
			define_type(type);
			return *type;
		}

		template <class ClassT>
		Type &define_class(const std::string &name)
		{
			// Ensure that ClassT inherits PropertyClass
			static_assert(std::is_base_of<PropertyClass, ClassT>::value, "ClassT must inherit PropertyClass!");

			// TODO: Create class types
		}

		template <typename EnumT>
		Type &define_enum(const std::string &name)
		{
			// Ensure that EnumT is an enum
			static_assert(std::is_enum<EnumT>::value, "EnumT must be an enum!");

			// TODO: Create enum types
		}

	protected:
		void define_type(Type *type);

	private:
		static TypeSystem *s_instance;

		TypeList m_types;
		TypeNameMap m_type_name_lookup;
		TypeHashMap m_type_hash_lookup;
		HashCalculator *m_hash_calculator;
	};
}
}
